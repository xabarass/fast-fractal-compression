#ifndef ENCODER_H
#define ENCODER_H

#include <utils/Image.h>

extern "C"{
    #include <encoder/QuadTreeEncoder.h>
}

class Encoder
{
public:
    Encoder();

    void Encode(Image& src, Image &dst, struct Transforms* transformation);
};

#endif // ENCODER_H
