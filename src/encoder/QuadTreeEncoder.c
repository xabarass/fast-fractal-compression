#include "QuadTreeEncoder.h"

#define MODULE_NAME "QuadTreeEncoder"

static pixel_value* buffer;
static size_t MAX_BUFFER_SIZE;
static size_t MIN_BUFFER_SIZE;

static
inline
ERR_RET ifs_transformation_execute_downsampled(int from_x, int from_y, enum ifs_type symmetry,
                                    u_int32_t size, pixel_value* src, u_int32_t src_width,
                                    pixel_value* dest, u_int32_t dest_width){

    INCREMENT_FLOP_COUNT(6, 0, 0, 0)

    from_x = from_x / 2;
    from_y = from_y / 2;
    int d_x = 1;
    int d_y = 1;
    bool in_order = (
            symmetry == SYM_NONE ||
            symmetry == SYM_R180 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_VFLIP
            );

    if (!(  symmetry == SYM_NONE ||
            symmetry == SYM_R90 ||
            symmetry == SYM_VFLIP ||
            symmetry == SYM_RDFLIP))
    {
        from_x += size - 1;
        d_x = -1;
    }

    if (!(  symmetry == SYM_NONE ||
            symmetry == SYM_R270 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_RDFLIP))
    {
        from_y += size - 1;
        d_y = -1;
    }

    int start_x = from_x;
    int start_y = from_y;

    INCREMENT_FLOP_COUNT(2*size*size,
                         4*size*size, size*size, 0)
    INCREMENT_FLOP_COUNT(size,0,0,0)

    for (int to_y = 0; to_y < size; to_y++)
    {
        for (int to_x = 0; to_x < size; to_x++)
        {
            int pixel = src[from_y * src_width + from_x];
            dest[to_y * dest_width + to_x] = pixel;

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

    return ERR_SUCCESS;
}

static inline
u_int32_t get_average_pixel(const pixel_value* domain_data, u_int32_t domain_width,
    u_int32_t domain_x, u_int32_t domain_y, u_int32_t size)
{
    // Simple average of all pixels.
    INCREMENT_FLOP_COUNT(size*size, size*size*2,0,0)

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
            size = size - 32;
            curr+=32;
        }else if(size >= 16) {
            for(int y=0; y<size_copy; y++) {
                uint32_t const1 = (domain_y+y)*domain_width + domain_x +curr;
                __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_2);

                __m128i domain_block1 = _mm256_castsi256_si128(domain_block);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                __m128i domain_block1_hi = _mm_unpackhi_epi64(domain_block1, _mm_setzero_si128());
                __m256i domain_temp2 = _mm256_cvtepu8_epi32(domain_block1_hi);

                top1 = _mm256_add_epi32(top1, domain_temp1);
                top2 = _mm256_add_epi32(top2, domain_temp2);
            }
            size = size - 16;
            curr+=16;
        }else if(size >= 8) {
            for(int y=0; y<size_copy; y++) {
                uint32_t const1 = (domain_y+y)*domain_width + domain_x+curr;
                __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_1);

                __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);

                top1 = _mm256_add_epi32(top1, domain_temp1);
            }
            size = size - 8;
            curr+=8;
        }
    }

    __m256i temp_top1 = _mm256_add_epi32(top1, top2);
    __m256i temp_top3 = _mm256_add_epi32(top3, top4);

    __m256i temp_top13 = _mm256_add_epi32(temp_top1, temp_top3);

    // Bunch of hadd to get the final value

    __m256i temp_t1 = _mm256_hadd_epi32(temp_top13, zeros);
    __m256i temp_t2 = _mm256_hadd_epi32(temp_t1, zeros);

    uint32_t array_top[8];

    __m256i mask1 = _mm256_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

    _mm256_maskstore_epi32(array_top, mask1, temp_t2);

    top = array_top[0] + array_top[4];

    for (size_t y = 0; y < size_copy; y++)
    {
        for (size_t x = 0; x < size; x++)
        {
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
double get_error(
    pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
    pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg,
    int size, double scale)
{
    INCREMENT_FLOP_COUNT(size*size*3, size*size*10,size*size,0)

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
    double bottom = (double)(size * size);

    uint32_t curr = 0;
    __m256i mask64_1 = _mm256_set_epi64x(0, 0, 0, 0xFFFFFFFFFFFFFFFF);
    __m256i mask64_2 = _mm256_set_epi64x(0, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

    while(size >= 8) {
        if(size >= 32) {
            for(int y=0; y<size_copy; y++) {
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

                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i domain2 = _mm256_sub_epi32(domain_temp2, dom_avg);
                __m256i domain3 = _mm256_sub_epi32(domain_temp3, dom_avg);
                __m256i domain4 = _mm256_sub_epi32(domain_temp4, dom_avg);

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

                __m256 diff1 = _mm256_fmsub_ps(scaling_factor, fdomain1, frange1);
                __m256 diff2 = _mm256_fmsub_ps(scaling_factor, fdomain2, frange2);
                __m256 diff3 = _mm256_fmsub_ps(scaling_factor, fdomain3, frange3);
                __m256 diff4 = _mm256_fmsub_ps(scaling_factor, fdomain4, frange4);

                top1 = _mm256_fmadd_ps(diff1, diff1, top1);
                top2 = _mm256_fmadd_ps(diff2, diff2, top2);
                top3 = _mm256_fmadd_ps(diff3, diff3, top3);
                top4 = _mm256_fmadd_ps(diff4, diff4, top4);
            }
            size = size - 32;
            curr+=32;
        }else if(size >= 16) {
            for(int y=0; y<size; y++) {
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

                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i domain2 = _mm256_sub_epi32(domain_temp2, dom_avg);

                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);
                __m256i range2 = _mm256_sub_epi32(range_temp2, ran_avg);

                __m256 fdomain1 = _mm256_cvtepi32_ps(domain1);
                __m256 fdomain2 = _mm256_cvtepi32_ps(domain2);

                __m256 frange1 = _mm256_cvtepi32_ps(range1);
                __m256 frange2 = _mm256_cvtepi32_ps(range2);

                __m256 diff1 = _mm256_fmsub_ps(scaling_factor, fdomain1, frange1);
                __m256 diff2 = _mm256_fmsub_ps(scaling_factor, fdomain2, frange2);

                top1 = _mm256_fmadd_ps(diff1, diff1, top1);
                top2 = _mm256_fmadd_ps(diff2, diff2, top2);
            }
            size = size - 16;
            curr+=16;
        }else if(size >= 8) {
            for(int y=0; y<size; y++) {
                uint32_t const1 = (domain_y+y)*domain_width + domain_x + curr;
                uint32_t const2 = (range_y+y)*range_width + range_x + curr;
                __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_1);
                __m256i range_block  = _mm256_maskload_epi64(range_data + const2, mask64_1);

                __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                __m128i range_block1 = _mm256_castsi256_si128(range_block);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                __m256i range_temp1 = _mm256_cvtepu8_epi32(range_block1);

                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);

                __m256 fdomain1 = _mm256_cvtepi32_ps(domain1);

                __m256 frange1 = _mm256_cvtepi32_ps(range1);

                __m256 diff1 = _mm256_fmsub_ps(scaling_factor, fdomain1, frange1);

                top1 = _mm256_fmadd_ps(diff1, diff1, top1);
            }
            size = size - 8;
            curr+=8;
        }
    }


    __m256 temp_top1 = _mm256_add_ps(top1, top2);
    __m256 temp_top2 = _mm256_add_ps(top3, top4);
    __m256 temp_top12 = _mm256_add_ps(temp_top1, temp_top2);

    __m256 temp = _mm256_hadd_ps(temp_top12, zeros);
    __m256 temp1 = _mm256_hadd_ps(temp, zeros);

    float top_temp[8];
    _mm256_store_ps(top_temp, temp1);
    top = (double) top_temp[0] + (double) top_temp[4];

    for (int y = 0; y < size_copy; y++)
    {
        for (int x = 0; x < size; x++)
        {
            int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x) + curr] - domain_avg);
            int range = (range_data[(range_y + y) * range_width + (range_x + x) + curr] - range_avg);
            int diff = (scale * domain) - range;

            // According to the formula we want (DIFF*DIFF)/(SIZE*SIZE)
            top += (diff * diff);

            if (top < 0)
            {
                //printf("Error: Overflow occured during error %lf\n", top);
                return .0;
            }
        }
    }
    return top / bottom;
}

static inline
double get_scale_factor(
        pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
        pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg,
        int size)
{
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
                uint32_t addr_domain = (domain_y+y)*domain_width + domain_x + curr;
                uint32_t addr_range = (range_y+y)*range_width + range_x + curr;

                //printf("HH %d %d\n", const1, const2);
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

                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i domain2 = _mm256_sub_epi32(domain_temp2, dom_avg);
                __m256i domain3 = _mm256_sub_epi32(domain_temp3, dom_avg);
                __m256i domain4 = _mm256_sub_epi32(domain_temp4, dom_avg);

                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);
                __m256i range2 = _mm256_sub_epi32(range_temp2, ran_avg);
                __m256i range3 = _mm256_sub_epi32(range_temp3, ran_avg);
                __m256i range4 = _mm256_sub_epi32(range_temp4, ran_avg);

                // Need some testing now
                __m256i numerator1 = _mm256_mullo_epi32(domain1, range1);
                __m256i numerator2 = _mm256_mullo_epi32(domain2, range2);
                __m256i numerator3 = _mm256_mullo_epi32(domain3, range3);
                __m256i numerator4 = _mm256_mullo_epi32(domain4, range4);

                __m256i denominator1 = _mm256_mullo_epi32(domain1, domain1);
                __m256i denominator2 = _mm256_mullo_epi32(domain2, domain2);
                __m256i denominator3 = _mm256_mullo_epi32(domain3, domain3);
                __m256i denominator4 = _mm256_mullo_epi32(domain4, domain4);

                top1 = _mm256_add_epi32(top1, numerator1);
                top2 = _mm256_add_epi32(top2, numerator2);
                top3 = _mm256_add_epi32(top3, numerator3);
                top4 = _mm256_add_epi32(top4, numerator4);

                bottom1 = _mm256_add_epi32(bottom1, denominator1);
                bottom2 = _mm256_add_epi32(bottom2, denominator2);
                bottom3 = _mm256_add_epi32(bottom3, denominator3);
                bottom4 = _mm256_add_epi32(bottom4, denominator4);
            }
            curr += 32;
            size -= 32;
        }else if(size>=16) {
            for(int y=0; y<size_copy; y++) {
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


                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i domain2 = _mm256_sub_epi32(domain_temp2, dom_avg);

                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);
                __m256i range2 = _mm256_sub_epi32(range_temp2, ran_avg);

                __m256i numerator1 = _mm256_mullo_epi32(domain1, range1);
                __m256i numerator2 = _mm256_mullo_epi32(domain2, range2);

                __m256i denominator1 = _mm256_mullo_epi32(domain1, domain1);
                __m256i denominator2 = _mm256_mullo_epi32(domain2, domain2);

                top1 = _mm256_add_epi32(top1, numerator1);
                top2 = _mm256_add_epi32(top2, numerator2);

                bottom1 = _mm256_add_epi32(bottom1, denominator1);
                bottom2 = _mm256_add_epi32(bottom2, denominator2);
            }
            size -= 16;
            curr += 16;
        }else if(size >= 8) {
            for(int y=0; y<size; y++) {
                uint32_t const1 = (domain_y+y)*domain_width + domain_x + curr;
                uint32_t const2 = (range_y+y)*range_width + range_x + curr;
                __m256i domain_block = _mm256_maskload_epi64(domain_data + const1, mask64_1);
                __m256i range_block  = _mm256_maskload_epi64(range_data + const2, mask64_1);

                __m128i domain_block1 = _mm256_castsi256_si128(domain_block);
                __m128i range_block1 = _mm256_castsi256_si128(range_block);

                __m256i domain_temp1 = _mm256_cvtepu8_epi32(domain_block1);
                __m256i range_temp1 = _mm256_cvtepu8_epi32(range_block1);

                __m256i domain1 = _mm256_sub_epi32(domain_temp1, dom_avg);
                __m256i range1 = _mm256_sub_epi32(range_temp1, ran_avg);

                __m256i numerator1 = _mm256_mullo_epi32(domain1, range1);
                __m256i denominator1 = _mm256_mullo_epi32(domain1, domain1);

                top1 = _mm256_add_epi32(top1, numerator1);
                bottom1 = _mm256_add_epi32(bottom1, denominator1);
            }
            size -= 8;
            curr += 8;
        }
    }

    __m256i temp_top1 = _mm256_add_epi32(top1, top2);
    __m256i temp_top3 = _mm256_add_epi32(top3, top4);

    __m256i temp_bottom1 = _mm256_add_epi32(bottom1, bottom2);
    __m256i temp_bottom3 = _mm256_add_epi32(bottom3, bottom4);

    __m256i temp_top13 = _mm256_add_epi32(temp_top1, temp_top3);
    __m256i temp_bottom13 = _mm256_add_epi32(temp_bottom1, temp_bottom3);

    // Bunch of hadd to get the final value

    __m256i temp_t1 = _mm256_hadd_epi32(temp_top13, zeros);
    __m256i temp_b1 = _mm256_hadd_epi32(temp_bottom13, zeros);
    __m256i temp_t2 = _mm256_hadd_epi32(temp_t1, zeros);
    __m256i temp_b2 = _mm256_hadd_epi32(temp_b1, zeros);

    int array_top[8];
    int array_bottom[8];

    _mm256_maskstore_epi32(array_top, mask1, temp_t2);
    _mm256_maskstore_epi32(array_bottom, mask1, temp_b2);


    top = array_top[0] + array_top[4];
    bottom = array_bottom[0] + array_bottom[4];

    for (int y = 0; y < size_copy; y++)
    {
        for (int x = 0; x < size; x++)
        {
            int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x) + curr] - domain_avg);
            int range = (range_data[(range_y + y) * range_width + (range_x + x) + curr] - range_avg);

            // According to the formula we want (R*D) / (D*D)
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

    return ((double)top) / ((double)bottom);
}

#define ASSIGN_IFS_VALUES(DST, FROM_X, FROM_Y, TO_X, TO_Y, TR_TYPE, SCALE, OFFSET, SIZE)\
    DST.from_x=FROM_X;\
    DST.from_y=FROM_Y;\
    DST.to_x=TO_X;\
    DST.to_y=TO_Y;\
    DST.transformation_type=TR_TYPE;\
    DST.scale=SCALE;\
    DST.offset=OFFSET;\
    DST.size=SIZE;

#define CALCULATE_ERR(SC, DM_X, DM_Y, DM_AVG, RB_X, RB_Y, RB_AVG, OFFSET, ERROR)\
    SC=get_scale_factor(img->image_channels[0], img->width, DM_X, DM_Y, DM_AVG,\
            buffer, block_size, RB_X, RB_Y, RB_AVG, half_block_size);\
    OFFSET = (int)(RB_AVG - SC * (double)DM_AVG);\
\
    ERROR=get_error(buffer, block_size, RB_X, RB_Y, DM_AVG, img->image_channels[0],\
            img->width, DM_X, DM_Y, RB_AVG,\
            half_block_size, SC);


#define UPDATE_MIN_ERROR(DM_X, DM_Y)\
if(tmp_error1<error_1){\
    ASSIGN_IFS_VALUES(best_ifs_1, DM_X, DM_Y, rb_x0, rb_y0, transformation_type, scale_factor1, offset1, half_block_size);\
    error_1=tmp_error1;\
}\
if(tmp_error2<error_2){\
    ASSIGN_IFS_VALUES(best_ifs_2, DM_X, DM_Y, rb_x1, rb_y0, transformation_type, scale_factor2, offset2, half_block_size);\
    error_2=tmp_error2;\
}\
if(tmp_error3<error_3){\
    ASSIGN_IFS_VALUES(best_ifs_3, DM_X, DM_Y, rb_x0, rb_y1, transformation_type, scale_factor3, offset3, half_block_size);\
    error_3=tmp_error3;\
}\
if(tmp_error4<error_4){\
    ASSIGN_IFS_VALUES(best_ifs_4, DM_X, DM_Y, rb_x1, rb_y1, transformation_type, scale_factor4, offset4, half_block_size);\
    error_4=tmp_error4;\
}\

#define CALCULATE_MIN(X_1, Y_1, AVG_1, X_2, Y_2, AVG_2, X_3, Y_3, AVG_3, X_4, Y_4, AVG_4)\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_1, 0, 0, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_1, 0, 0, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_1, 0, 0, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_1, 0, 0, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_1,Y_1);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_2, half_block_size, 0, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_2, half_block_size, 0, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_2, half_block_size, 0, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_2, half_block_size, 0, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_2,Y_2);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_3, 0, half_block_size, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_3, 0, half_block_size, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_3, 0, half_block_size, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_3, 0, half_block_size, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_3,Y_3);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_4, half_block_size, half_block_size, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_4, half_block_size, half_block_size, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_4, half_block_size, half_block_size, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_4, half_block_size, half_block_size, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_4,Y_4);\


static inline
ERR_RET find_matches_for(struct image_data* img, struct ifs_transformation_list* transformations,
                         u_int32_t rb_x0, u_int32_t rb_y0, u_int32_t block_size, u_int32_t threshold){
    u_int32_t half_block_size=block_size/2;
    u_int32_t double_block_size=block_size*2;
    u_int32_t rb_x1=rb_x0+half_block_size;
    u_int32_t rb_y1=rb_y0+half_block_size;

    u_int32_t avarage_pix_0=get_average_pixel(img->image_channels[0], img->width, rb_x0, rb_y0, block_size);
    u_int32_t avarage_pix_1=get_average_pixel(img->image_channels[0], img->width, rb_x0, rb_y0, half_block_size);
    u_int32_t avarage_pix_2=get_average_pixel(img->image_channels[0], img->width, rb_x1, rb_y0, half_block_size);
    u_int32_t avarage_pix_3=get_average_pixel(img->image_channels[0], img->width, rb_x0, rb_y1, half_block_size);
    u_int32_t avarage_pix_4=get_average_pixel(img->image_channels[0], img->width, rb_x1, rb_y1, half_block_size);

    bool found_match0=false;

    double error_0=1e9;
    double error_1=1e9;
    double error_2=1e9;
    double error_3=1e9;
    double error_4=1e9;

    struct ifs_transformation best_ifs_0;
    struct ifs_transformation best_ifs_1;
    struct ifs_transformation best_ifs_2;
    struct ifs_transformation best_ifs_3;
    struct ifs_transformation best_ifs_4;

    double scale_factor1;
    double scale_factor2;
    double scale_factor3;
    double scale_factor4;

    int offset1;
    int offset2;
    int offset3;
    int offset4;

    double tmp_error1;
    double tmp_error2;
    double tmp_error3;
    double tmp_error4;

    int x_1;
    int y_1;

    pixel_value* img_channels=img->image_channels[0];
    pixel_value* img_channels_downsampled=img->image_channels[1];
    int half_width=img->width/2;

    for(size_t y=0; y<img->height; y+=double_block_size)
    {
        for (size_t x=0; x<img->width; x+=double_block_size)
        {
            int x_half=x/2;
            int y_half=y/2;

            u_int32_t domain_avg0=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half, block_size);

            u_int32_t domain_avg1=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half, half_block_size);
            u_int32_t domain_avg2=get_average_pixel(img_channels_downsampled, half_width, x_half+half_block_size, y_half, half_block_size);
            u_int32_t domain_avg3=get_average_pixel(img_channels_downsampled, half_width, x_half, y_half+half_block_size, half_block_size);
            u_int32_t domain_avg4=get_average_pixel(img_channels_downsampled, half_width, x_half+half_block_size, y_half+half_block_size, half_block_size);

            for(int transformation_type=0; transformation_type<SYM_MAX; ++transformation_type)
            {
                ifs_transformation_execute_downsampled(x, y, transformation_type,
                                                    block_size, img->image_channels[1], img->width/2,
                                                    buffer, block_size);

                double scale_factor=get_scale_factor(img->image_channels[0], img->width, rb_x0, rb_y0, domain_avg0,
                        buffer, block_size, 0 ,0, avarage_pix_0, block_size);
                int offset = (int)(avarage_pix_0 - scale_factor * (double)domain_avg0);

                double error=get_error(buffer, block_size, 0, 0, domain_avg0, img->image_channels[0],
                        img->width, rb_x0, rb_y0, avarage_pix_0,
                        block_size, scale_factor);

                if(error<error_0){
                    ASSIGN_IFS_VALUES(best_ifs_0, x, y, rb_x0, rb_y0, transformation_type, scale_factor, offset, block_size);

                    error_0=error;

                    if(error<threshold){
                        found_match0=true;
                    }
                }

                // Smaller blocks
                if(!found_match0){
                    x_1=x+block_size;
                    y_1=y+block_size;

                    switch (transformation_type){
                    case SYM_NONE:
                        CALCULATE_MIN(x,y, domain_avg1,
                                      x_1,y, domain_avg2,
                                      x,y_1, domain_avg3,
                                      x_1,y_1, domain_avg4);
                        break;
                    case SYM_R90:
                        CALCULATE_MIN(x,y_1, domain_avg3,
                                      x,y, domain_avg1,
                                      x_1,y_1, domain_avg4,
                                      x_1,y, domain_avg2);
                        break;
                    case SYM_R180:
                        CALCULATE_MIN(x_1,y_1, domain_avg4,
                                      x,y_1, domain_avg3,
                                      x_1,y, domain_avg2,
                                      x,y, domain_avg1);
                        break;
                    case SYM_R270:
                        CALCULATE_MIN(x_1,y, domain_avg2,
                                      x_1,y_1, domain_avg4,
                                      x,y, domain_avg1,
                                      x,y_1, domain_avg3);
                        break;
                    case SYM_HFLIP:
                        CALCULATE_MIN(x_1,y, domain_avg2,
                                      x,y, domain_avg1,
                                      x_1,y_1, domain_avg4,
                                      x,y_1, domain_avg3);
                        break;
                    case SYM_VFLIP:
                        CALCULATE_MIN(x,y_1, domain_avg3,
                                      x_1,y_1, domain_avg4,
                                      x,y, domain_avg1,
                                      x_1,y, domain_avg2);
                        break;
                    case SYM_FDFLIP:
                        CALCULATE_MIN(x_1,y_1, domain_avg4,
                                      x_1,y, domain_avg2,
                                      x,y_1, domain_avg3,
                                      x,y, domain_avg1);
                        break;
                    case SYM_RDFLIP:
                        CALCULATE_MIN(x,y, domain_avg1,
                                      x,y_1, domain_avg3,
                                      x_1,y, domain_avg2,
                                      x_1,y_1, domain_avg4);
                        break;
                    }
                }
            }
        }
    }

    if(found_match0){
        ifs_trans_push_back(transformations, &best_ifs_0);
    }else if(block_size>(2*MIN_BUFFER_SIZE)){
        u_int32_t quater_block_size=half_block_size/2;
        if(error_1<threshold){
            ifs_trans_push_back(transformations, &best_ifs_1);
        }else{
            find_matches_for(img, transformations, rb_x0, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0, rb_y0+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y0+quater_block_size, quater_block_size, threshold);
        }

        if(error_2<threshold){
            ifs_trans_push_back(transformations, &best_ifs_2);
        }else{
            find_matches_for(img, transformations, rb_x1, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y0, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1, rb_y0+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y0+quater_block_size, quater_block_size, threshold);
        }

        if(error_3<threshold){
            ifs_trans_push_back(transformations, &best_ifs_3);
        }else{
            find_matches_for(img, transformations, rb_x0, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0, rb_y1+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x0+quater_block_size, rb_y1+quater_block_size, quater_block_size, threshold);
        }

        if(error_4<threshold){
            ifs_trans_push_back(transformations, &best_ifs_4);
        }else{
            find_matches_for(img, transformations, rb_x1, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y1, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1, rb_y1+quater_block_size, quater_block_size, threshold);
            find_matches_for(img, transformations, rb_x1+quater_block_size, rb_y1+quater_block_size, quater_block_size, threshold);
        }
    }else if(block_size==MIN_BUFFER_SIZE){
        ifs_trans_push_back(transformations, &best_ifs_1);
        ifs_trans_push_back(transformations, &best_ifs_2);
        ifs_trans_push_back(transformations, &best_ifs_3);
        ifs_trans_push_back(transformations, &best_ifs_4);
    }else{
        assert(false && "Should not happen!");
    }

    return ERR_SUCCESS;
}

ERR_RET qtree_encode(struct Transforms* transformations, struct image_data* src, struct encoder_params params,
                     u_int32_t threshold_param){

    struct image_data img;
    u_int32_t width=src->width;
    u_int32_t height=src->height;
    u_int32_t threshold=threshold_param;    //Temporary fixed value!

    if (width % 32 != 0 || height % 32 != 0)
    {
        return ERR_IMAGE_WRONG_SIZE;
    }

    if(width!=height){
        printf("ERROR! W!=H\n");
        return ERR_IMAGE_WRONG_SIZE;
    }

    MAX_BUFFER_SIZE=width/32;
    if(MAX_BUFFER_SIZE<16)
           MAX_BUFFER_SIZE=16;
    MIN_BUFFER_SIZE=MAX_BUFFER_SIZE/4;
    buffer=malloc(MAX_BUFFER_SIZE*MAX_BUFFER_SIZE*sizeof(pixel_value));
    transformations->max_block_size=MAX_BUFFER_SIZE;
    printf("Starting to compress with block sizes MAX: %d MIN: %d\n", MAX_BUFFER_SIZE, MIN_BUFFER_SIZE);

    if(params.use_ycbcr){
        if(src->color_mode!=COLOR_MODE_YCbCr){
            convert_from_RGB_to_YCbCr(src);
        }
        transformations->color_mode=COLOR_MODE_YCbCr;
    }else{
        transformations->color_mode=COLOR_MODE_RGB;
    }

    //We do our crazy stuff here
    /*
     * Make sense because you are only working with two channels
     * [0] -> Original channel data
     * [1] -> Downsampled channel data
     */
    // We allocate only one channel other one is just a pointer to original
    init_image_data(&img, width, height, 1);
    img.image_channels[1]=img.image_channels[0];
    transformations->channels=src->channels;

    for (size_t channel=0; channel<src->channels; channel++){
//        printf("Channel: %d ==================================\n", channel);
        img.image_channels[0]=src->image_channels[channel];
        down_sample(img.image_channels[0], width, 0,0, width/2, img.image_channels[1]);
        transformations->ch[channel].head=NULL;
        transformations->ch[channel].tail=NULL;
        transformations->ch[channel].elements=0;

        if (channel >= 1 && params.use_ycbcr)
            threshold *= 2;

        for (size_t y = 0; y < img.height; y += MAX_BUFFER_SIZE)
        {
            for (size_t x = 0; x < img.width; x += MAX_BUFFER_SIZE)
            {
                find_matches_for(&img, transformations->ch+channel, x, y, MAX_BUFFER_SIZE, threshold);
                #ifdef DEBUG
                printf(".");
                #endif
            }
            #ifdef DEBUG
            printf("\n");
            #endif
        }

        if (channel >= 1 && params.use_ycbcr)
            threshold /= 2;
    }

    img.image_channels[0]=img.image_channels[1];
    clear_image_data(&img);

    return ERR_SUCCESS;
}
