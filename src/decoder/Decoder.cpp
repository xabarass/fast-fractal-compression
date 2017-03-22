#include "Decoder.h"

Decoder::Decoder(){

}

void Decoder::Decode(Transforms* transform, int height, int width){

    qtree_decode(transform,height, width);

}
