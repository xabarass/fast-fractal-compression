#ifndef FRAC_ERRORS_H
#define FRAC_ERRORS_H

typedef enum _ERROR_CODES{
    ERR_SUCCESS=0,
    ERR_INVALID_ARGUMENT,
    ERR_NOT_IMPLEMENTED,
    ERR_CHANNEL_NOT_AVAILABLE,
    ERR_ACCUM_ROLL, //Accumulator rolled over averaging pixels
    ERR_OVERFLOW,
    ERR_IMAGE_WRONG_SIZE,   //Image must have dimensions that are multiples of 32
    ERR_NO_IMAGE_PATH
} ERR_RET;

#endif // FRAC_ERRORS_H
