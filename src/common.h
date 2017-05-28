#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "immintrin.h"
#include <include/perf.h>

typedef uint8_t u_int8_t;
typedef uint32_t u_int32_t;

#ifdef COUNT_FLOPS
    struct global_op_count{
        size_t int_mults;
        size_t int_adds;
        size_t fp_mults;
        size_t fp_adds;
    };

    extern struct global_op_count __GLOBAL_OP_COUNT;

    #define INCREMENT_FLOP_COUNT(_int_mul, _int_add, _fp_mul, _fp_add) __GLOBAL_OP_COUNT.int_mults+=(_int_mul);\
        __GLOBAL_OP_COUNT.int_adds+=(_int_add);\
        __GLOBAL_OP_COUNT.fp_adds+=(_fp_add);\
        __GLOBAL_OP_COUNT.fp_mults+=(_fp_mul);\

#else
    #define INCREMENT_FLOP_COUNT(_int_mul, _int_add, fp_mul, fp_add)
#endif


#ifdef COUNT_DETAIL_CYCLES
    struct global_function_cycle_count{
        size_t get_average_pixel_cycles;
        size_t get_error_cycles;
        size_t get_scale_factor_cycles;
        size_t down_sample_cycles;
        size_t ifs_transformation_execute_cycles;
    };

    extern struct global_function_cycle_count __COUNT_DETAIL_CYCLES;


    #define INCREMENT_CYCLE_COUNT(_get_average_pixel_cycles, _get_error_cycles, _get_scale_factor_cycles, _down_sample_cycles, _ifs_transformation_execute_cycles) __COUNT_DETAIL_CYCLES.get_average_pixel_cycles+=(_get_average_pixel_cycles);\
            __COUNT_DETAIL_CYCLES.get_error_cycles+=(_get_error_cycles);\
            __COUNT_DETAIL_CYCLES.get_scale_factor_cycles+=(_get_scale_factor_cycles);\
            __COUNT_DETAIL_CYCLES.down_sample_cycles+=(_down_sample_cycles);\
            __COUNT_DETAIL_CYCLES.ifs_transformation_execute_cycles+=(_ifs_transformation_execute_cycles);\

#else
    #define INCREMENT_CYCLE_COUNT(_get_average_pixel_cycles, _get_error_cycles, _get_scale_factor_cycles, _down_sample_cycles, _ifs_transformation_execute_cycles)
#endif

#include "frac_errors.h"

#define LOGT(...) printf("T [" MODULE_NAME"]: " __VA_ARGS__)
#define LOGD(...) printf("D [" MODULE_NAME"]: " __VA_ARGS__)
#define LOGW(...) fprintf(stderr, "W [" MODULE_NAME"]: " __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "E [" MODULE_NAME"]: " __VA_ARGS__)

#endif // COMMON_H
