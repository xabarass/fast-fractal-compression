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

ERR_RET get_average_pixel(pixel_value* domain_data, u_int32_t domain_width,
    u_int32_t domain_x, u_int32_t domain_y, u_int32_t size, u_int32_t *average_pixel);

ERR_RET get_scale_factor(
    pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
    pixel_value* range_Data, int range_width, int range_x, int range_y, int range_avg,
    int size, double* scale_factor);

ERR_RET get_error(
    pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
    pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg,
    int size, double scale, double* error);

#endif // ENCODERUTILS_H
