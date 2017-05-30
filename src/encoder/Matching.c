#include "Matching.h"

#include "QuadTreeEncoder.h"
#include "EncoderUtils.h"
#include "EncoderMacros.h"
#include "EncoderTransformations.h"

static pixel_value* buffer;
static size_t MAX_BUFFER_SIZE;
static size_t MIN_BUFFER_SIZE;

static inline
ERR_RET find_matches_for(struct image_data* img, struct ifs_transformation_list* transformations, u_int32_t to_x,
                         u_int32_t to_y, u_int32_t block_size, u_int32_t threshold){

    assert(block_size<=MAX_BUFFER_SIZE);

    struct ifs_transformation best_ifs_transform;
    best_ifs_transform.transformation_type=SYM_NONE;
    double best_error = 1e9;

    INCREMENT_FLOP_COUNT(1, 0, 0, 0)
    u_int32_t range_avarage=get_average_pixel(img->image_channels[0], img->width, to_x, to_y, block_size);
    int increment = block_size * 2;

    int height = img->height;
    int width = img->width;

    for(size_t y=0; y < height; y+=increment)
    {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (size_t x=0; x < width; x+=increment)
        {
            u_int32_t domain_avg=get_average_pixel(img->image_channels[1], img->width>>1, x>>1, y>>1, block_size);

            INCREMENT_FLOP_COUNT(0, 1, 0, 0)
            for(int transformation_type=0; transformation_type<SYM_MAX; ++transformation_type)
            {
                INCREMENT_FLOP_COUNT(0, 2, 1, 0)

                ifs_transformation_execute_downsampled(x, y, transformation_type,
                                                            block_size, img->image_channels[1], img->width>>1,
                                                            buffer, block_size);

                double scale_factor=get_scale_factor(img->image_channels[0], img->width, to_x, to_y, domain_avg,
                        buffer, block_size, 0 ,0, range_avarage, block_size);
                int offset = (int)(range_avarage - scale_factor * (double)domain_avg);

                double error=get_error(buffer, block_size, 0,0,domain_avg,img->image_channels[0],
                        img->width, to_x, to_y, range_avarage, block_size, scale_factor);

                if(error<best_error){
                    ASSIGN_IFS_VALUES(best_ifs_transform, x, y, to_x, to_y, transformation_type, scale_factor, offset, block_size);
                    best_error=error;
                }
            }
        }
    }

    if (block_size > MIN_BUFFER_SIZE && best_error >= threshold)
    {
        // Recurse into the four corners of the current block.
        block_size = block_size >> 1;
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

ERR_RET match_blocks(struct Transforms* transformations, struct image_data* src, struct image_data* img,
                     u_int32_t threshold, int number_of_channels, size_t size, bool usingYcbCr,
                     pixel_value* allocated_block_buffer, size_t max_buffer_size){

    buffer=allocated_block_buffer;
    MAX_BUFFER_SIZE=max_buffer_size;
    MIN_BUFFER_SIZE=max_buffer_size>>3;

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
