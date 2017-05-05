#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

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

#include "frac_errors.h"



#define LOGT(...) printf("T [" MODULE_NAME"]: " __VA_ARGS__)
#define LOGD(...) printf("D [" MODULE_NAME"]: " __VA_ARGS__)
#define LOGW(...) fprintf(stderr, "W [" MODULE_NAME"]: " __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "E [" MODULE_NAME"]: " __VA_ARGS__)

#endif // COMMON_H
