#ifndef ERROR_PROCESSING_H
#define ERROR_PROCESSING_H

#include <stdio.h>

enum asm_err
{
    ASM_ERR_OK                         = 0,
    ASM_ERR_NULLPTR                    = 1 << 0,
    ASM_ERR_CALLOC                     = 1 << 1,
    ASM_ERR_MEM                        = 1 << 2,
    ASM_ERR_UNKNOWN                    = 1 << 3,
    ASM_ERR_STAT                       = 1 << 4,
    ASM_ERR_INPUT_DATA                 = 1 << 5,
    ASM_ERR_FILE_OPEN                  = 1 << 6,
    ASM_ERR_FILE_CLOSE                 = 1 << 7,
    ASM_ERR_ARGS                       = 1 << 8,
    ASM_ERR_WRONG_COEF                 = 1 << 9,
    ASM_ERR_INIT                       = 1 << 10,
    ASM_ERR_SYNTAX                     = 1 << 11,
    ASM_ERR_INVALID_LABEL              = 1 << 12,
};

const char *get_descr(unsigned long long err);

int fprintf_abort(const char file_name[], const int line, const char function_name[], const char error_descr[]);

const char *get_bit_descr(unsigned long long err);

void asm_add_err(asm_err *src, asm_err add);


#ifdef _DEBUG
    #define DEBUG_ERROR(code) fprintf_red(stderr, "{%s} [%s: %d]: descr{%s}\n", __FILE_NAME__, __PRETTY_FUNCTION__, __LINE__, get_bit_descr(code));

#else
    #define DEBUG_ERROR(code) ;

#endif // _DEBUG

#endif // ERROR_PROCESSING_H
