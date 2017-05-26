#include "QuadTreeDecoder.h"

#define MODULE_NAME "QTDecoder"

ERR_RET print_transformation(struct ifs_transformation best_ifs) {
    printf("to=(%d, %d)\n", best_ifs.to_x, best_ifs.to_y);
    printf("from=(%d, %d)\n", best_ifs.from_x, best_ifs.from_y);
    printf("best symmetry=%d\n", best_ifs.transformation_type);
    printf("best offset=%d\n", best_ifs.offset);
    printf("\n");
    return ERR_SUCCESS;
}

void qtree_decode(struct Transforms* transforms, int height, int width, struct image_data* destination, pixel_value* buffer) {
    // Decoding starts here
    destination->channels = transforms->channels;
    for (int channel = 0; channel < destination->channels; channel++) {
        pixel_value *original_image = destination->image_channels[channel];

        // Iterate over Transforms
        struct ifs_transformation_list iter = transforms->ch[channel];
        struct ifs_transformation* temp = iter.array;
        size_t elements=iter.elements;
        for(int i=0;i<elements; ++i){
            ifs_transformation_execute(temp+i, original_image, destination->width, original_image, destination->width, false, buffer);
        }

    }
}
