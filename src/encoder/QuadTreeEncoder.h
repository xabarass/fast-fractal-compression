#ifndef QUADTREEENCODER_H
#define QUADTREEENCODER_H

#include <utils/ImageData.h>
#include <utils/EncoderUtils.h>

struct encoder_params{
    u_int32_t compress_ratio;
};

ERR_RET qtree_encode(struct image_data* src,struct image_data *dst, struct encoder_params params);

#endif // QUADTREEENCODER_H
