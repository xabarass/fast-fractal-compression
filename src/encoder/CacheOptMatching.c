#include "Matching.h"

#include "QuadTreeEncoder.h"
#include "EncoderUtils.h"
#include "EncoderMacros.h"
#include "EncoderTransformations.h"

static pixel_value* buffer;
static size_t MAX_BUFFER_SIZE;
static size_t MIN_BUFFER_SIZE;

static inline
ERR_RET find_matches_for(struct image_data* img, struct ifs_transformation_list* transformations,
                         u_int32_t rb_x0, u_int32_t rb_y0, u_int32_t block_size, u_int32_t threshold){

    INCREMENT_FLOP_COUNT(4,0,0,0)

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

    INCREMENT_FLOP_COUNT(1,0,0,0)

    pixel_value* img_channels=img->image_channels[0];
    pixel_value* img_channels_downsampled=img->image_channels[1];
    int half_width=img->width/2;

    for(size_t y=0; y<img->height; y+=double_block_size)
    {
        INCREMENT_FLOP_COUNT(1,0,0,0)

        for (size_t x=0; x<img->width; x+=double_block_size)
        {
            INCREMENT_FLOP_COUNT(3,0,0,0)

            int x_half=x/2;
            int y_half=y/2;
            INCREMENT_FLOP_COUNT(4,0,0,0)

            u_int32_t domain_avg0=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half, block_size);
            u_int32_t domain_avg1=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half, half_block_size);
            u_int32_t domain_avg2=get_average_pixel(img_channels_downsampled, half_width, x_half+half_block_size, y_half, half_block_size);
            u_int32_t domain_avg3=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half+half_block_size, half_block_size);
            u_int32_t domain_avg4=get_average_pixel(img_channels_downsampled, half_width, x_half+half_block_size, y_half+half_block_size, half_block_size);

            for(int transformation_type=0; transformation_type<SYM_MAX; ++transformation_type)
            {
                INCREMENT_FLOP_COUNT(6,0,0,0)

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
                    INCREMENT_FLOP_COUNT(2,0,0,0)

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
        INCREMENT_FLOP_COUNT(1,0,0,0)
        u_int32_t quater_block_size=half_block_size/2;
        if(error_1<threshold){
            ifs_trans_push_back(transformations, &best_ifs_1);
        }else{
            INCREMENT_FLOP_COUNT(4,0,0,0)
            find_matches_for(img, transformations, rb_x0, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0, rb_y0+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y0+quater_block_size, quater_block_size, threshold);
        }

        if(error_2<threshold){
            ifs_trans_push_back(transformations, &best_ifs_2);
        }else{
            INCREMENT_FLOP_COUNT(4,0,0,0)

            find_matches_for(img, transformations, rb_x1, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1, rb_y0+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y0+quater_block_size, quater_block_size, threshold);
        }

        if(error_3<threshold){
            ifs_trans_push_back(transformations, &best_ifs_3);
        }else{
            INCREMENT_FLOP_COUNT(4,0,0,0)

            find_matches_for(img, transformations, rb_x0, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0, rb_y1+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y1+quater_block_size, quater_block_size, threshold);
        }

        if(error_4<threshold){
            ifs_trans_push_back(transformations, &best_ifs_4);
        }else{
            INCREMENT_FLOP_COUNT(4,0,0,0)

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

ERR_RET match_blocks(struct Transforms* transformations, struct image_data* src, struct image_data* img,
                     u_int32_t threshold, int number_of_channels, size_t size, bool usingYcbCr,
                     pixel_value* allocated_block_buffer, size_t max_buffer_size, size_t min_buffer_size){

    buffer=allocated_block_buffer;
    MIN_BUFFER_SIZE=min_buffer_size;
    MAX_BUFFER_SIZE=max_buffer_size;

    size_t half_size=size>>1;

    for (size_t channel = 0; channel < number_of_channels; channel++){
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)

        img->image_channels[0] = src->image_channels[channel];
        down_sample(img->image_channels[0], size, 0,0, half_size, img->image_channels[1]);

        if (channel >= 1 && usingYcbCr){
            threshold *= 2;
            INCREMENT_FLOP_COUNT(1, 0, 0, 0)
        }

        for (size_t y = 0; y < size; y += MAX_BUFFER_SIZE)
        {
            INCREMENT_FLOP_COUNT(0, 1, 0, 0)

            for (size_t x = 0; x < size; x += MAX_BUFFER_SIZE)
            {
                INCREMENT_FLOP_COUNT(0, 1, 0, 0)
                find_matches_for(img, transformations->ch+channel, x, y, MAX_BUFFER_SIZE, threshold);
                INCREMENT_FLOP_COUNT(0, 1, 0, 0)
                #ifdef DEBUG
                printf(".");
                #endif
            }
            #ifdef DEBUG
            printf("\n");
            #endif
        }

        if (channel >= 1 && usingYcbCr){
            INCREMENT_FLOP_COUNT(1, 0, 0, 0)
            threshold /= 2;
        }
    }

    return ERR_SUCCESS;
}
