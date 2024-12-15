#ifndef ASSEMBLER_FUNCS_H
#define ASSEMBLER_FUNCS_H

#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "error_processing.h"
#include "bind_coms.h"

const size_t max_asm_line_sz = 1024;
const size_t max_asm_command_size = 64; // TODO: заменить на calloc
const size_t max_asm_commands_n = 1024; //TODO: написать динамический массив
const size_t label_list_max_sz = 16; // TODO: сделать динамический массив на void *, поддерживающий разные типы в одном массиве
const size_t max_label_name_sz = 32;
const size_t fix_up_table_max_sz = 16;
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

const int DEFAULT_label_VALUE = -1;

struct reg_t
{
    int id;
    const char *name;
};

struct label_t
{
    int addr;
    char name[max_label_name_sz];
};

struct fix_up_obj_t
{
    size_t label_bin_code_idx;
    // FIXME: add label_number = ..
    char label_name[max_label_name_sz]; // FIXME: делать поиск метки по номерк, а не по названию
};

struct asm_data_t {
    size_t empty_fix_up_idx;
    fix_up_obj_t fix_up_table[fix_up_table_max_sz];
    size_t empty_label_idx;
    label_t label_list[label_list_max_sz];
    asm_code_t asm_code;
    bin_code_t bin_code;
};

struct com_t {
    const char com_name[max_com_sz];
    enum asm_coms_nums com_num;
    void (*write_func)(asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
};

const reg_t reg_list[] =
{
    {0, "AX"},
    {1, "BX"},
    {2, "CX"},
    {3, "DX"},
};

const size_t reg_list_sz = sizeof(reg_list) / sizeof(const reg_t);

struct arg_t {
    int arg_mask;
    int immc;
    char reister_str[register_max_sz];
    bool err;
};

void asm_data_t_ctor(asm_data_t *asm_data);
int get_reg_id(const char name[]);
void label_list_dump(FILE *stream, asm_data_t *asm_data);
void bin_code_dump(FILE *stream, bin_code_t bin_code);
void fill_label_list(asm_data_t *asm_data, int default_addr);
int add_label_to_list(asm_data_t *asm_data, int addr, const char *name);
int get_label_addr_from_list(asm_data_t *asm_data, const char *name);
void fix_up_table_add(asm_data_t *asm_data, const size_t label_bin_code_idx, const char name[]);
void fix_up_table_pull_up(asm_data_t *asm_data, asm_err *return_err);
bool check_label_elem(const char com[]);
void process_label(asm_data_t *asm_data, asm_err *return_err);
void write_register(bin_code_t *bin_code, const char register_str[], asm_err *return_err);
arg_t parse_push_arg(const char arg_str[]);
void fprintf_bin(FILE *stream, int mask);
void write_universal_push(asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
void write_universal_pop(asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
bool asm_end_idx(const asm_data_t *asm_data);
void write_jump(asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
void write_label(asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
void write_conditional_jmp(asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
void write_simple_com(asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
void write_call_com(asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
void asm_com_launch(int asm_com_idx, asm_data_t *asm_data, const enum asm_coms_nums com_num, asm_err *return_err);
int get_bin_idx_from_list(const char name[]);
bool asm_getline(FILE* stream, char line[], const size_t n);
void fprint_asm_commands_list(FILE *stream, asm_data_t *asm_data);
void asm_code_read(asm_code_t *asm_code, const char path[], asm_err *return_err);
void asm_commands_translate(asm_data_t *asm_data, asm_err *return_err);
void bin_code_write(const char path[], bin_code_t bin_code, asm_err *return_err);

const com_t asm_com_list[] =
{
    {"in"    , IN_COM, write_simple_com},
    {"outc"   , OUTC_COM, write_simple_com},
    {"out"   , OUT_COM, write_simple_com},
    {"add"   , ADD_COM, write_simple_com},
    {"sub"   , SUB_COM, write_simple_com},
    {"mult"  , MULT_COM, write_simple_com},
    {"jmp"   , JMP_COM, write_jump},
    {"ja"    , JA_COM, write_conditional_jmp},
    {"jae"   , JAE_COM, write_conditional_jmp},
    {"jb"    , JB_COM, write_conditional_jmp},
    {"jbe"   , JBE_COM, write_conditional_jmp},
    {"je"    , JE_COM, write_conditional_jmp},
    {"jne"   , JNE_COM, write_conditional_jmp},
    {"hlt"   , HLT_COM, write_simple_com},
    {"call"  , CALL_COM, write_call_com},
    {"ret"   , RET_COM, write_simple_com},
    {"draw"  , DRAW_COM, write_simple_com},
    {"div"   , DIV_COM, write_simple_com},
    {"sqrt"  , SQRT_COM, write_simple_com},

    {"push" , PUSH_COM, write_universal_push},
    {"pop" , POP_COM, write_universal_pop},

    {"LABEL:", LABEL_COM, write_label}
};
const size_t asm_com_list_sz = sizeof(asm_com_list) / sizeof(com_t);

#endif // ASSEMBLER_FUNCS_H