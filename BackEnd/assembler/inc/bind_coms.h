#ifndef BIN_COMS_H
#define BIN_COMS_H

#include <stdio.h>
#include <limits.h>


const size_t max_bin_code_sz = 1024;

const int MASK_MEM  = 1 << 8;
const int MASK_REG  = 1 << 7;
const int MASK_IMMC = 1 << 6;
const int filter_mask = INT_MAX & ~MASK_IMMC & ~MASK_MEM & ~MASK_REG;

enum asm_coms_nums
{
    NULL_COM = 0,

    IN_COM = 3,
    OUT_COM = 4,
    ADD_COM = 5,
    SUB_COM = 6,
    MULT_COM = 7,
    JMP_COM = 10,
    LABEL_COM = 11,
    JA_COM = 12,
    JAE_COM = 13,
    JB_COM = 14,
    JBE_COM = 15,
    JE_COM = 16,
    JNE_COM = 17,
    CALL_COM = 18,
    RET_COM = 19,

    PUSH_COM = 20,
    POP_COM = 21,
    OUTC_COM = 22,
    DRAW_COM = 23,
    DIV_COM  = 24,
    SQRT_COM = 25,


    HLT_COM = INT_MAX & filter_mask,
    UNKNOWN_COM = 31,
};

#endif // BIN_COMS_H