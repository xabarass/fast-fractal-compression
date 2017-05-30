#ifndef ENCODERTRANSFORMATIONS_H
#define ENCODERTRANSFORMATIONS_H

#include <common.h>

static
inline
ERR_RET ifs_transformation_execute_downsampled(int from_x, int from_y, enum ifs_type symmetry,
                                    u_int32_t size, pixel_value* src, u_int32_t src_width,
                                    pixel_value* dest, u_int32_t dest_width){

#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    INCREMENT_FLOP_COUNT(2, 0, 0, 0)

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
        INCREMENT_FLOP_COUNT(0, 2, 0, 0)
        from_x += size - 1;
        d_x = -1;
    }

    if (!(  symmetry == SYM_NONE ||
            symmetry == SYM_R270 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_RDFLIP))
    {
        INCREMENT_FLOP_COUNT(0, 2, 0, 0)
        from_y += size - 1;
        d_y = -1;
    }

    int start_x = from_x;
    int start_y = from_y;


    for (int to_y = 0; to_y < size; to_y++)
    {
        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
        for (int to_x = 0; to_x < size; to_x++)
        {
            INCREMENT_FLOP_COUNT(1, 5, 0, 0)
            int pixel = src[from_y * src_width + from_x];
            dest[to_y * dest_width + to_x] = pixel;

            if (in_order)
                from_x += d_x;
            else
                from_y += d_y;
        }

        INCREMENT_FLOP_COUNT(0, 1, 0, 0)
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
#ifdef COUNT_DETAIL_CYCLES
    int64_t cycles = cycles_count_stop();
    INCREMENT_CYCLE_COUNT(0,0,0,0,cycles)
#endif
    return ERR_SUCCESS;
}

#endif // ENCODERTRANSFORMATIONS_H
