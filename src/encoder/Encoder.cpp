#include "Encoder.h"

Encoder::Encoder(){}

void Encoder::Encode(Image &src, Image &dst){
    struct encoder_params params;
    qtree_encode(&src.img, &dst.img, params);
}
