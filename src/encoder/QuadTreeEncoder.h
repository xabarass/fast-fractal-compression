#ifndef QUADTREEENCODER_H
#define QUADTREEENCODER_H

#include <utils/ImageData.h>
#include <ifs_transformations/ifs_transform.h>
#include <utils/EncoderUtils.h>

struct encoder_params{
    u_int32_t compress_ratio;
    bool use_ycbcr;
};

ERR_RET qtree_encode(struct Transforms* transformed_channels, struct image_data* src, struct encoder_params params,
                     u_int32_t threshold_param);

ERR_RET print_best_transformation(struct ifs_transformation best_ifs, double best_err);

#endif // QUADTREEENCODER_H
