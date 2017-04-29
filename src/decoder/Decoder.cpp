#include "Decoder.h"

Decoder::Decoder(){

}

void Decoder::Decode(Transforms* transform, Image& result){
    struct image_data* destination = &result.img;
    destination->width = result.GetWidth();
    destination->height = result.GetHeight();
    destination->channels = 3;

    for(int i = 0; i < destination->channels; i++) {
        for(int j = 0; j < destination->width * destination->height; j++) {
            destination->image_channels[i][j] = 127;
        }
    }

    qtree_decode(transform, result.GetHeight(), result.GetWidth(), destination);
}
