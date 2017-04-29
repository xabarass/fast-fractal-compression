#include "BMPImage.h"

#define MODULE_NAME "bmp_image"

BMPHeader::BMPHeader():type(0x4d42), reserved1(0x0), reserved2(0x0), offset(sizeof(BMPHeader)+sizeof(BMPInformation)),size(sizeof(BMPHeader)+sizeof(BMPInformation)){
}

BMPInformation::BMPInformation():size(sizeof(BMPInformation)){}

BMPImage::BMPImage(string fileName):Image(fileName){
}

BMPImage::BMPImage(string fileName, unsigned width, unsigned height, unsigned channels):
Image(fileName, width, height, channels){}

void BMPImage::Load(){

    FILE *f = fopen(fileName.c_str(), "rb");
    BMPHeader header;
    BMPInformation information;

    fread(&header,sizeof(BMPHeader),1,f);
    fread(&information,sizeof(BMPInformation),1,f);

    unsigned char channels=(information.bits/8);
    unsigned size=information.width*information.height*channels;
    unsigned char* data = new unsigned char[size];

    unsigned rowSize=information.width*channels;
    unsigned padding=(4-rowSize%4)%4;
    unsigned paddingPlaceholder;

    fseek(f, header.offset, SEEK_SET);
    for(int i=0; i<information.height; ++i){
        fread(data+(i*rowSize), 1, rowSize,f);
        fread(&paddingPlaceholder, 1, padding,f);
    }

    fclose(f);

    init_image_data(&img, information.width, information.height, channels);
    unsigned element=0;
    unsigned i=0;
    for (;i < size; element++)
    {
        for (int j=channels-1; j>=0; --j){   //we go in reverse order, RGB in BMP is in order B, G, R
            img.image_channels[j][element]=data[i++];
        }
    }
    printf("%d\n", convert_from_RGB_to_YCbCr(&img));

    delete []data;
}

void BMPImage::Save(){
    FILE *f = fopen(fileName.c_str(), "wb");

    BMPHeader header;
    unsigned rowSize=img.width*img.channels;
    unsigned padding=(4-rowSize%4)%4;
    rowSize+=padding;

    unsigned imageSize=img.height*rowSize;
    header.size+=imageSize;

    BMPInformation info;
    info.bits=img.channels*8;
    info.planes=1;
    info.compression=0;
    info.imagesize=imageSize;
    info.xresolution=0;
    info.yresolution=0;
    info.ncolours=1;
    info.importantcolours=0;
    info.width=img.width;
    info.height=img.height;

    LOGD("Channel bits: %d \n", info.bits);
    LOGD("Offset: %d \n", header.offset);
    LOGD("Size: %d w: %d h: %d \n", info.size, info.width, info.height);

    fwrite(&header,sizeof(BMPHeader), 1, f);
    fwrite(&info, sizeof(BMPInformation), 1, f);
    convert_from_YCbCr_to_RGB(&img);
    unsigned char* row=new unsigned char[rowSize];
    int pixelIndex=0;
    for (int i=0;i<info.height; ++i){
        for(int j=0;j<info.width; ++j, ++pixelIndex){

            int segment=j*img.channels;
            for (int c=img.channels-1; c>=0;--c){
                row[segment++]=img.image_channels[c][pixelIndex];
            }
        }
        fwrite(row, rowSize, 1, f);
    }

    fclose(f);
    delete []row;
}
