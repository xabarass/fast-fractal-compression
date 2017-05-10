#include "QuadTreeEncoder.h"

#define MODULE_NAME "QuadTreeEncoder"

ERR_RET qtree_encode(struct Transforms* transformations, struct image_data* src, struct encoder_params params,
                     u_int32_t threshold_param){

    struct image_data img;
    u_int32_t width=src->width;
    u_int32_t height=src->height;
    u_int32_t size=width*height;
    u_int32_t threshold=threshold_param;    //Temporary fixed value!

    if (width % 32 != 0 || height % 32 != 0)
    {
        return ERR_IMAGE_WRONG_SIZE;
    }

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
        img.image_channels[0]=src->image_channels[channel];
        down_sample(img.image_channels[0], width, 0,0, width/2, img.image_channels[1]);
        transformations->ch[channel].head=NULL;
        transformations->ch[channel].tail=NULL;
        transformations->ch[channel].elements=0;

        if (channel >= 1 && params.use_ycbcr)
            threshold *= 2;

        for (size_t y = 0; y < img.height; y += BUFFER_SIZE)
        {
            for (size_t x = 0; x < img.width; x += BUFFER_SIZE)
            {
                find_matches_for(&img, transformations->ch+channel, x, y, BUFFER_SIZE, threshold);
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

ERR_RET print_best_transformation(struct ifs_transformation best_ifs, double best_err) {
    printf("to=(%d, %d)\n", best_ifs.to_x, best_ifs.to_y);
    printf("from=(%d, %d)\n", best_ifs.from_x, best_ifs.from_y);
    printf("best error=%lf\n", best_err);
    printf("best symmetry=%d\n", best_ifs.transformation_type);
    printf("best offset=%d\n", best_ifs.offset);
    printf("best scale=%lf\n", best_ifs.scale);
    return ERR_SUCCESS;
}

ERR_RET find_matches_for(struct image_data* img, struct ifs_transformation_list* transformations, u_int32_t to_x,
                         u_int32_t to_y, u_int32_t block_size, u_int32_t threshold){

    assert(block_size<=BUFFER_SIZE);

    struct ifs_transformation best_ifs_transform;
    best_ifs_transform.transformation_type=SYM_NONE;
    double best_error = 1e9;

    static pixel_value buffer[BUFFER_SIZE*BUFFER_SIZE];

    u_int32_t range_avarage;
    get_average_pixel(img->image_channels[0], img->width, to_x, to_y, block_size, &range_avarage);
    for(size_t y=0; y<img->height; y+=block_size*2)
    {
        for (size_t x=0; x<img->width; x+=block_size*2)
        {
            for(int transformation_type=0; transformation_type<SYM_MAX; ++transformation_type)
            {
                enum ifs_type current_type=(enum ifs_type)transformation_type;

                struct ifs_transformation ifs={
                    .from_x=x,
                    .from_y=y,
                    .to_x=0,
                    .to_y=0,
                    .size=block_size,
                    .transformation_type=transformation_type,
                    .scale=1.0,
                    .offset=0
                };

                ifs_transformation_execute(&ifs, img->image_channels[1], img->width/2, buffer, block_size, true);

                u_int32_t domain_avg;
                get_average_pixel(buffer, block_size, 0, 0, block_size, &domain_avg);

                double scale_factor;
                get_scale_factor(img->image_channels[0], img->width, to_x, to_y, domain_avg,
                        buffer, block_size, 0 ,0, range_avarage, block_size, &scale_factor);
                int offset = (int)(range_avarage - scale_factor * (double)domain_avg);

                double error;
                get_error(buffer, block_size, 0,0,domain_avg,img->image_channels[0],
                        img->width, to_x, to_y, range_avarage, block_size, scale_factor, &error);

                if(error<best_error){
                    best_ifs_transform.from_x=x;
                    best_ifs_transform.from_y=y;
                    best_ifs_transform.to_x=to_x;
                    best_ifs_transform.to_y=to_y;
                    best_ifs_transform.transformation_type=current_type;
                    best_ifs_transform.scale=scale_factor;
                    best_ifs_transform.offset=offset;
                    best_ifs_transform.size=ifs.size;

                    best_error=error;
                }

                if(!transformation_type)
                    break;
            }

        }
    }

    if (block_size > 2 && best_error >= threshold)
    {
        // Recurse into the four corners of the current block.
        block_size /= 2;
        find_matches_for(img,transformations,to_x, to_y,block_size,threshold);
        find_matches_for(img,transformations,to_x+block_size, to_y,block_size,threshold);
        find_matches_for(img,transformations,to_x, to_y+block_size,block_size,threshold);
        find_matches_for(img,transformations,to_x+block_size, to_y+block_size,block_size,threshold);
    }
    else
    {
        // Use this transformation
        ifs_trans_push_back(transformations, &best_ifs_transform);
        // print_best_transformation(best_ifs_transform, best_error);
    }

    return ERR_SUCCESS;
}
