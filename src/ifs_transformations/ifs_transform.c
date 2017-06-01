#include "ifs_transform.h"

#define MODULE_NAME "ifsTransform"

#define MAX_NUMBER_OF_TRANSFORMATIONS (1024+4096+16384+65536)
static struct ifs_transformation CHANNELS[3][MAX_NUMBER_OF_TRANSFORMATIONS];

ERR_RET ifs_trans_init_transformations(struct Transforms* transforms, int number_of_channels){
    for(int i=0; i<number_of_channels;++i){
        transforms->ch[i].array=CHANNELS[i];
        transforms->ch[i].elements=0;
    }

    return ERR_SUCCESS;
}

ERR_RET ifs_trans_push_back(struct ifs_transformation_list* list, struct ifs_transformation* transformation){

    INCREMENT_FLOP_COUNT(0, 2, 0, 0);

    int element=list->elements++;
    memcpy(list->array+element, transformation, sizeof(struct ifs_transformation));

    return ERR_SUCCESS;
}

ERR_RET ifs_transformation_execute(struct ifs_transformation *transformation, pixel_value *src, u_int32_t src_width,
                                   pixel_value *dest, u_int32_t dest_width, bool downsampled, pixel_value *buffer) {
    INCREMENT_FLOP_COUNT(6, 0, 0, 0)

    int from_x = transformation->from_x / 2;
    int from_y = transformation->from_y / 2;
    int d_x = 1;
    int d_y = 1;
    enum ifs_type symmetry = transformation->transformation_type;
    bool in_order = isScanlineOrder(symmetry);
    double scale = transformation->scale;
    u_int32_t offset = transformation->offset;

    if (!downsampled) {
        pixel_value *downsampled_img = buffer;
        down_sample(src, src_width, transformation->from_x, transformation->from_y, transformation->size,
                    downsampled_img);
        src = downsampled_img;
        src_width = transformation->size;
        from_y = from_x = 0;
    }

    if (!isPositiveX(symmetry)) {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        from_x += transformation->size - 1;
        d_x = -1;
    }

    if (!isPositiveY(symmetry)) {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        from_y += transformation->size - 1;
        d_y = -1;
    }

    int start_x = from_x;
    int start_y = from_y;

    INCREMENT_FLOP_COUNT(2 * transformation->size * transformation->size,
                         4 * transformation->size * transformation->size, transformation->size * transformation->size,
                         0)
    INCREMENT_FLOP_COUNT(transformation->size, 0, 0, 0)
    for (int to_y = transformation->to_y; to_y < (transformation->to_y + transformation->size); to_y++) {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (int to_x = transformation->to_x; to_x < (transformation->to_x + transformation->size); to_x++) {
            INCREMENT_FLOP_COUNT(2, 3, 0, 0)
            int pixel = src[from_y * src_width + from_x];
            pixel = (int) (scale * pixel) + offset;

            if (pixel < 0)
                pixel = 0;
            if (pixel > 255)
                pixel = 255;
            INCREMENT_FLOP_COUNT(1, 2, 0, 0)
            dest[to_y * dest_width + to_x] = pixel;
            if (in_order)
                from_x += d_x;
            else
                from_y += d_y;
        }
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        if (in_order) {
            from_x = start_x;
            from_y += d_y;
        } else {
            from_y = start_y;
            from_x += d_x;
        }
    }

    return ERR_SUCCESS;
}

bool isScanlineOrder(enum ifs_type symmetry) {
    return (
            symmetry == SYM_NONE ||
            symmetry == SYM_R180 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_VFLIP
    );
}

bool isPositiveX(enum ifs_type symmetry) {
    return (
            symmetry == SYM_NONE ||
            symmetry == SYM_R90 ||
            symmetry == SYM_VFLIP ||
            symmetry == SYM_RDFLIP
    );
}

bool isPositiveY(enum ifs_type symmetry) {
    return (
            symmetry == SYM_NONE ||
            symmetry == SYM_R270 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_RDFLIP
    );
}

