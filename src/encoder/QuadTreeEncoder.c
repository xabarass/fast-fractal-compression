#include "QuadTreeEncoder.h"

ERR_RET qtree_encode(struct image_data* img, struct encoder_params params){
    convert_from_RGB_to_YCbCr(img);

    for(int i=0;i<img->width*img->height;++i){
        img->image_channels[Cb][i]=0;
        img->image_channels[Cr][i]=0;
    }

    convert_from_YCbCr_to_RGB(img);

    return ERR_SUCCESS;
}
