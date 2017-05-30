#ifndef ENCODERUTILS_H
#define ENCODERUTILS_H

#include <common.h>

static
inline u_int32_t get_average_pixel(const pixel_value* domain_data, u_int32_t domain_width,
    u_int32_t domain_x, u_int32_t domain_y, u_int32_t size)
{
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    INCREMENT_FLOP_COUNT(1, 1, 0, 0)
    u_int32_t top1 = 0;
    u_int32_t top2 = 0;
    u_int32_t bottom = (size * size);

    // Simple average of all pixels.
    for (size_t y = domain_y; y < domain_y + size; y++)
    {
        size_t x;
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (x = domain_x; x < domain_x + size - 2; x+=2)
        {
            INCREMENT_FLOP_COUNT(1, 4, 0, 0)
            int index = y * domain_width + x;
            top1 += domain_data[index];
            top2 += domain_data[index + 1];
        }

        for (; x < domain_x + size; x++)
        {
            INCREMENT_FLOP_COUNT(1, 3, 0, 0)
            top1 += domain_data[y * domain_width + x];

        }
    }
    top1 += top2;
    if (top1 < 0)
    {
        printf("Error: Accumulator rolled over averaging pixels.\n");
        return ERR_ACCUM_ROLL;
    }
#ifdef COUNT_DETAIL_CYCLES
    int64_t cycles = cycles_count_stop();
    INCREMENT_CYCLE_COUNT(cycles,0,0,0,0)
#endif
    return top1 / bottom;
}

static inline
void get_average_pixel_bulk(const pixel_value* domain_data, uint32_t domain_width,
    uint32_t domain_x, uint32_t domain_y, uint32_t size, uint8_t* average_pixel) {
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    int blocksize = size/2;
    uint32_t top = 0;
    uint32_t bottom = size*size;
    uint32_t mid_bottom = blocksize * blocksize;
    uint8_t index = 0;
    for(size_t i = 0; i < size; i += blocksize) {
        for(size_t j = 0; j < size; j += blocksize) {
            uint32_t temp_top = 0;
            for(size_t ii=i; ii < i+blocksize; ii++) {
                for(size_t jj=j; jj<j+blocksize; jj++) {
                    temp_top += domain_data[(domain_y + ii)*domain_width + domain_x + jj];
                }
            }
            *(average_pixel+index) = temp_top/mid_bottom;
            top += temp_top;
            index++;
        }
    }
    *(average_pixel+index) = top/bottom;
#ifdef COUNT_DETAIL_CYCLES
    int64_t cycles = cycles_count_stop();
    INCREMENT_CYCLE_COUNT(cycles,0,0,0,0)
#endif
}

static
inline double get_error(
    pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
    pixel_value* range_data, int range_width, int range_x, int range_y, int range_avg,
    int size, double scale)
{
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    INCREMENT_FLOP_COUNT(1, 0, 0, 0)
    double top = 0;
    double bottom = (double)(size * size);

    for (int y = 0; y < size; y++)
    {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (int x = 0; x < size; x++)
        {
            INCREMENT_FLOP_COUNT(4, 11, 1, 0)
            int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x)] - domain_avg);
            int range = (range_data[(range_y + y) * range_width + (range_x + x)] - range_avg);
            int diff = (int)(scale * (double)domain) - range;

            // According to the formula we want (DIFF*DIFF)/(SIZE*SIZE)
            top += (diff * diff);

            if (top < 0)
            {
                printf("Error: Overflow occured during error %lf\n", top);
                return ERR_OVERFLOW;
            }
        }
    }
#ifdef COUNT_DETAIL_CYCLES
    int64_t cycles = cycles_count_stop();
    INCREMENT_CYCLE_COUNT(0,cycles,0,0,0)
#endif
    return top / bottom;
}

static
inline double get_scale_factor(
    pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
    pixel_value* range_Data, int range_width, int range_x, int range_y, int range_avg,
    int size)
{
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    int top = 0;
    int bottom = 0;

    for (int y = 0; y < size; y++)
    {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (int x = 0; x < size; x++)
        {
            INCREMENT_FLOP_COUNT(4, 10, 0, 0)
            int domain = (domain_data[(domain_y + y) * domain_width + (domain_x + x)] - domain_avg);
            int range = (range_Data[(range_y + y) * range_width + (range_x + x)] - range_avg);

            // According to the formula we want (R*D) / (D*D)
            top += range * domain;
            bottom += domain * domain;

            if (bottom < 0)
            {
                printf("Error: Overflow occured during scaling %d %d %d %d\n", y, domain_width, bottom, top);
                return ERR_OVERFLOW;
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
