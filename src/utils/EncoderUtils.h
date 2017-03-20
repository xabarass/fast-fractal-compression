#ifndef ENCODERUTILS_H
#define ENCODERUTILS_H

#include <utils/ImageData.h>
#include <frac_errors.h>

ERR_RET filter_grayscale(struct image_data* src, struct image_data* dst);
ERR_RET adjust_image_size(struct image_data* src, struct image_data* dst, u_int32_t rows, u_int32_t col);

#endif // ENCODERUTILS_H
