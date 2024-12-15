#ifndef PROC_OUTPUT_H
#define PROC_OUTPUT_H

#include <assert.h>
#include <cstddef>
#include <stdio.h>
#include <string.h>

const size_t border_size = 100;

#define GRN "\e[0;32m"
#define WHT "\e[0;20m"
#define RED "\e[0;31m"
#define YEL "\e[0;33m"

#define printf_red(str_, ...) printf(RED str_ WHT, ##__VA_ARGS__) // TODO: дефайны КАПСОМ
#define printf_wht(str_, ...) printf(WHT str_ WHT, ##__VA_ARGS__)
#define printf_grn(str_, ...) printf(GRN str_ WHT, ##__VA_ARGS__)
#define printf_yel(str_, ...) printf(YEL str_ WHT, ##__VA_ARGS__)

#define fprintf_red(stream, str_, ...) fprintf(stream, RED str_ WHT, ##__VA_ARGS__)
#define fprintf_wht(stream, str_, ...) fprintf(stream, WHT str_ WHT, ##__VA_ARGS__)
#define fprintf_grn(stream, str_, ...) fprintf(stream, GRN str_ WHT, ##__VA_ARGS__)
#define fprintf_yel(stream, str_, ...) fprintf(stream, YEL str_ WHT, ##__VA_ARGS__)

void fprintf_bin_code(FILE *stream, int code[], const size_t n);

#endif // PROC_OUTPUT_H