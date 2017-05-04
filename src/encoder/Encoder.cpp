#include "Encoder.h"

Encoder::Encoder(){}

void Encoder::Encode(Image &src, Transforms* transformation, u_int32_t threshold, bool useYCbCr){
    struct encoder_params params;
    params.use_ycbcr=useYCbCr;
    qtree_encode(transformation, &src.img, params, threshold);
}
