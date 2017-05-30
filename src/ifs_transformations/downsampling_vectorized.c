#include "ifs_transform.h"

ERR_RET down_sample(pixel_value *src, int src_width, int start_x, int start_y, int target_size, pixel_value *sample) {
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
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
#ifdef COUNT_DETAIL_CYCLES
    int64_t cycles = cycles_count_stop();
    INCREMENT_CYCLE_COUNT(0,0,0,cycles,0)
#endif
    return ERR_SUCCESS;
}

