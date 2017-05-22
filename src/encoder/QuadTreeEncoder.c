#include "QuadTreeEncoder.h"

#define MODULE_NAME "QuadTreeEncoder"

static pixel_value* buffer;
static size_t MAX_BUFFER_SIZE;
static size_t MIN_BUFFER_SIZE;

static
inline
ERR_RET ifs_transformation_execute_downsampled(int from_x, int from_y, enum ifs_type symmetry,
                                    u_int32_t size, pixel_value* src, u_int32_t src_width,
                                    pixel_value* dest, u_int32_t dest_width){

    INCREMENT_FLOP_COUNT(6, 0, 0, 0)

    from_x = from_x / 2;
    from_y = from_y / 2;
    int d_x = 1;
    int d_y = 1;
    bool in_order = (
            symmetry == SYM_NONE ||
            symmetry == SYM_R180 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_VFLIP
            );

    if (!(  symmetry == SYM_NONE ||
            symmetry == SYM_R90 ||
            symmetry == SYM_VFLIP ||
            symmetry == SYM_RDFLIP))
    {
        from_x += size - 1;
        d_x = -1;
    }

    if (!(  symmetry == SYM_NONE ||
            symmetry == SYM_R270 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_RDFLIP))
    {
        from_y += size - 1;
        d_y = -1;
    }

    int start_x = from_x;
    int start_y = from_y;

    INCREMENT_FLOP_COUNT(2*size*size,
                         4*size*size, size*size, 0)
    INCREMENT_FLOP_COUNT(size,0,0,0)

    for (int to_y = 0; to_y < size; to_y++)
    {
        for (int to_x = 0; to_x < size; to_x++)
        {
            int pixel = src[from_y * src_width + from_x];
            dest[to_y * dest_width + to_x] = pixel;

            if (in_order)
                from_x += d_x;
            else
                from_y += d_y;
        }

        if (in_order)
        {
            from_x= start_x;
            from_y += d_y;
        }
        else
        {
            from_y = start_y;
            from_x += d_x;
        }
    }

    return ERR_SUCCESS;
}

static
inline u_int32_t get_average_pixel(const pixel_value* domain_data, u_int32_t domain_width,
    u_int32_t domain_x, u_int32_t domain_y, u_int32_t size)
{
    u_int32_t top = 0;
    u_int32_t bottom = (size * size);

    // Simple average of all pixels.
    INCREMENT_FLOP_COUNT(size*size, size*size*2,0,0)
    for (size_t y = domain_y; y < domain_y + size; y++)
    {
        for (size_t x = domain_x; x < domain_x + size; x++)
        {
            top += domain_data[y * domain_width + x];

            if (top < 0)
            {
                printf("Error: Accumulator rolled over averaging pixels.\n");
                return ERR_ACCUM_ROLL;
            }
        }
    }
    return top/bottom;
}

static
inline double get_error(
    pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
    pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg,
    int size, double scale)
{
    double top = 0;
    double bottom = (double)(size * size);

    INCREMENT_FLOP_COUNT(size*size*3, size*size*10,size*size,0)
    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x)] - domain_avg);
            int range = (range_data[(range_y + y) * range_width + (range_x + x)] - range_avg);
            int diff = (int)(scale * (double)domain) - range;

            // According to the formula we want (DIFF*DIFF)/(SIZE*SIZE)
            top += (diff * diff);

            if (top < 0)
            {
                printf("Error: Overflow occured during error %lf\n", top);
                return ERR_OVERFLOW;
            }
        }
    }

    return top / bottom;
}

static
inline double get_scale_factor(
    pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
    pixel_value* range_Data, int range_width, int range_x, int range_y, int range_avg,
    int size)
{
    int top = 0;
    int bottom = 0;

    INCREMENT_FLOP_COUNT(size*size*4, size*size*10, 0, 0)
    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x)] - domain_avg);
            int range = (range_Data[(range_y + y) * range_width + (range_x + x)] - range_avg);

            // According to the formula we want (R*D) / (D*D)
            top += range * domain;
            bottom += domain * domain;

            if (bottom < 0)
            {
                printf("Error: Overflow occured during scaling %d %d %d %d\n", y, domain_width, bottom, top);
                return ERR_OVERFLOW;
            }
        }
    }

    if (bottom == 0)
    {
        top = 0;
        bottom = 1;
    }

    return ((double)top) / ((double)bottom);
}

#define ASSIGN_IFS_VALUES(DST, FROM_X, FROM_Y, TO_X, TO_Y, TR_TYPE, SCALE, OFFSET, SIZE)\
    DST.from_x=FROM_X;\
    DST.from_y=FROM_Y;\
    DST.to_x=TO_X;\
    DST.to_y=TO_Y;\
    DST.transformation_type=TR_TYPE;\
    DST.scale=SCALE;\
    DST.offset=OFFSET;\
    DST.size=SIZE;

#define CALCULATE_ERR(SC, DM_X, DM_Y, DM_AVG, RB_X, RB_Y, RB_AVG, OFFSET, ERROR)\
    SC=get_scale_factor(img->image_channels[0], img->width, DM_X, DM_Y, DM_AVG,\
            buffer, block_size, RB_X, RB_Y, RB_AVG, half_block_size);\
    OFFSET = (int)(RB_AVG - SC * (double)DM_AVG);\
\
    ERROR=get_error(buffer, block_size, RB_X, RB_Y, DM_AVG, img->image_channels[0],\
            img->width, DM_X, DM_Y, RB_AVG,\
            half_block_size, SC);


#define UPDATE_MIN_ERROR(DM_X, DM_Y)\
if(tmp_error1<error_1){\
    ASSIGN_IFS_VALUES(best_ifs_1, DM_X, DM_Y, rb_x0, rb_y0, transformation_type, scale_factor1, offset1, half_block_size);\
    error_1=tmp_error1;\
}\
if(tmp_error2<error_2){\
    ASSIGN_IFS_VALUES(best_ifs_2, DM_X, DM_Y, rb_x1, rb_y0, transformation_type, scale_factor2, offset2, half_block_size);\
    error_2=tmp_error2;\
}\
if(tmp_error3<error_3){\
    ASSIGN_IFS_VALUES(best_ifs_3, DM_X, DM_Y, rb_x0, rb_y1, transformation_type, scale_factor3, offset3, half_block_size);\
    error_3=tmp_error3;\
}\
if(tmp_error4<error_4){\
    ASSIGN_IFS_VALUES(best_ifs_4, DM_X, DM_Y, rb_x1, rb_y1, transformation_type, scale_factor4, offset4, half_block_size);\
    error_4=tmp_error4;\
}\

#define CALCULATE_MIN(X_1, Y_1, AVG_1, X_2, Y_2, AVG_2, X_3, Y_3, AVG_3, X_4, Y_4, AVG_4)\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_1, 0, 0, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_1, 0, 0, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_1, 0, 0, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_1, 0, 0, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_1,Y_1);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_2, half_block_size, 0, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_2, half_block_size, 0, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_2, half_block_size, 0, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_2, half_block_size, 0, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_2,Y_2);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_3, 0, half_block_size, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_3, 0, half_block_size, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_3, 0, half_block_size, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_3, 0, half_block_size, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_3,Y_3);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_4, half_block_size, half_block_size, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_4, half_block_size, half_block_size, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_4, half_block_size, half_block_size, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_4, half_block_size, half_block_size, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_4,Y_4);\


static inline
ERR_RET find_matches_for(struct image_data* img, struct ifs_transformation_list* transformations,
                         u_int32_t rb_x0, u_int32_t rb_y0, u_int32_t block_size, u_int32_t threshold){
    u_int32_t half_block_size=block_size/2;
    u_int32_t double_block_size=block_size*2;
    u_int32_t rb_x1=rb_x0+half_block_size;
    u_int32_t rb_y1=rb_y0+half_block_size;

    u_int32_t avarage_pix_0=get_average_pixel(img->image_channels[0], img->width, rb_x0, rb_y0, block_size);
    u_int32_t avarage_pix_1=get_average_pixel(img->image_channels[0], img->width, rb_x0, rb_y0, half_block_size);
    u_int32_t avarage_pix_2=get_average_pixel(img->image_channels[0], img->width, rb_x1, rb_y0, half_block_size);
    u_int32_t avarage_pix_3=get_average_pixel(img->image_channels[0], img->width, rb_x0, rb_y1, half_block_size);
    u_int32_t avarage_pix_4=get_average_pixel(img->image_channels[0], img->width, rb_x1, rb_y1, half_block_size);

    bool found_match0=false;

    double error_0=1e9;
    double error_1=1e9;
    double error_2=1e9;
    double error_3=1e9;
    double error_4=1e9;

    struct ifs_transformation best_ifs_0;
    struct ifs_transformation best_ifs_1;
    struct ifs_transformation best_ifs_2;
    struct ifs_transformation best_ifs_3;
    struct ifs_transformation best_ifs_4;

    double scale_factor1;
    double scale_factor2;
    double scale_factor3;
    double scale_factor4;

    int offset1;
    int offset2;
    int offset3;
    int offset4;

    double tmp_error1;
    double tmp_error2;
    double tmp_error3;
    double tmp_error4;

    int x_1;
    int y_1;

    pixel_value* img_channels=img->image_channels[0];
    pixel_value* img_channels_downsampled=img->image_channels[1];
    int half_width=img->width/2;

    for(size_t y=0; y<img->height; y+=double_block_size)
    {
        for (size_t x=0; x<img->width; x+=double_block_size)
        {
            int x_half=x/2;
            int y_half=y/2;

            u_int32_t domain_avg0=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half, block_size);

            u_int32_t domain_avg1=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half, half_block_size);
            u_int32_t domain_avg2=get_average_pixel(img_channels_downsampled, half_width, x_half+half_block_size, y_half, half_block_size);
            u_int32_t domain_avg3=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half+half_block_size, half_block_size);
            u_int32_t domain_avg4=get_average_pixel(img_channels_downsampled, half_width, x_half+half_block_size, y_half+half_block_size, half_block_size);

            for(int transformation_type=0; transformation_type<SYM_MAX; ++transformation_type)
            {
                ifs_transformation_execute_downsampled(x, y, transformation_type,
                                                    block_size, img->image_channels[1], img->width/2,
                                                    buffer, block_size);

                double scale_factor=get_scale_factor(img->image_channels[0], img->width, rb_x0, rb_y0, domain_avg0,
                        buffer, block_size, 0 ,0, avarage_pix_0, block_size);
                int offset = (int)(avarage_pix_0 - scale_factor * (double)domain_avg0);

                double error=get_error(buffer, block_size, 0, 0, domain_avg0, img->image_channels[0],
                        img->width, rb_x0, rb_y0, avarage_pix_0,
                        block_size, scale_factor);

                if(error<error_0){
                    ASSIGN_IFS_VALUES(best_ifs_0, x, y, rb_x0, rb_y0, transformation_type, scale_factor, offset, block_size);

                    error_0=error;

                    if(error<threshold){
                        found_match0=true;
                    }
                }

                // Smaller blocks
                if(!found_match0){
                    x_1=x+block_size;
                    y_1=y+block_size;

                    switch (transformation_type){
                    case SYM_NONE:
                        CALCULATE_MIN(x,y, domain_avg1,
                                      x_1,y, domain_avg2,
                                      x,y_1, domain_avg3,
                                      x_1,y_1, domain_avg4);
                        break;
                    case SYM_R90:
                        CALCULATE_MIN(x,y_1, domain_avg3,
                                      x,y, domain_avg1,
                                      x_1,y_1, domain_avg4,
                                      x_1,y, domain_avg2);
                        break;
                    case SYM_R180:
                        CALCULATE_MIN(x_1,y_1, domain_avg4,
                                      x,y_1, domain_avg3,
                                      x_1,y, domain_avg2,
                                      x,y, domain_avg1);
                        break;
                    case SYM_R270:
                        CALCULATE_MIN(x_1,y, domain_avg2,
                                      x_1,y_1, domain_avg4,
                                      x,y, domain_avg1,
                                      x,y_1, domain_avg3);
                        break;
                    case SYM_HFLIP:
                        CALCULATE_MIN(x_1,y, domain_avg2,
                                      x,y, domain_avg1,
                                      x_1,y_1, domain_avg4,
                                      x,y_1, domain_avg3);
                        break;
                    case SYM_VFLIP:
                        CALCULATE_MIN(x,y_1, domain_avg3,
                                      x_1,y_1, domain_avg4,
                                      x,y, domain_avg1,
                                      x_1,y, domain_avg2);
                        break;
                    case SYM_FDFLIP:
                        CALCULATE_MIN(x_1,y_1, domain_avg4,
                                      x_1,y, domain_avg2,
                                      x,y_1, domain_avg3,
                                      x,y, domain_avg1);
                        break;
                    case SYM_RDFLIP:
                        CALCULATE_MIN(x,y, domain_avg1,
                                      x,y_1, domain_avg3,
                                      x_1,y, domain_avg2,
                                      x_1,y_1, domain_avg4);
                        break;
                    }
                }
            }
        }
    }

    if(found_match0){
        ifs_trans_push_back(transformations, &best_ifs_0);
    }else if(block_size>(2*MIN_BUFFER_SIZE)){
        u_int32_t quater_block_size=half_block_size/2;
        if(error_1<threshold){
            ifs_trans_push_back(transformations, &best_ifs_1);
        }else{
            find_matches_for(img, transformations, rb_x0, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0, rb_y0+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y0+quater_block_size, quater_block_size, threshold);
        }

        if(error_2<threshold){
            ifs_trans_push_back(transformations, &best_ifs_2);
        }else{
            find_matches_for(img, transformations, rb_x1, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1, rb_y0+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y0+quater_block_size, quater_block_size, threshold);
        }

        if(error_3<threshold){
            ifs_trans_push_back(transformations, &best_ifs_3);
        }else{
            find_matches_for(img, transformations, rb_x0, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0, rb_y1+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y1+quater_block_size, quater_block_size, threshold);
        }

        if(error_4<threshold){
            ifs_trans_push_back(transformations, &best_ifs_4);
        }else{
            find_matches_for(img, transformations, rb_x1, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1, rb_y1+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y1+quater_block_size, quater_block_size, threshold);
        }
    }else if(block_size==MIN_BUFFER_SIZE){
        ifs_trans_push_back(transformations, &best_ifs_1);
        ifs_trans_push_back(transformations, &best_ifs_2);
        ifs_trans_push_back(transformations, &best_ifs_3);
        ifs_trans_push_back(transformations, &best_ifs_4);
    }else{
        assert(false && "Should not happen!");
    }

    return ERR_SUCCESS;
}

ERR_RET qtree_encode(struct Transforms* transformations, struct image_data* src, struct encoder_params params,
                     u_int32_t threshold_param){

    struct image_data img;
    u_int32_t width=src->width;
    u_int32_t height=src->height;
    u_int32_t threshold=threshold_param;    //Temporary fixed value!

    if (width % 32 != 0 || height % 32 != 0)
    {
        return ERR_IMAGE_WRONG_SIZE;
    }

    if(width!=height){
        printf("ERROR! W!=H\n");
        return ERR_IMAGE_WRONG_SIZE;
    }

    MAX_BUFFER_SIZE=width/32;
    if(MAX_BUFFER_SIZE<16)
           MAX_BUFFER_SIZE=16;
    MIN_BUFFER_SIZE=MAX_BUFFER_SIZE/4;
    buffer=malloc(MAX_BUFFER_SIZE*MAX_BUFFER_SIZE*sizeof(pixel_value));
    transformations->max_block_size=MAX_BUFFER_SIZE;
    printf("Starting to compress with block sizes MAX: %d MIN: %d\n", MAX_BUFFER_SIZE, MIN_BUFFER_SIZE);

    if(params.use_ycbcr){
        if(src->color_mode!=COLOR_MODE_YCbCr){
            convert_from_RGB_to_YCbCr(src);
        }
        transformations->color_mode=COLOR_MODE_YCbCr;
    }else{
        transformations->color_mode=COLOR_MODE_RGB;
    }

    //We do our crazy stuff here
    /*
     * Make sense because you are only working with two channels
     * [0] -> Original channel data
     * [1] -> Downsampled channel data
     */
    // We allocate only one channel other one is just a pointer to original
    init_image_data(&img, width, height, 1);
    img.image_channels[1]=img.image_channels[0];
    transformations->channels=src->channels;

    for (size_t channel=0; channel<src->channels; channel++){
//        printf("Channel: %d ==================================\n", channel);
        img.image_channels[0]=src->image_channels[channel];
        down_sample(img.image_channels[0], width, 0,0, width/2, img.image_channels[1]);
        transformations->ch[channel].head=NULL;
        transformations->ch[channel].tail=NULL;
        transformations->ch[channel].elements=0;

        if (channel >= 1 && params.use_ycbcr)
            threshold *= 2;

        for (size_t y = 0; y < img.height; y += MAX_BUFFER_SIZE)
        {
            for (size_t x = 0; x < img.width; x += MAX_BUFFER_SIZE)
            {
                find_matches_for(&img, transformations->ch+channel, x, y, MAX_BUFFER_SIZE, threshold);
                #ifdef DEBUG
                printf(".");
                #endif
            }
            #ifdef DEBUG
            printf("\n");
            #endif
        }

        if (channel >= 1 && params.use_ycbcr)
            threshold /= 2;
    }

    img.image_channels[0]=img.image_channels[1];
    clear_image_data(&img);

    return ERR_SUCCESS;
}
