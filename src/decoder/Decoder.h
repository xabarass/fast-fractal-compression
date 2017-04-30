#ifndef DECODER_H
#define DECODER_H

#include <utils/Image.h>
#include <ifs_transformations/ifs_transform.h>

extern "C"{
    #include <decoder/QuadTreeDecoder.h>
}

class Decoder {
    public:
        Decoder();
        void Decode(Transforms* transform, Image &result, int maxphases);
};

#endif // DECODER_H
