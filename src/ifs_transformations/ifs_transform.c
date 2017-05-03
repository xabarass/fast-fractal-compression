#include "ifs_transform.h"

#define MODULE_NAME "ifsTransform"

ERR_RET down_sample(pixel_value *src, int src_width, int start_x, int start_y, int target_size, pixel_value* sample) {
    int dest_x = 0;
    int dest_y = 0;

    for (int y = start_y; y < start_y + target_size * 2; y += 2) {
        for (int x = start_x; x < start_x + target_size * 2; x += 2) {
            // Perform simple 2x2 average
            uint32_t pixel = 0;
//            printf("Down_Sample: y: %d x: %d src_width: %d\n", y, x, src_width);
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
    struct ifs_transformation* new_transformation=(struct ifs_transformation*)malloc(sizeof(struct ifs_transformation));
    memcpy(new_transformation, transformation, sizeof(struct ifs_transformation));
    new_transformation->next=NULL;
    if(list->head==NULL && list->tail==NULL){
        list->head=new_transformation;
        list->tail=new_transformation;
    }else{
        assert(list->tail!=NULL);
        list->tail->next=new_transformation;
        list->tail=new_transformation;
    }
    list->elements++;
    return ERR_SUCCESS;
}

ERR_RET ifs_transformation_execute(struct ifs_transformation* transformation, pixel_value* src, u_int32_t src_width,
                                   pixel_value* dest, u_int32_t dest_width, bool downsampled){

    int from_x = transformation->from_x / 2;
    int from_y = transformation->from_y / 2;
    int d_x = 1;
    int d_y = 1;
    enum ifs_type symmetry=transformation->transformation_type;
    bool in_order = isScanlineOrder(symmetry);
    // printf("from_x: %d, from_y: %d, d_x: %d, d_y: %d, in_order: %d\n",
    //         from_x, from_y, d_x, d_y, in_order);
    if (!downsampled)
    {
        pixel_value* downsampled_img=(pixel_value*)malloc(transformation->size*transformation->size*sizeof(pixel_value));
        down_sample(src, src_width, transformation->from_x, transformation->from_y, transformation->size, downsampled_img);
        src = downsampled_img;
        src_width = transformation->size;
        from_y = from_x = 0;
    }

    if (!isPositiveX(symmetry))
    {
        from_x += transformation->size - 1;
        d_x = -1;
    }

    if (!isPositiveY(symmetry))
    {
        from_y += transformation->size - 1;
        d_y = -1;
    }

    int start_x = from_x;
    int start_y = from_y;

    for (int to_y = transformation->to_y; to_y < (transformation->to_y +  transformation->size); to_y++)
    {
        for (int to_x = transformation->to_x; to_x < (transformation->to_x + transformation->size); to_x++)
        {

//            printf("Execute: y: %d x: %d src_width: %d\n", from_y, from_x, src_width);
            int pixel = src[from_y * src_width+ from_x];
            pixel = (int)(transformation->scale * pixel) + transformation->offset;

            if (pixel < 0)
                pixel = 0;
            if (pixel > 255)
                pixel = 255;

            dest[to_y * dest_width+ to_x] = pixel;

            if (in_order)
                from_x += d_x;
            else
                from_y += d_y;
        }

        if (in_order)
        {
            from_x= start_x;
            from_y += d_y;
        }
        else
        {
            from_y = start_y;
            from_x += d_x;
        }
    }

    if (!downsampled)
    {
        free(src);
        src = NULL;
    }

    return ERR_SUCCESS;

}

bool isScanlineOrder(enum ifs_type symmetry)
{
    return (
        symmetry == SYM_NONE ||
        symmetry == SYM_R180 ||
        symmetry == SYM_HFLIP ||
        symmetry == SYM_VFLIP
    );
}

bool isPositiveX(enum ifs_type symmetry)
{
    return (
        symmetry == SYM_NONE ||
        symmetry == SYM_R90 ||
        symmetry == SYM_VFLIP ||
        symmetry == SYM_RDFLIP
    );
}

bool isPositiveY(enum ifs_type symmetry)
{
    return (
        symmetry == SYM_NONE ||
        symmetry == SYM_R270 ||
        symmetry == SYM_HFLIP ||
        symmetry == SYM_RDFLIP
    );
}
