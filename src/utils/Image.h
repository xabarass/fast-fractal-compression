#ifndef IMAGE_H
#define IMAGE_H

#include <common.h>
#include <string>
#include <stdexcept>
#include <string.h>

extern "C"{
    #include <utils/ImageData.h>
}

typedef pixel_value PixelValue;

using std::runtime_error;
using std::string;

class Encoder;
class Decoder;

class Image
{
public:
    Image(string fileName, unsigned width, unsigned height, unsigned channels);
    Image(string fileName);

    virtual ~Image();

    virtual void Load()=0;

    virtual void Save()=0;

    void GetChannelData(int channel, PixelValue *buffer, unsigned int bufferSize);

    void SetChannelData(int channel, PixelValue *buffer, unsigned size);

    int GetWidth();

    int GetHeight();

    int GetChannels();

    int64_t getSize();
protected:
    string fileName;
    struct image_data img;

    friend class Encoder;
    friend class Decoder;
};

#endif // IMAGE_H
