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

#endif // ENCODERUTILS_H
