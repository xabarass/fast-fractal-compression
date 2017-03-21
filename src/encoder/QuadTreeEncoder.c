#include "QuadTreeEncoder.h"

#define MODULE_NAME "QuadTreeEncoder"

ERR_RET qtree_encode(struct image_data* src, struct image_data* dst, struct encoder_params params){
    filter_grayscale(src,dst);

    struct image_tile_list tile_list;
    tile_rectengular(dst, 64, 64, &tile_list);

    for(size_t tile=0; tile<tile_list.size; ++tile){
        struct image_tile* current_tile=tile_list.tiles+tile;

        for(size_t y=current_tile->y; y<current_tile->y+current_tile->height; ++y){
            for(size_t x=y*tile_list.src_image->width+current_tile->x;
                x<y*tile_list.src_image->width+current_tile->x+current_tile->width;++x){

                if(tile%2)
                    tile_list.src_image->image_channels[R][x]=0;
            }
        }
    }

    return ERR_SUCCESS;
}

ERR_RET find_matches_for(struct image_data* img, struct ifs_transformation_list* transformations, u_int32_t to_x,
                         u_int32_t to_y, u_int32_t block_size, u_int32_t threshold){

    struct ifs_transformation best_ifs_transform;
    best_ifs_transform.transformation_type=SYM_NONE;
    double best_error = 1e9;

    pixel_value* buffer=(pixel_value*)malloc(block_size*block_size*sizeof(pixel_value));

    u_int32_t range_avarage;
    get_average_pixel(img->image_channels[0], img->width, to_x, to_y, block_size, &range_avarage);

    for(size_t y=0; y<img->height; y+=block_size*2){
        for (size_t x=0; x<img->width; x+=block_size*2){
            for(int transformation_type=0; transformation_type!=SYM_MAX; ++transformation_type){
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

                    best_error=error;
                }

                if(!transformations)
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
        ifs_trans_push_back(transformations,&best_ifs_transform);
    }

    return ERR_SUCCESS;
}
