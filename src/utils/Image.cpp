#include "Image.h"

Image::Image(string fileName){
    this->fileName=fileName;
    if(init_image_data(&img, 0, 0, 0)!=ERR_SUCCESS){
        throw runtime_error("Cannot create image!");
    }
}

Image::Image(string fileName, unsigned width, unsigned height, unsigned channels){
    this->fileName = fileName;
    if(init_image_data(&img, width, height, channels)!=ERR_SUCCESS){
        throw runtime_error("Cannot create image!");
    }
}

Image::~Image(){
    clear_image_data(&img);
}

void Image::GetChannelData(int channel, PixelValue* buffer, unsigned int bufferSize)
{
    if (channel > img.channels || channel <= 0){
        throw runtime_error("Channel index out of range!");
    }

    if(img.image_channels[channel]==NULL){
        throw runtime_error("Channel index not available!");
    }

    unsigned imageSize=img.width * img.height;
    if (imageSize > bufferSize){
        throw runtime_error("Buffer too small!");   //That's what she said!
    }

    memcpy(buffer, img.image_channels[channel], imageSize);
}

void Image::SetChannelData(int channel, PixelValue* buffer, unsigned size)
{
    if (channel > img.channels || channel <= 0){
        throw runtime_error("Channel index out of range!");
    }

    unsigned imageSize=img.width*img.height;
    if(imageSize<size){
        throw runtime_error("Buffer too small!");
    }

    memcpy(img.image_channels[channel], buffer, size);
}

int Image::GetWidth()
{
    return img.width;
}

int Image::GetHeight()
{
    return img.height;
}

int Image::GetChannels()
{
    return img.channels;
}
