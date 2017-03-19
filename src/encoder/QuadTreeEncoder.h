#ifndef QUADTREEENCODER_H
#define QUADTREEENCODER_H

#include <utils/ImageData.h>

struct encoder_params{
    u_int32_t compress_ratio;
};

ERR_RET qtree_encode(struct image_data* img, struct encoder_params params);

#endif // QUADTREEENCODER_H
