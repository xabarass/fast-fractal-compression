#ifndef QUADTREEENCODER_H
#define QUADTREEENCODER_H

#include <utils/ImageData.h>
#include <utils/EncoderUtils.h>
#include <ifs_transformations/ifs_transform.h>

struct encoder_params{
    u_int32_t compress_ratio;
};

ERR_RET qtree_encode(struct Transforms* transformed_channels, struct image_data* src, struct encoder_params params);
ERR_RET find_matches_for(struct image_data *img, struct ifs_transformation_list* transformations, u_int32_t to_x,
                         u_int32_t to_y, u_int32_t block_size, u_int32_t threshold);

#endif // QUADTREEENCODER_H
