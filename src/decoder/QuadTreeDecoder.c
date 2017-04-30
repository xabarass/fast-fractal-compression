#include "QuadTreeDecoder.h"

#define MODULE_NAME "QTDecoder"

ERR_RET print_transformation(struct ifs_transformation best_ifs) {
    printf("to=(%d, %d)\n", best_ifs.to_x, best_ifs.to_y);
    printf("from=(%d, %d)\n", best_ifs.from_x, best_ifs.from_y);
    printf("best symmetry=%d\n", best_ifs.transformation_type);
    printf("best offset=%d\n", best_ifs.offset);
    printf("best scale=%lf\n", best_ifs.scale);
    return ERR_SUCCESS;
}

void qtree_decode(struct Transforms* transforms, int height, int width, struct image_data* destination) {
    // Decoding starts here
    destination->channels = transforms->channels;
    printf("channels: %d\n", destination->channels);
    for (int channel = 0; channel < destination->channels; channel++) {
        pixel_value *original_image = destination->image_channels[channel];

        // Iterate over Transforms
        struct ifs_transformation_list iter = transforms->ch[channel];
        printf("Number of transformation: %d\n", iter.elements);
        struct ifs_transformation* temp = iter.head;
        while(temp != NULL) {
            // print_transformation(*temp);
            ifs_transformation_execute(temp, original_image, destination->width, original_image, destination->width, false);
            // Print the destination image
            temp = temp->next;
        }
    }
    printf("Ended\n");
}
