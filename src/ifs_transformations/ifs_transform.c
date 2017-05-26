#include "ifs_transform.h"

#define MODULE_NAME "ifsTransform"

ERR_RET down_sample(pixel_value *src, int src_width, int start_x, int start_y, int target_size, pixel_value *sample) {

    uint32_t dest_x = 0;
    uint32_t dest_y = 0;
    INCREMENT_FLOP_COUNT(2, 2, 0, 0)
    int limit_x = start_x + target_size * 2;
    int limit_y = start_y + target_size * 2;

    __m256i zeros = _mm256_set1_epi32(0);
    __m256i mask = _mm256_set_epi32( 0, 0, 0, 0,-1, -1, -1, -1);

    for (int y = start_y; y < limit_y; y += 2) {
        int x = start_x;
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (; x+32 <= limit_x;  dest_x += 16, x += 32) {
            INCREMENT_FLOP_COUNT(0, 3, 0, 0)

            INCREMENT_FLOP_COUNT(1, 2, 0, 0)
            uint32_t addr1 = y * src_width + x;
            uint32_t addr2 = addr1 + src_width;

            INCREMENT_FLOP_COUNT(0, 2, 0, 0)
            __m256i src1 = _mm256_loadu_si256(src + addr1);
            __m256i src2 = _mm256_loadu_si256(src + addr2);

            __m256i src1_1_16 = _mm256_srli_epi16(src1, 10);
            __m256i src1_2_16 = _mm256_slli_epi16(src1, 8);
            src1_2_16 = _mm256_srli_epi16(src1_2_16, 10);

            __m256i src2_1_16 = _mm256_srli_epi16(src2, 10);
            __m256i src2_2_16 = _mm256_slli_epi16(src2, 8);
            src2_2_16 = _mm256_srli_epi16(src2_2_16, 10);

            INCREMENT_FLOP_COUNT(0, 16, 0, 0)
            __m256i sum0 = _mm256_add_epi16(src1_1_16, src1_2_16);

            INCREMENT_FLOP_COUNT(0, 16, 0, 0)
            __m256i sum1 = _mm256_add_epi16(src2_1_16, src2_2_16);

            INCREMENT_FLOP_COUNT(0, 16, 0, 0)
            __m256i sum_total = _mm256_add_epi16(sum0, sum1);

            __m256i temp1 = _mm256_packus_epi16(sum_total, zeros);

            __m256i ans = _mm256_permute4x64_epi64(temp1, 0b01011000);

            INCREMENT_FLOP_COUNT(1, 2, 0, 0)
            _mm256_maskstore_epi32(sample + dest_y * target_size + dest_x, mask, ans);
        }
        for (; x < limit_x; x += 2) {
            uint32_t pixel = 0;

            INCREMENT_FLOP_COUNT(1, 1, 0, 0)
            int index_x = y * src_width + x;

            INCREMENT_FLOP_COUNT(1, 2, 0, 0)
            int index_y = (y + 1) * src_width + x;

            INCREMENT_FLOP_COUNT(0, 6, 0, 0)
            pixel += src[index_x];
            pixel += src[index_x + 1];
            pixel += src[index_y];
            pixel += src[index_y + 1];

            pixel = pixel >> 2;

            INCREMENT_FLOP_COUNT(1, 2, 0, 0)
            sample[dest_y * target_size + dest_x] = pixel;
            dest_x++;
        }

        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        dest_y++;
        dest_x = 0;
    }
    return ERR_SUCCESS;
}

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

    INCREMENT_FLOP_COUNT(0, 1, 0, 0);

    int element=list->elements++;
    memcpy(list->array+element, transformation, sizeof(struct ifs_transformation));

    return ERR_SUCCESS;
}

ERR_RET ifs_trans_clear_list(struct Transforms *transforms){
//    INCREMENT_FLOP_COUNT(0, channels * 2, 0, 0);
//    for(int i=0;i<transforms->channels;++i){
//        struct ifs_transformation_list* tr_list=transforms->ch+i;
//        free(tr_list->array);
//        tr_list->elements=0;
//    }

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
        from_x += transformation->size - 1;
        d_x = -1;
    }

    if (!isPositiveY(symmetry)) {
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
        for (int to_x = transformation->to_x; to_x < (transformation->to_x + transformation->size); to_x++) {
            int pixel = src[from_y * src_width + from_x];
            pixel = (int) (scale * pixel) + offset;

            if (pixel < 0)
                pixel = 0;
            if (pixel > 255)
                pixel = 255;

            dest[to_y * dest_width + to_x] = pixel;

            if (in_order)
                from_x += d_x;
            else
                from_y += d_y;
        }

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
