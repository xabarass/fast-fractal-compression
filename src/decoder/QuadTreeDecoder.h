#ifndef QUADTREEDECODER_H
#define QUADTREEDECODER_H

#include <ifs_transformations/ifs_transform.h>
#include <utils/ImageData.h>

void qtree_decode(struct Transforms* transforms, int height, int width, struct image_data *destination, pixel_value *buffer);
ERR_RET print_transformation(struct ifs_transformation best_ifs);

#endif // QUADTREEDECODER_H
