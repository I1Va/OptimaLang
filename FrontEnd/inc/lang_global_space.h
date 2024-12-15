#ifndef LANG_GLOBAL_SPACE_H
#define LANG_GLOBAL_SPACE_H

#include <string.h>
#include "string_funcs.h"
#include "diff_tree.h"

const size_t TOKEN_LIST_MAX_SZ = 1028;

const size_t PARSER_ERR_GRULE_LIST_SZ = 128;

union token_value_t {
    int ival;
    long long lval;
    long double dval;
};

enum token_t {
    T_EOF = -1,
    T_EMPTY = 0,

    T_NUM = 1,

    T_ADD = 2,
    T_MUL = 3,
    T_SUB = 4,
    T_DIV = 5,
    T_POW = 6,

    T_O_BRACE = 7,
    T_C_BRACE = 8,
    T_O_FIG_BRACE = 9,
    T_C_FIG_BRACE = 10,

    T_EOL = 11, // '\n'

    T_SPACE = 12,


    T_ID = 13,
    T_IF = 14, // key_words
    T_WHILE = 15,


};

struct text_pos_t {
    size_t lines;
    size_t syms;
};

struct lexem_t {
    enum token_t token_type;
    union token_value_t token_val;

    text_pos_t text_pos;
    size_t len;
};

struct key_name_t {
    char *name;
    size_t len;
    token_t token_type;
};

enum grammar_rule_num {
    EMPTY_GRULE = 0,
    GET_CODE_BLOCK = 1,
    GET_ADDITIVE_EXPRESSION = 2,
    GET_MULTIPLICATIVE_EXPRESSION = 3,
    GET_DIRECT_DECLARATOR = 4,
    GET_FUNCTION = 5,
    GET_PRIMARY_EXPRESSION = 6,
    GET_CONSTANT = 7,
    GET_IDENTIFICATOR = 8,
};

struct parser_err_t {
    bool err_state;
    lexem_t lex;

    grammar_rule_num grule_list[PARSER_ERR_GRULE_LIST_SZ];
    size_t grule_list_size;
};

struct parsing_block_t {
    char *text;
    size_t text_idx;

    key_name_t *name_table;
    size_t name_table_sz;

    lexem_t *lexem_list;
    size_t lexem_list_idx;
    size_t lexem_list_size;

    parser_err_t parser_err;

    str_storage_t **storage;

    FILE *asm_code_file_ptr;
};

#endif // LANG_GLOBAL_SPACE_H