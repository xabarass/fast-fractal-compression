#include "Encoder.h"

Encoder::Encoder(){}

void Encoder::Encode(Image &src, Image &dst, struct Transforms* transformation){
    struct encoder_params params;
    qtree_encode(transformation, &src.img, &dst.img, params);
}
