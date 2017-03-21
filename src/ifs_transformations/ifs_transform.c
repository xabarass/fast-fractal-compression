#include "ifs_transform.h"

//static PixelValue *DownSample(pixel_value *src, int srcWidth, int startX, int startY, int targetSize) {
//    pixel_value * dest = malloc((targetSize * targetSize)*sizeof(pixel_value));
//    int destX = 0;
//    int destY = 0;

//    for (int y = startY; y < startY + targetSize * 2; y += 2) {
//        for (int x = startX; x < startX + targetSize * 2; x += 2) {
//            // Perform simple 2x2 average
//            uint8_t pixel = 0;
//            pixel += src[y * srcWidth + x];
//            pixel += src[y * srcWidth + (x + 1)];
//            pixel += src[(y + 1) * srcWidth + x];
//            pixel += src[(y + 1) * srcWidth + (x + 1)];
//            pixel /= 4;

//            dest[destY * targetSize + destX] = pixel;
//            destX++;
//        }
//        destY++;
//        destX = 0;
//    }

//    return dest;
//}

ERR_RET ifs_trans_push_back(struct ifs_transformation_list* list, struct ifs_transformation* transformation){
    struct ifs_transformation* new_transformation=(struct ifs_transformation*)malloc(sizeof(struct ifs_transformation));
    memcpy(new_transformation, transformation, sizeof(struct ifs_transformation));
    if(list->head==NULL && list->tail==NULL){
        list->head=new_transformation;
        list->tail=new_transformation;
    }else{
        list->tail->next=new_transformation;
        list->tail=new_transformation;
    }

    return ERR_SUCCESS;
}

ERR_RET ifs_transformation_execute(struct ifs_transformation* transformation, pixel_value* src, u_int32_t src_width,
                                   pixel_value* dest, u_int32_t dest_width, bool downsampled){

}
