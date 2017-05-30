#include "ifs_transform.h"

ERR_RET down_sample(pixel_value *src, int src_width, int start_x, int start_y, int target_size, pixel_value* sample) {
#ifdef COUNT_DETAIL_CYCLES
    cycles_count_start ();
#endif
    int dest_x = 0;
    int dest_y = 0;
    INCREMENT_FLOP_COUNT(2, 2, 0, 0)
    int index_x = start_x + target_size * 2;
    int index_y = start_y + target_size * 2;
    for (int y = start_y; y < index_y; y += 2) {
        INCREMENT_FLOP_COUNT(0, 2, 0, 0)
        for (int x = start_x; x < index_x; x += 2) {
            INCREMENT_FLOP_COUNT(3, 11, 0, 0)

            uint32_t pixel = 0;
            int index_x = y * src_width + x;
            int index_y = (y + 1) * src_width + x;
            pixel += src[index_x];
            pixel += src[index_x + 1];
            pixel += src[index_y];
            pixel += src[index_y + 1];
            pixel = pixel >> 2;
            sample[dest_y * target_size + dest_x] = pixel;
            dest_x++;
        }
        dest_y++;
        dest_x = 0;
    }
#ifdef COUNT_DETAIL_CYCLES
    int64_t cycles = cycles_count_stop();
    INCREMENT_CYCLE_COUNT(0,0,0,cycles,0)
#endif
    return ERR_SUCCESS;
}
