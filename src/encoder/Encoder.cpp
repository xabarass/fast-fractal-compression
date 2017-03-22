#include "Encoder.h"

Encoder::Encoder(){}

void Encoder::Encode(Image &src, Transforms** transformation){
    *transformation=(struct Transforms*)malloc(sizeof(struct Transforms));

    struct encoder_params params;
    qtree_encode(*transformation, &src.img, params);
}
