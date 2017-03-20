#include "QuadTreeEncoder.h"

ERR_RET qtree_encode(struct image_data* src, struct image_data* dst, struct encoder_params params){
//    convert_from_RGB_to_YCbCr(img);

    filter_grayscale(src,dst);

    return ERR_SUCCESS;
}
