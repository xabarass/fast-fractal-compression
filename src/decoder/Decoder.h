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
        ~Decoder();
        void Decode(ifs_transformation_list list, int height, int width);
};

#endif // DECODER_H