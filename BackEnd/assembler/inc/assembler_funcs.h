#ifndef ASSEMBLER_FUNCS_H
#define ASSEMBLER_FUNCS_H

#include <stdio.h>
#include <limits.h>

#include "error_processing.h"
#include "./../../binder_dir/bind_coms.h"

const size_t max_asm_line_sz = 1024;
const size_t max_asm_command_size = 64; // TODO: заменить на calloc
const size_t max_asm_commands_n = 1024; //TODO: написать динамический массив
const size_t label_list_max_sz = 128; // TODO: сделать динамический массив на void *, поддерживающий разные типы в одном массиве
const size_t max_label_name_sz = 64;
const size_t fix_up_table_max_sz = 128;
const size_t max_com_sz = 16;
const size_t register_max_sz = 8;

struct bin_code_t {
    int code[max_bin_code_sz] = {};
    size_t bin_idx = 0;
};

struct asm_code_t {
    char code[max_asm_commands_n][max_asm_command_size] = {};
    size_t asm_idx = 0;
    size_t code_sz = max_asm_commands_n;
};

asm_code_t asm_code_init();

bin_code_t bin_code_init();

asm_code_t asm_code_read(const char path[], asm_err *return_err);

bool asm_getline(FILE* stream, char line[], const size_t n);

void asm_commands_translate(bin_code_t *bin_code, asm_code_t *asm_code, asm_err *return_err);

void fprint_asm_commands_list(FILE *stream, const char asm_commands[][max_asm_command_size], const size_t n_coms);

void bin_code_write(const char path[], bin_code_t bin_code, asm_err *return_err);

#endif // ASSEMBLER_FUNCS_H