#ifndef ENCODERUTILS_H
#define ENCODERUTILS_H

#include <common.h>
#include <utils/ImageData.h>
#include <frac_errors.h>

struct image_tile{
    u_int32_t x;
    u_int32_t y;
    u_int32_t width;
    u_int32_t height;
};

struct image_tile_list{
    struct image_data* src_image;
    struct image_tile* tiles;
    u_int32_t size;
};

ERR_RET filter_grayscale(struct image_data* src, struct image_data* dst);
ERR_RET adjust_image_size_down(struct image_data* src, struct image_data* dst, u_int32_t rows, u_int32_t col);

ERR_RET tile_rectengular(struct image_data* img, u_int32_t rows, u_int32_t columns, struct image_tile_list* tiles);


#endif // ENCODERUTILS_H
