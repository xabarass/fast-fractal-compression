#include "Encoder.h"

Encoder::Encoder(){}

void Encoder::Encode(Image &image){
    struct encoder_params params;
    qtree_encode(&image.img, params);
}
