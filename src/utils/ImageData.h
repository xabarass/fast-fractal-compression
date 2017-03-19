#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <common.h>

typedef uint8_t pixel_value;

enum rgb_colors{
    R=0,
    G=1,
    B=2
};

enum rgb_YCbCr{
    Y=0,
    Cb=1,
    Cr=2
};

enum color_mode{
    COLOR_MODE_RGB,
    COLOR_MODE_YCbCr
};

void rgb_to_ycbcr(pixel_value r, pixel_value g, pixel_value b, pixel_value* y, pixel_value* cb, pixel_value* cr);
void ycbcr_to_rgb(pixel_value y, pixel_value cb, pixel_value cr, pixel_value* r, pixel_value* g, pixel_value* b);

struct image_data{
    pixel_value* image_channels[3];
    uint32_t width;
    uint32_t height;
    uint8_t channels;

    enum color_mode color_mode;
};

ERR_RET init_image_data(struct image_data* img, uint32_t width, uint32_t height, uint8_t channels);
ERR_RET clear_image_data(struct image_data* img);

ERR_RET convert_from_RGB_to_YCbCr(struct image_data* data);
ERR_RET convert_from_YCbCr_to_RGB(struct image_data* data);

#endif // IMAGEDATA_H
