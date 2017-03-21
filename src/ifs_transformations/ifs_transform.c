#include "ifs_transform.h"

ERR_RET down_sample(pixel_value *src, int src_width, int start_x, int start_y, int target_size, pixel_value* sample) {
    sample = malloc((target_size * target_size)*sizeof(pixel_value));
    int dest_x = 0;
    int dest_y = 0;

    for (int y = start_y; y < start_y + target_size * 2; y += 2) {
        for (int x = start_x; x < start_x + target_size * 2; x += 2) {
            // Perform simple 2x2 average
            uint8_t pixel = 0;
            pixel += src[y * src_width + x];
            pixel += src[y * src_width + (x + 1)];
            pixel += src[(y + 1) * src_width + x];
            pixel += src[(y + 1) * src_width + (x + 1)];
            pixel /= 4;

            sample[dest_y * target_size + dest_x] = pixel;
            dest_x++;
        }
        dest_y++;
        dest_x = 0;
    }
    return ERR_SUCCESS;
}

ERR_RET ifs_trans_push_back(struct ifs_transformation_list* list, struct ifs_transformation* transformation){
    if(list->head==NULL && list->tail==NULL){
        list->head=transformation;
        list->tail=transformation;
    }else{
        list->tail->next=transformation;
        list->tail=transformation;
    }

    return ERR_SUCCESS;
}

ERR_RET ifs_transformation_execute(struct ifs_transformation* transformation, pixel_value* src, u_int32_t src_width,
                                   pixel_value* dest, u_int32_t dest_width, bool downsampled){

}
