#ifndef BMPIMAGE_H
#define BMPIMAGE_H

#include <utils/Image.h>

struct __attribute__ ((packed)) BMPHeader{
    BMPHeader();

    unsigned short int type;                 /* Magic identifier            */
    unsigned int size;                       /* File size in bytes          */
    unsigned short int reserved1, reserved2;
    unsigned int offset;                     /* Offset to image data, bytes */
};

struct __attribute__ ((packed)) BMPInformation{
    BMPInformation();

    unsigned int size;               /* Header size in bytes      */
    int width,height;                /* Width and height of image */
    unsigned short int planes;       /* Number of colour planes   */
    unsigned short int bits;         /* Bits per pixel            */
    unsigned int compression;        /* Compression type          */
    unsigned int imagesize;          /* Image size in bytes       */
    int xresolution,yresolution;     /* Pixels per meter          */
    unsigned int ncolours;           /* Number of colours         */
    unsigned int importantcolours;   /* Important colours         */
};

class BMPImage : public Image
{
public:
    BMPImage(string fileName);
    BMPImage(string fileName, unsigned width, unsigned height, unsigned channels);

    /**
     * @brief Loads image in RGB format
     */
    void Load();

    /**
     * @brief Saves image, expected state is RGB
     */
    void Save();
};

#endif // BMPIMAGE_H
