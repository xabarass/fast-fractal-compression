#include "QuadTreeEncoder.h"
#include "EncoderMacros.h"
#include "Matching.h"
#include "EncoderTransformations.h"
#include "EncoderUtils.h"

ERR_RET qtree_encode(struct Transforms* transformations, struct image_data* src, struct encoder_params params,
                     u_int32_t threshold_param){

    struct image_data img;
    u_int32_t width=src->width;
    u_int32_t height=src->height;
    u_int32_t threshold=threshold_param;    //Temporary fixed value!

    if (width % 32 != 0 || height % 32 != 0)
    {
        return ERR_IMAGE_WRONG_SIZE;
    }

    if(width!=height){
        printf("ERROR! W!=H\n");
        return ERR_IMAGE_WRONG_SIZE;
    }

    static pixel_value* buffer;
    static size_t max_buffer_size;

    INCREMENT_FLOP_COUNT(4, 0, 0, 0)
    max_buffer_size=width/32;
    if(max_buffer_size<16)
           max_buffer_size=16;
    buffer=malloc(max_buffer_size*max_buffer_size*sizeof(pixel_value));
    transformations->max_block_size=max_buffer_size;

    if(params.use_ycbcr){
        if(src->color_mode!=COLOR_MODE_YCbCr){
            convert_from_RGB_to_YCbCr(src);
        }
        transformations->color_mode=COLOR_MODE_YCbCr;
    }else{
        transformations->color_mode=COLOR_MODE_RGB;
    }

    //We do our crazy stuff here
    /*
     * Make sense because you are only working with two channels
     * [0] -> Original channel data
     * [1] -> Downsampled channel data
     */
    // We allocate only one channel other one is just a pointer to original
    init_image_data(&img, width, height, 1);
    img.image_channels[1]=img.image_channels[0];
    transformations->channels=src->channels;

    match_blocks(transformations,src,&img,threshold,src->channels,width,params.use_ycbcr,buffer,max_buffer_size);

    img.image_channels[0]=img.image_channels[1];
    clear_image_data(&img);

    return ERR_SUCCESS;
}
