#ifndef ENCODERUTILS_H
#define ENCODERUTILS_H

#include <common.h>

static inline
u_int32_t get_average_pixel(const pixel_value* domain_data, u_int32_t domain_width,
                       u_int32_t domain_x, u_int32_t domain_y, u_int32_t size)
{
    // Simple average of all pixels.
        INCREMENT_FLOP_COUNT(1,0,0,0)
        uint32_t bottom = size*size;
        uint32_t top = 0;
        uint32_t size_copy = size;
        __m256i top1 = _mm256_set1_epi32(0);
        __m256i top2 = _mm256_set1_epi32(0);
        __m256i top3 = _mm256_set1_epi32(0);
        __m256i top4 = _mm256_set1_epi32(0);
        __m256i zeros = _mm256_set1_epi32(0);
        __m256i mask64_1 = _mm256_set_epi64x(0, 0, 0, 0xFFFFFFFFFFFFFFFF);
        __m256i mask64_2 = _mm256_set_epi64x(0, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
        uint32_t curr = 0;
        while(size >= 8) {

            if(size >= 32) {
                for(int y=0; y<size_copy; y++) {
                    INCREMENT_FLOP_COUNT(6,32,0,0)

                    uint32_t addr_domain = (domain_y+y)*domain_width + domain_x + curr;
                    //printf("HH %d %d\n", const1, const2);
                    __m256i domain_block = _mm256_loadu_si256(domain_data + addr_domain);
                    __m128i domain_block1 = _mm256_extracti128_si256(domain_block, 0);
                    __m128i domain_block2 = _mm256_extracti128_si256(domain_block, 1);
                    __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                    __m128i domain_block1_hi = _mm_unpackhi_epi64(domain_block1, _mm_setzero_si128());
                    __m256i domain_temp2 = _mm256_cvtepu8_epi32(domain_block1_hi);
                    __m256i domain_temp3 = _mm256_cvtepu8_epi32(domain_block2);
                    __m128i domain_block2_hi = _mm_unpackhi_epi64(domain_block2, _mm_setzero_si128());
                    __m256i domain_temp4 = _mm256_cvtepu8_epi32(domain_block2_hi);
                    top1 = _mm256_add_epi32(top1, domain_temp1);
                    top2 = _mm256_add_epi32(top2, domain_temp2);
                    top3 = _mm256_add_epi32(top3, domain_temp3);
                    top4 = _mm256_add_epi32(top4, domain_temp4);
                }
                INCREMENT_FLOP_COUNT(2,0,0,0)

                size = size - 32;
                curr+=32;
            }else if(size >= 16) {
                for(int y=0; y<size_copy; y++) {
                    INCREMENT_FLOP_COUNT(6,16,0,0)

                    uint32_t const1 = (domain_y+y)*domain_width + domain_x +curr;
                    __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_2);
                    __m256i domain_block_16_1 = _mm256_unpacklo_epi8(domain_block, zeros);
                    __m256i domain_block_16_2 = _mm256_unpackhi_epi8(domain_block, zeros);
                    __m256i domain_block_32_1 = _mm256_unpacklo_epi16(domain_block_16_1, zeros);
                    __m256i domain_block_32_2 = _mm256_unpackhi_epi16(domain_block_16_1, zeros);
                    __m256i domain_block_32_3 = _mm256_add_epi32(domain_block_32_1, domain_block_32_2);
                    __m256i domain_block_32_4 = _mm256_unpacklo_epi16(domain_block_16_2, zeros);
                    __m256i domain_block_32_5 = _mm256_unpackhi_epi16(domain_block_16_2, zeros);
                    __m256i domain_block_32_6 = _mm256_add_epi32(domain_block_32_4, domain_block_32_5);
                    top1 = _mm256_add_epi32(top1, domain_block_32_3);
                    top2 = _mm256_add_epi32(top2, domain_block_32_6);
                }
                INCREMENT_FLOP_COUNT(2,0,0,0)

                size = size - 16;
                curr+=16;
            }else if(size >= 8) {
                for(int y=0; y<size_copy; y++) {
                    INCREMENT_FLOP_COUNT(14,0,0,0)

                    uint32_t const1 = (domain_y+y)*domain_width + domain_x+curr;
                    __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_1);
                    __m256i domain_block_16 = _mm256_unpacklo_epi8(domain_block, zeros);
                    __m256i domain_block_32_1 = _mm256_unpacklo_epi16(domain_block_16, zeros);
                    __m256i domain_block_32_2 = _mm256_unpackhi_epi16(domain_block_16, zeros);
                    __m256i domain_block_32_3 = _mm256_add_epi32(domain_block_32_1, domain_block_32_2);
                    top1 = _mm256_add_epi32(top1, domain_block_32_3);
                }
                INCREMENT_FLOP_COUNT(2,0,0,0)

                size = size - 8;
                curr+=8;
            }
        }
        INCREMENT_FLOP_COUNT(42,0,0,0)

        __m256i temp_top1 = _mm256_add_epi32(top1, top2);
        __m256i temp_top3 = _mm256_add_epi32(top3, top4);
        __m256i temp_top13 = _mm256_add_epi32(temp_top1, temp_top3);
        // Bunch of hadd to get the final value
        __m256i temp_t1 = _mm256_hadd_epi32(temp_top13, zeros);
        __m256i temp_t2 = _mm256_hadd_epi32(temp_t1, zeros);
        uint32_t array_top[8];
        _mm256_store_si256(array_top, temp_t2);
        top = array_top[0] + array_top[4];
        for (size_t y = 0; y < size_copy; y++)
        {
            INCREMENT_FLOP_COUNT(1,0,0,0)

            for (size_t x = 0; x < size; x++)
            {
                INCREMENT_FLOP_COUNT(7,0,0,0)

                top += domain_data[(domain_y+y) * domain_width + domain_x + x +curr];
                if (top < 0)
                {
                    printf("Error: Accumulator rolled over averaging pixels.\n");
                    return 0;
                }
            }
        }
        return top/bottom;
}

static inline
void get_average_pixel_bulk(const pixel_value* domain_data, uint32_t domain_width,
                       uint32_t domain_x, uint32_t domain_y, uint32_t size, uint8_t* average_pixel)
{
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    uint32_t blocksize = size/2;
    uint32_t top = 0;
    INCREMENT_FLOP_COUNT(3, 0, 0, 0)
    uint32_t mid_bottom = blocksize * blocksize;
    uint32_t bottom = size*size;
    uint8_t index = 0;
    for(size_t i = 0; i < size; i += blocksize) {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for(size_t j = 0; j < size; j += blocksize) {
            INCREMENT_FLOP_COUNT(0, 1, 0, 0)
            uint32_t temp_top = 0;
            uint32_t blocksize_copy = blocksize;
            __m256i top1 = _mm256_set1_epi32(0);
            __m256i top2 = _mm256_set1_epi32(0);
            __m256i top3 = _mm256_set1_epi32(0);
            __m256i top4 = _mm256_set1_epi32(0);
            __m256i zeros = _mm256_set1_epi32(0);
            __m256i mask64_1 = _mm256_set_epi64x(0, 0, 0, 0xFFFFFFFFFFFFFFFF);
            __m256i mask64_2 = _mm256_set_epi64x(0, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
            uint32_t curr = 0;
            while(blocksize_copy >= 8) {
                if(blocksize_copy >= 32) {
                    for(int y = i; y< i+blocksize; y++) {
                        INCREMENT_FLOP_COUNT(32, 6, 0, 0)
                        uint32_t addr_domain = (domain_y+y)*domain_width + domain_x + curr + j;
                        __m256i domain_block = _mm256_loadu_si256(domain_data + addr_domain);
                        __m128i domain_block1 = _mm256_extracti128_si256(domain_block, 0);
                        __m128i domain_block2 = _mm256_extracti128_si256(domain_block, 1);
                        __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                        __m128i domain_block1_hi = _mm_unpackhi_epi64(domain_block1, _mm_setzero_si128());
                        __m256i domain_temp2 = _mm256_cvtepu8_epi32(domain_block1_hi);
                        __m256i domain_temp3 = _mm256_cvtepu8_epi32(domain_block2);
                        __m128i domain_block2_hi = _mm_unpackhi_epi64(domain_block2, _mm_setzero_si128());
                        __m256i domain_temp4 = _mm256_cvtepu8_epi32(domain_block2_hi);
                        top1 = _mm256_add_epi32(top1, domain_temp1);
                        top2 = _mm256_add_epi32(top2, domain_temp2);
                        top3 = _mm256_add_epi32(top3, domain_temp3);
                        top4 = _mm256_add_epi32(top4, domain_temp4);
                    }
                    INCREMENT_FLOP_COUNT(0, 2, 0, 0)
                    blocksize_copy -= 32;
                    curr += 32;
                }else if(blocksize_copy >= 16) {
                    for (int y = i; y < i+blocksize; y++) {
                        INCREMENT_FLOP_COUNT(0, 22, 0, 0)
                        uint32_t const1 = (domain_y + y) * domain_width + domain_x + curr + j;
                        __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_2);
                        __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                        __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                        __m128i domain_block1_hi = _mm_unpackhi_epi64(domain_block1, _mm_setzero_si128());
                        __m256i domain_temp2 = _mm256_cvtepu8_epi32(domain_block1_hi);
                        top1 = _mm256_add_epi32(top1, domain_temp1);
                        top2 = _mm256_add_epi32(top2, domain_temp2);
                    }
                    INCREMENT_FLOP_COUNT(0, 2, 0, 0)
                    blocksize_copy -= 16;
                    curr += 16;
                }else if(blocksize_copy >= 8) {
                    for(int y = i; y < i+blocksize; y++) {
                        INCREMENT_FLOP_COUNT(8, 4, 0, 0)
                        uint32_t const1 = (domain_y+y)*domain_width + domain_x+curr + j;
                        __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_1);
                        __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                        __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                        top1 = _mm256_add_epi32(top1, domain_temp1);
                    }
                    INCREMENT_FLOP_COUNT(0, 2, 0, 0)
                    blocksize_copy -= 8;
                    curr += 8;
                }
            }
            INCREMENT_FLOP_COUNT(0, 8*3, 0, 0)
            __m256i temp_top1 = _mm256_add_epi32(top1, top2);
            __m256i temp_top3 = _mm256_add_epi32(top3, top4);
            __m256i temp_top13 = _mm256_add_epi32(temp_top1, temp_top3);
            // Bunch of hadd to get the final value
            INCREMENT_FLOP_COUNT(0, 8, 0, 0)
            __m256i temp_t1 = _mm256_hadd_epi32(temp_top13, zeros);
            INCREMENT_FLOP_COUNT(0, 8, 0, 0)
            __m256i temp_t2 = _mm256_hadd_epi32(temp_t1, zeros);
            uint32_t array_top[8];
            __m256i mask1 = _mm256_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
            _mm256_maskstore_epi32(array_top, mask1, temp_t2);
            INCREMENT_FLOP_COUNT(0, 1, 0, 0)
            temp_top = array_top[0] + array_top[4];
            for (size_t y = i; y < i+blocksize; y++)
            {
                INCREMENT_FLOP_COUNT(0, 2, 0, 0)
                for (size_t x = 0; x < blocksize_copy; x++)
                {
                    INCREMENT_FLOP_COUNT(1, 6, 0, 0)
                    temp_top += domain_data[(domain_y+y) * domain_width + domain_x + x + curr +j];
                    if (temp_top < 0)
                    {
                        printf("Error: Accumulator rolled over averaging pixels.\n");
                        return;
                    }
                }
            }
            INCREMENT_FLOP_COUNT(0, 4, 0, 0)
            *(average_pixel+index) = temp_top/mid_bottom;
            top += temp_top;
            index++;
        }
    }
    INCREMENT_FLOP_COUNT(0, 1, 0, 0)
    *(average_pixel+index) = top/bottom;
    #ifdef COUNT_DETAIL_CYCLES
        int64_t cycles = cycles_count_stop();
        INCREMENT_CYCLE_COUNT(cycles,0,0,0,0)
    #endif
}

static inline
double get_error(
    pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
    pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg,
    int size, double scale)
{
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    uint32_t size_copy = size;
    __m256 top1 = _mm256_setzero_ps();
    __m256 top2 = _mm256_setzero_ps();
    __m256 top3 = _mm256_setzero_ps();
    __m256 top4 = _mm256_setzero_ps();

    __m256 scaling_factor = _mm256_set1_ps((float)scale);
    __m256 zeros = _mm256_setzero_ps();

    __m256i dom_avg = _mm256_set1_epi32(domain_avg);
    __m256i ran_avg = _mm256_set1_epi32(range_avg);

    double top = 0;
    INCREMENT_FLOP_COUNT(0, 1, 0, 0)
    double bottom = (double)(size * size);

    uint32_t curr = 0;
    __m256i mask64_1 = _mm256_set_epi64x(0, 0, 0, 0xFFFFFFFFFFFFFFFF);
    __m256i mask64_2 = _mm256_set_epi64x(0, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    while(size >= 8) {
        if(size >= 32) {
            for(int y=0; y<size_copy; y++) {
                INCREMENT_FLOP_COUNT(0, 10, 0, 0)
                uint32_t const1 = (domain_y+y)*domain_width + domain_x + curr;
                uint32_t const2 = (range_y+y)*range_width + range_x + curr;
                __m256i domain_block = _mm256_loadu_si256(domain_data + const1);
                __m256i range_block  = _mm256_loadu_si256(range_data + const2);

                __m128i domain_block1 = _mm256_extracti128_si256(domain_block, 0);
                __m128i domain_block2 = _mm256_extracti128_si256(domain_block, 1);

                __m128i range_block1 = _mm256_extracti128_si256(range_block, 0);
                __m128i range_block2 = _mm256_extracti128_si256(range_block, 1);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                __m128i domain_block1_hi = _mm_unpackhi_epi64(domain_block1, _mm_setzero_si128());
                __m256i domain_temp2 = _mm256_cvtepu8_epi32(domain_block1_hi);
                __m256i domain_temp3 = _mm256_cvtepu8_epi32(domain_block2);
                __m128i domain_block2_hi = _mm_unpackhi_epi64(domain_block2, _mm_setzero_si128());
                __m256i domain_temp4 = _mm256_cvtepu8_epi32(domain_block2_hi);

                __m256i range_temp1 = _mm256_cvtepu8_epi32(range_block1);
                __m128i range_block1_hi = _mm_unpackhi_epi64(range_block1, _mm_setzero_si128());
                __m256i range_temp2 = _mm256_cvtepu8_epi32(range_block1_hi);
                __m256i range_temp3 = _mm256_cvtepu8_epi32(range_block2);
                __m128i range_block2_hi = _mm_unpackhi_epi64(range_block2, _mm_setzero_si128());
                __m256i range_temp4 = _mm256_cvtepu8_epi32(range_block2_hi);

                INCREMENT_FLOP_COUNT(0, 4*8, 0, 0)
                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i domain2 = _mm256_sub_epi32(domain_temp2, dom_avg);
                __m256i domain3 = _mm256_sub_epi32(domain_temp3, dom_avg);
                __m256i domain4 = _mm256_sub_epi32(domain_temp4, dom_avg);

                INCREMENT_FLOP_COUNT(0, 4*8, 0, 0)
                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);
                __m256i range2 = _mm256_sub_epi32(range_temp2, ran_avg);
                __m256i range3 = _mm256_sub_epi32(range_temp3, ran_avg);
                __m256i range4 = _mm256_sub_epi32(range_temp4, ran_avg);

                __m256 fdomain1 = _mm256_cvtepi32_ps(domain1);
                __m256 fdomain2 = _mm256_cvtepi32_ps(domain2);
                __m256 fdomain3 = _mm256_cvtepi32_ps(domain3);
                __m256 fdomain4 = _mm256_cvtepi32_ps(domain4);

                __m256 frange1 = _mm256_cvtepi32_ps(range1);
                __m256 frange2 = _mm256_cvtepi32_ps(range2);
                __m256 frange3 = _mm256_cvtepi32_ps(range3);
                __m256 frange4 = _mm256_cvtepi32_ps(range4);


                INCREMENT_FLOP_COUNT(0, 0, 8*4, 8*4)
                __m256 diff1 = _mm256_fmsub_ps(scaling_factor, fdomain1, frange1);
                __m256 diff2 = _mm256_fmsub_ps(scaling_factor, fdomain2, frange2);
                __m256 diff3 = _mm256_fmsub_ps(scaling_factor, fdomain3, frange3);
                __m256 diff4 = _mm256_fmsub_ps(scaling_factor, fdomain4, frange4);

                INCREMENT_FLOP_COUNT(0, 0, 8*4, 8*4)
                top1 = _mm256_fmadd_ps(diff1, diff1, top1);
                top2 = _mm256_fmadd_ps(diff2, diff2, top2);
                top3 = _mm256_fmadd_ps(diff3, diff3, top3);
                top4 = _mm256_fmadd_ps(diff4, diff4, top4);
            }
            INCREMENT_FLOP_COUNT(0, 2, 0, 0)
            size = size - 32;
            curr+=32;
        }else if(size >= 16) {
            for(int y=0; y<size; y++) {
                INCREMENT_FLOP_COUNT(2, 5, 0, 0)
                uint32_t const1 = (domain_y+y)*domain_width + domain_x +curr;
                uint32_t const2 = (range_y+y)*range_width + range_x+curr;

                __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_2);
                __m256i range_block  = _mm256_maskload_epi64(range_data + const2, mask64_2);

                __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                __m128i range_block1 = _mm256_castsi256_si128(range_block);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                __m128i domain_block1_hi = _mm_unpackhi_epi64(domain_block1, _mm_setzero_si128());
                __m256i domain_temp2 = _mm256_cvtepu8_epi32(domain_block1_hi);

                __m256i range_temp1 = _mm256_cvtepu8_epi32(range_block1);
                __m128i range_block1_hi = _mm_unpackhi_epi64(range_block1, _mm_setzero_si128());
                __m256i range_temp2 = _mm256_cvtepu8_epi32(range_block1_hi);

                INCREMENT_FLOP_COUNT(16, 16, 0, 0)
                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i domain2 = _mm256_sub_epi32(domain_temp2, dom_avg);

                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);
                __m256i range2 = _mm256_sub_epi32(range_temp2, ran_avg);

                __m256 fdomain1 = _mm256_cvtepi32_ps(domain1);
                __m256 fdomain2 = _mm256_cvtepi32_ps(domain2);

                __m256 frange1 = _mm256_cvtepi32_ps(range1);
                __m256 frange2 = _mm256_cvtepi32_ps(range2);

                INCREMENT_FLOP_COUNT(0, 0, 8*2, 8*2)
                __m256 diff1 = _mm256_fmsub_ps(scaling_factor, fdomain1, frange1);
                __m256 diff2 = _mm256_fmsub_ps(scaling_factor, fdomain2, frange2);

                INCREMENT_FLOP_COUNT(0, 0, 8*2, 8*2)
                top1 = _mm256_fmadd_ps(diff1, diff1, top1);
                top2 = _mm256_fmadd_ps(diff2, diff2, top2);
            }
            INCREMENT_FLOP_COUNT(0, 2, 0, 0)
            size = size - 16;
            curr+=16;
        }else if(size >= 8) {
            for(int y=0; y<size; y++) {
                INCREMENT_FLOP_COUNT(0, 1, 0, 0)
                INCREMENT_FLOP_COUNT(2, 8, 0, 0)
                uint32_t const1 = (domain_y+y)*domain_width + domain_x + curr;
                uint32_t const2 = (range_y+y)*range_width + range_x + curr;
                __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_1);
                __m256i range_block  = _mm256_maskload_epi64(range_data + const2, mask64_1);

                __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                __m128i range_block1 = _mm256_castsi256_si128(range_block);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                __m256i range_temp1 = _mm256_cvtepu8_epi32(range_block1);
                INCREMENT_FLOP_COUNT(0, 0, 8*2, 0)
                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);

                __m256 fdomain1 = _mm256_cvtepi32_ps(domain1);

                __m256 frange1 = _mm256_cvtepi32_ps(range1);

                INCREMENT_FLOP_COUNT(0, 0, 8, 8)
                __m256 diff1 = _mm256_fmsub_ps(scaling_factor, fdomain1, frange1);

                INCREMENT_FLOP_COUNT(0, 0, 8, 8)
                top1 = _mm256_fmadd_ps(diff1, diff1, top1);
            }
            INCREMENT_FLOP_COUNT(0, 2, 0, 0)
            size = size - 8;
            curr+=8;
        }
    }


    INCREMENT_FLOP_COUNT(0, 0, 0, 3*8)
    __m256 temp_top1 = _mm256_add_ps(top1, top2);
    __m256 temp_top2 = _mm256_add_ps(top3, top4);
    __m256 temp_top12 = _mm256_add_ps(temp_top1, temp_top2);

    INCREMENT_FLOP_COUNT(0, 0, 0, 8)
    __m256 temp = _mm256_hadd_ps(temp_top12, zeros);
    INCREMENT_FLOP_COUNT(0, 0, 0, 9)
    __m256 temp1 = _mm256_hadd_ps(temp, zeros);

    float top_temp[8];
    _mm256_store_ps(top_temp, temp1);
    INCREMENT_FLOP_COUNT(0, 0, 0, 3)
    top = (double) top_temp[0] + (double) top_temp[4];

    for (int y = 0; y < size_copy; y++)
    {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (int x = 0; x < size; x++)
        {
            INCREMENT_FLOP_COUNT(2, 12, 0, 0)
            int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x) + curr] - domain_avg);
            int range = (range_data[(range_y + y) * range_width + (range_x + x) + curr] - range_avg);
            int diff = (scale * domain) - range;

            // According to the formula we want (DIFF*DIFF)/(SIZE*SIZE)
            INCREMENT_FLOP_COUNT(1, 0, 0, 1)
            top += (diff * diff);

            if (top < 0)
            {
                //printf("Error: Overflow occured during error %lf\n", top);
                return 0.0;
            }
        }
    }
#ifdef COUNT_DETAIL_CYCLES
    int64_t cycles = cycles_count_stop();
    INCREMENT_CYCLE_COUNT(0,cycles,0,0,0)
#endif
    return top / bottom;
}

static inline
double get_scale_factor(
        pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
        pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg,
        int size)
{
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    __m256i mask64_1 = _mm256_set_epi64x(0, 0, 0, 0xFFFFFFFFFFFFFFFF);
    __m256i mask64_2 = _mm256_set_epi64x(0, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    __m256i mask1 = _mm256_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    __m256i top1 = _mm256_set1_epi32(0);
    __m256i top2 = _mm256_set1_epi32(0);
    __m256i top3 = _mm256_set1_epi32(0);
    __m256i top4 = _mm256_set1_epi32(0);
    __m256i bottom1 = _mm256_set1_epi32(0);
    __m256i bottom2 = _mm256_set1_epi32(0);
    __m256i bottom3 = _mm256_set1_epi32(0);
    __m256i bottom4 = _mm256_set1_epi32(0);
    __m256i zeros = _mm256_set1_epi32(0);

    __m256i dom_avg = _mm256_set1_epi32(domain_avg);
    __m256i ran_avg = _mm256_set1_epi32(range_avg);

    int top = 0;
    int bottom = 0;
    uint32_t size_copy = size;

    uint32_t curr = 0;
    while(size >= 8) {
        if(size >= 32) {
            for(int y=0; y<size_copy; y++) {
                INCREMENT_FLOP_COUNT(3, 5, 0, 0)
                uint32_t addr_domain = (domain_y+y)*domain_width + domain_x + curr;
                uint32_t addr_range = (range_y+y)*range_width + range_x + curr;

                //printf("HH %d %d\n", const1, const2);
                INCREMENT_FLOP_COUNT(0, 2, 0, 0)
                __m256i domain_block = _mm256_loadu_si256(domain_data + addr_domain);
                __m256i range_block  = _mm256_loadu_si256(range_data + addr_range);

                __m128i domain_block1 = _mm256_extracti128_si256(domain_block, 0);
                __m128i domain_block2 = _mm256_extracti128_si256(domain_block, 1);

                __m128i range_block1 = _mm256_extracti128_si256(range_block, 0);
                __m128i range_block2 = _mm256_extracti128_si256(range_block, 1);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);

                __m128i domain_block1_hi = _mm_unpackhi_epi64(domain_block1, _mm_setzero_si128());
                __m256i domain_temp2 = _mm256_cvtepu8_epi32(domain_block1_hi);
                __m256i domain_temp3 = _mm256_cvtepu8_epi32(domain_block2);
                __m128i domain_block2_hi = _mm_unpackhi_epi64(domain_block2, _mm_setzero_si128());
                __m256i domain_temp4 = _mm256_cvtepu8_epi32(domain_block2_hi);


                __m256i range_temp1 = _mm256_cvtepu8_epi32(range_block1);
                __m128i range_block1_hi = _mm_unpackhi_epi64(range_block1, _mm_setzero_si128());
                __m256i range_temp2 = _mm256_cvtepu8_epi32(range_block1_hi);
                __m256i range_temp3 = _mm256_cvtepu8_epi32(range_block2);
                __m128i range_block2_hi = _mm_unpackhi_epi64(range_block2, _mm_setzero_si128());
                __m256i range_temp4 = _mm256_cvtepu8_epi32(range_block2_hi);


                INCREMENT_FLOP_COUNT(0, 4*8, 0, 0)
                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i domain2 = _mm256_sub_epi32(domain_temp2, dom_avg);
                __m256i domain3 = _mm256_sub_epi32(domain_temp3, dom_avg);
                __m256i domain4 = _mm256_sub_epi32(domain_temp4, dom_avg);

                INCREMENT_FLOP_COUNT(0, 4*8, 0, 0)
                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);
                __m256i range2 = _mm256_sub_epi32(range_temp2, ran_avg);
                __m256i range3 = _mm256_sub_epi32(range_temp3, ran_avg);
                __m256i range4 = _mm256_sub_epi32(range_temp4, ran_avg);

                // Need some testing now
                INCREMENT_FLOP_COUNT(4*8, 0, 0, 0)
                __m256i numerator1 = _mm256_mullo_epi32(domain1, range1);
                __m256i numerator2 = _mm256_mullo_epi32(domain2, range2);
                __m256i numerator3 = _mm256_mullo_epi32(domain3, range3);
                __m256i numerator4 = _mm256_mullo_epi32(domain4, range4);


                INCREMENT_FLOP_COUNT(4*8, 0, 0, 0)
                __m256i denominator1 = _mm256_mullo_epi32(domain1, domain1);
                __m256i denominator2 = _mm256_mullo_epi32(domain2, domain2);
                __m256i denominator3 = _mm256_mullo_epi32(domain3, domain3);
                __m256i denominator4 = _mm256_mullo_epi32(domain4, domain4);


                INCREMENT_FLOP_COUNT(0, 4*8, 0, 0)
                top1 = _mm256_add_epi32(top1, numerator1);
                top2 = _mm256_add_epi32(top2, numerator2);
                top3 = _mm256_add_epi32(top3, numerator3);
                top4 = _mm256_add_epi32(top4, numerator4);

                INCREMENT_FLOP_COUNT(0, 4*8, 0, 0)
                bottom1 = _mm256_add_epi32(bottom1, denominator1);
                bottom2 = _mm256_add_epi32(bottom2, denominator2);
                bottom3 = _mm256_add_epi32(bottom3, denominator3);
                bottom4 = _mm256_add_epi32(bottom4, denominator4);
            }
            INCREMENT_FLOP_COUNT(0, 2, 0, 0)
            curr += 32;
            size -= 32;
        }else if(size>=16) {
            for(int y=0; y<size_copy; y++) {
                INCREMENT_FLOP_COUNT(2, 9, 0, 0)
                uint32_t const1 = (domain_y+y)*domain_width + domain_x + curr;
                uint32_t const2 = (range_y+y)*range_width + range_x + curr;

                __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_2);
                __m256i range_block  = _mm256_maskload_epi64(range_data + const2, mask64_2);

                __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                __m128i range_block1 = _mm256_castsi256_si128(range_block);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                __m128i domain_block1_hi = _mm_unpackhi_epi64(domain_block1, _mm_setzero_si128());
                __m256i domain_temp2 = _mm256_cvtepu8_epi32(domain_block1_hi);

                __m256i range_temp1 = _mm256_cvtepu8_epi32(range_block1);
                __m128i range_block1_hi = _mm_unpackhi_epi64(range_block1, _mm_setzero_si128());
                __m256i range_temp2 = _mm256_cvtepu8_epi32(range_block1_hi);



                INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i domain2 = _mm256_sub_epi32(domain_temp2, dom_avg);

                INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);
                __m256i range2 = _mm256_sub_epi32(range_temp2, ran_avg);

                INCREMENT_FLOP_COUNT(2*8, 0, 0, 0)
                __m256i numerator1 = _mm256_mullo_epi32(domain1, range1);
                __m256i numerator2 = _mm256_mullo_epi32(domain2, range2);

                INCREMENT_FLOP_COUNT(2*8, 0, 0, 0)
                __m256i denominator1 = _mm256_mullo_epi32(domain1, domain1);
                __m256i denominator2 = _mm256_mullo_epi32(domain2, domain2);


                INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
                top1 = _mm256_add_epi32(top1, numerator1);
                top2 = _mm256_add_epi32(top2, numerator2);

                INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
                bottom1 = _mm256_add_epi32(bottom1, denominator1);
                bottom2 = _mm256_add_epi32(bottom2, denominator2);
            }
            INCREMENT_FLOP_COUNT(0, 2, 0, 0)
            size -= 16;
            curr += 16;
        }else if(size >= 8) {
            for(int y=0; y<size; y++) {
                INCREMENT_FLOP_COUNT(2, 9, 0, 0)
                uint32_t const1 = (domain_y+y)*domain_width + domain_x + curr;
                uint32_t const2 = (range_y+y)*range_width + range_x + curr;
                __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_1);
                __m256i range_block  = _mm256_maskload_epi64(range_data + const2, mask64_1);

                __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                __m128i range_block1 = _mm256_castsi256_si128(range_block);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                __m256i range_temp1 = _mm256_cvtepu8_epi32(range_block1);

                INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);

                INCREMENT_FLOP_COUNT(2*8, 0, 0, 0)
                __m256i numerator1 = _mm256_mullo_epi32(domain1, range1);
                __m256i denominator1 = _mm256_mullo_epi32(domain1, domain1);

                INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
                top1 = _mm256_add_epi32(top1, numerator1);
                bottom1 = _mm256_add_epi32(bottom1, denominator1);
            }
            INCREMENT_FLOP_COUNT(0, 2, 0, 0)
            size -= 8;
            curr += 8;
        }
    }

    INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
    __m256i temp_top1 = _mm256_add_epi32(top1, top2);
    __m256i temp_top3 = _mm256_add_epi32(top3, top4);

    INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
    __m256i temp_bottom1 = _mm256_add_epi32(bottom1, bottom2);
    __m256i temp_bottom3 = _mm256_add_epi32(bottom3, bottom4);

    INCREMENT_FLOP_COUNT(0, 2*8, 0, 0)
    __m256i temp_top13 = _mm256_add_epi32(temp_top1, temp_top3);
    __m256i temp_bottom13 = _mm256_add_epi32(temp_bottom1, temp_bottom3);

    // Bunch of hadd to get the final value

    INCREMENT_FLOP_COUNT(0, 8, 0, 0)
    __m256i temp_t1 = _mm256_hadd_epi32(temp_top13, zeros);
    INCREMENT_FLOP_COUNT(0, 8, 0, 0)
    __m256i temp_b1 = _mm256_hadd_epi32(temp_bottom13, zeros);
    INCREMENT_FLOP_COUNT(0, 8, 0, 0)
    __m256i temp_t2 = _mm256_hadd_epi32(temp_t1, zeros);
    INCREMENT_FLOP_COUNT(0, 8, 0, 0)
    __m256i temp_b2 = _mm256_hadd_epi32(temp_b1, zeros);

    int array_top[8];
    int array_bottom[8];

    _mm256_maskstore_epi32(array_top, mask1, temp_t2);
    _mm256_maskstore_epi32(array_bottom, mask1, temp_b2);


    INCREMENT_FLOP_COUNT(0, 5, 0, 0)
    top = array_top[0] + array_top[4];
    bottom = array_bottom[0] + array_bottom[4];

    for (int y = 0; y < size_copy; y++)
    {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (int x = 0; x < size; x++)
        {
            INCREMENT_FLOP_COUNT(2, 11, 0, 0)
            int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x) + curr] - domain_avg);
            int range = (range_data[(range_y + y) * range_width + (range_x + x) + curr] - range_avg);

            // According to the formula we want (R*D) / (D*D)
            INCREMENT_FLOP_COUNT(2, 2, 0, 0)
            top += range * domain;
            bottom += domain * domain;

            if (bottom < 0)
            {
                printf("Error: Overflow occured during scaling %d %d %d %d\n", y, domain_width, bottom, top);
                return 0.0;
            }
        }
    }

    if (bottom == 0)
    {
        top = 0;
        bottom = 1;
    }
#ifdef COUNT_DETAIL_CYCLES
    int64_t cycles = cycles_count_stop();
    INCREMENT_CYCLE_COUNT(0,0,cycles,0,0)
#endif
    return ((double)top) / ((double)bottom);
}

#endif // ENCODERUTILS_H
