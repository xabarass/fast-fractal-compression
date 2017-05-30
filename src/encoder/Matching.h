#ifndef MATCHING_H
#define MATCHING_H

#include <common.h>
#include <ifs_transformations/ifs_transform.h>

ERR_RET match_blocks(struct Transforms* transformations, struct image_data* src, struct image_data* img,
                     u_int32_t threshold, int number_of_channels, size_t size, bool usingYcbCr, pixel_value *allocated_block_buffer, size_t max_buffer_size);

#endif // MATCHING_H
