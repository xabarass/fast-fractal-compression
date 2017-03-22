#include "Decoder.h"

Decoder::Decoder(){

}

void Decoder::Decode(Transforms* transform, Image& result){
    qtree_decode(transform, result.GetHeight(), result.GetWidth(), &result.img);

}
