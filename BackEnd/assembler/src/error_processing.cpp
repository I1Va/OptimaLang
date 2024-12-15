#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "error_processing.h"

const char *get_bit_descr(unsigned long long err) {
    #define BIT_DESCR_(code) case code : return #code;
    switch (err) {
        BIT_DESCR_(ASM_ERR_OK)
        BIT_DESCR_(ASM_ERR_UNKNOWN)
        BIT_DESCR_(ASM_ERR_CALLOC)
        BIT_DESCR_(ASM_ERR_NULLPTR)
        BIT_DESCR_(ASM_ERR_STAT)
        BIT_DESCR_(ASM_ERR_INPUT_DATA)
        BIT_DESCR_(ASM_ERR_MEM)
        BIT_DESCR_(ASM_ERR_FILE_CLOSE)
        BIT_DESCR_(ASM_ERR_FILE_OPEN)
        BIT_DESCR_(ASM_ERR_ARGS)
        BIT_DESCR_(ASM_ERR_WRONG_COEF)
        BIT_DESCR_(ASM_ERR_INIT)
        BIT_DESCR_(ASM_ERR_SYNTAX)

        default: return "VERY STRANGE ERROR:(";
    }
    #undef BIT_DESCR_
}

const char *get_descr(unsigned long long err) {
    #define DESCR_(code, err)           \
    {                                   \
        if (code & err) {               \
            return #err;                \
        }                               \
    }                                   \

    DESCR_(err, ASM_ERR_NULLPTR)
    DESCR_(err, ASM_ERR_CALLOC)
    DESCR_(err, ASM_ERR_UNKNOWN)
    DESCR_(err, ASM_ERR_STAT)
    DESCR_(err, ASM_ERR_INPUT_DATA)
    DESCR_(err, ASM_ERR_FILE_OPEN)
    DESCR_(err, ASM_ERR_FILE_CLOSE)
    DESCR_(err, ASM_ERR_ARGS)
    DESCR_(err, ASM_ERR_WRONG_COEF)
    DESCR_(err, ASM_ERR_INIT)

    if (err == ASM_ERR_OK) {
        return "ASM_ERR_OK";
    }

    return "STRANGE ERROR!";
}

void asm_add_err(asm_err *src, asm_err add) {
    *src = (asm_err) ((unsigned long long) (*src) | (unsigned long long) (add));
}

#undef DESCR_