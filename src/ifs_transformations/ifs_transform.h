//
// Created by Anton on 20.03.17.
//

#ifndef FRACTALCOMPRESSION_IFS_TRANSFORM_H
#define FRACTALCOMPRESSION_IFS_TRANSFORM_H

#include <common.h>
#include <utils/ImageData.h>
#include <string.h>

enum ifs_type
{
    SYM_NONE = 0,
    SYM_R90=1,
    SYM_R180=2,
    SYM_R270=3,
    SYM_HFLIP=4,
    SYM_VFLIP=5,
    SYM_FDFLIP=6,
    SYM_RDFLIP=7,
    SYM_MAX
};

struct ifs_transformation{
    u_int32_t from_x;
    u_int32_t from_y;

    u_int32_t to_x;
    u_int32_t to_y;

    u_int32_t size;
    double scale;
    u_int32_t offset;

    enum ifs_type transformation_type;

    struct ifs_transformation* next;
};

struct ifs_transformation_list{
    struct ifs_transformation* head;
    struct ifs_transformation* tail;
    u_int32_t elements;
};

bool isScanlineOrder(enum ifs_type symmetry);
bool isPositiveX(enum ifs_type symmetry);
bool isPositiveY(enum ifs_type symmetry);

struct Transforms {
    struct ifs_transformation_list ch[3];
    int channels;
    enum color_mode color_mode;
    size_t max_block_size;
};

ERR_RET ifs_trans_push_back(struct ifs_transformation_list* list, struct ifs_transformation* transformation);
ERR_RET ifs_transformation_execute(struct ifs_transformation* transformation, pixel_value* src, u_int32_t src_width,
                                   pixel_value* dest, u_int32_t dest_width, bool downsampled, pixel_value *buffer);
ERR_RET ifs_trans_clear_list(struct Transforms* transforms);

ERR_RET down_sample(pixel_value *src, int src_width, int start_x, int start_y, int target_size, pixel_value * sample);

#endif //FRACTALCOMPRESSION_IFS_TRANSFORM_H
