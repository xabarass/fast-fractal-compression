#include "QuadTreeEncoder.h"

#define MODULE_NAME "QuadTreeEncoder"

ERR_RET qtree_encode(struct image_data* src, struct image_data* dst, struct encoder_params params){
    filter_grayscale(src,dst);

    struct image_tile_list tile_list;
    tile_rectengular(dst, 8, 8, &tile_list);

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
