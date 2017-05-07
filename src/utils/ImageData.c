#include "ImageData.h"

#define MODULE_NAME "image_data"

static const u_int8_t MAX_CHANNELS=3;

inline void rgb_to_ycbcr(pixel_value r, pixel_value g, pixel_value b,
                  pixel_value* y, pixel_value* cb, pixel_value* cr){

    *y   = (   0.299  *r + 0.587  *g + 0.114  *b);
    *cb  = ( - 0.168736 *r - 0.3312 *g + 0.5    *b + 128 );
    *cr  = (   0.5    *r - 0.4187 *g - 0.0813 *b + 128 );
}

inline void ycbcr_to_rgb(pixel_value y, pixel_value cb, pixel_value cr,
                  pixel_value* r, pixel_value* g, pixel_value* b){

    *r = ( y                     + 1.402   *(cr-128) );
    *g = ( y - 0.34414 *(cb-128) - 0.71414 *(cr-128) );
    *b = ( y + 1.772   *(cb-128)                     );
}

ERR_RET init_image_data(struct image_data* img, uint32_t width, uint32_t height, uint8_t channels){
    if(channels>3){
        return ERR_INVALID_ARGUMENT;
    }

    img->width=width;
    img->height=height;
    img->channels=channels;

    uint32_t size=width*height*channels;

    if(size){
        pixel_value* buffer=malloc(size*sizeof(pixel_value));
        for(int i=0;i<channels;++i){
            img->image_channels[i]=buffer+(i*width*height);
        }
    }
    for(int i=channels;i<MAX_CHANNELS;++i){
        img->image_channels[i]=NULL;
    }
    return ERR_SUCCESS;
}

ERR_RET clear_image_data(struct image_data* img){
    free(img->image_channels[0]);
    return ERR_SUCCESS;
}

ERR_RET convert_from_RGB_to_YCbCr(struct image_data* data){
    if(data->color_mode!=COLOR_MODE_RGB){
        LOGE("Image not in RGB format, can't convert!");
        return ERR_INVALID_ARGUMENT;
    }
    INCREMENT_FLOP_COUNT(0,4*data->width*data->height, 4*data->width*data->height, 4*data->width*data->height)
    for (int i=0; i<data->width*data->height; ++i){
        rgb_to_ycbcr(data->image_channels[R][i], data->image_channels[G][i], data->image_channels[B][i],
                     data->image_channels[Y]+i, data->image_channels[Cb]+i, data->image_channels[Cr]+i);
    }

    data->color_mode=COLOR_MODE_YCbCr;
    return ERR_SUCCESS;
}

ERR_RET convert_from_YCbCr_to_RGB(struct image_data* data){
    if(data->color_mode!=COLOR_MODE_YCbCr){
        LOGE("Image not in YCbCr format, can't convert!");
        return ERR_INVALID_ARGUMENT;
    }
    INCREMENT_FLOP_COUNT(0,0, 9*data->width*data->height, 9*data->width*data->height)
    for (int i=0; i<data->width*data->height; ++i){
        ycbcr_to_rgb(data->image_channels[Y][i], data->image_channels[Cb][i], data->image_channels[Cr][i],
                     data->image_channels[R]+i, data->image_channels[G]+i, data->image_channels[B]+i);

    }

    data->color_mode=COLOR_MODE_RGB;
    return ERR_SUCCESS;
}
