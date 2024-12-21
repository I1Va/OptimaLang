#ifndef LANG_GLOBAL_SPACE_H
#define LANG_GLOBAL_SPACE_H

#include <string.h>
#include "string_funcs.h"
#include "AST_io.h"

const size_t TOKEN_LIST_MAX_SZ = 1028;

const size_t PARSER_ERR_GRULE_LIST_SZ = 128;

union token_value_t {
    int ival;
    long long lval;
    long double fval;
    char *sval;
};

struct text_pos_t {
    size_t lines;
    size_t syms;
};

struct lexem_t {
    enum ast_token_t token_type;
    union token_value_t token_val;

    text_pos_t text_pos;
    size_t len;

    bool key_word_state;
};

struct keyword_t {
    const char *name;
    size_t len;
    ast_token_t token_type;
};

struct name_t {
    char *name;
    size_t len;
    ast_token_t token_type;
};

enum grammar_rule_num {
    EMPTY_GRULE = 0,
    PARSE_LEXEMS = 1,
    GET_ADDITIVE_EXPRESSION = 2,
    GET_MULTIPLICATIVE_EXPRESSION = 3,
    GET_DIRECT_DECLARATOR = 4,
    GET_ONE_ARG_FUNCTION_CALL = 5,
    GET_PRIMARY_EXPRESSION = 6,
    GET_CONSTANT = 7,
    GET_VARIABLE = 8,
    GET_SELECTION_STATEMENT = 9,
    GET_STATEMENT = 10,
    GET_LOGICAL_EXPRESSION = 11,
    GET_ASSIGNMENT = 12,
    GET_EXPRESSION = 13,
    GET_WHILE = 14,
    GET_VARIABLE_INITIALIZATION = 15,
    GET_VARIABLE_INITIALIZATION_WITH_ASSIGNMENT = 16,
    GET_FUNC_SEPARATED_INIT_ARGS = 17,
    GET_FUNCTION_INITIALIZATION = 18,
    GET_FUNC_IDENTIFICATOR = 19,
    GET_STATEMENT_LIST = 20,
    GET_GLOBAL_STATEMENT = 21,
    GET_FUNCTION_CALL = 22,
    GET_GRULE_DIVIDED_LIST = 23,
    GET_SCOPE = 24,
    GET_CONT_RET_BREAK = 25,
    GET_RETURN = 26,
    GET_TYPE = 27,
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

    keyword_t *keywords_table;
    size_t keywords_table_sz;
    name_t *name_table;
    size_t name_table_sz;

    lexem_t *lexem_list;
    size_t lexem_list_idx;
    size_t lexem_list_size;

    parser_err_t parser_err;

    str_storage_t **storage;

    FILE *asm_code_file_ptr;
};

const lexem_t EMPTY_LEXEM = {AST_EMPTY, };

bool parsing_block_t_ctor(parsing_block_t *data, char *text,
    keyword_t keywords_table[], name_t *name_table,
    lexem_t *lexem_list, str_storage_t **storage, const char asm_code_file_path[]);

void parsing_block_t_dtor(parsing_block_t *data);
bool keywords_table_fill(keyword_t keywords_table[], size_t *keywords_table_size);

#endif // LANG_GLOBAL_SPACE_H