#ifndef AST_TRANSLATOR_H
#define AST_TRANSLATOR_H

#include <string.h>
#include "AST_io.h"

#include "stack_funcs.h"

const size_t MAX_FUNC_TABLE_SZ = 128;

void translate_node_to_asm_code(ast_tree_elem_t *node, stack_t *var_stack);
void var_stack_remove_local_variables(stack_t *var_stack);
void translate_ast_to_asm_code(const char path[], ast_tree_elem_t *root, stack_t *var_stack);

struct var_t {
    int deep;
    int type;
    int name_id;
    char *name;
    int loc_addr;
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

    T_DIVIDER = 16,

    T_MORE = 17,
    T_LESS = 18,
    T_MORE_EQ = 19,
    T_LESS_EQ = 20,
    T_EQ = 21,

    T_INT = 22,
    T_FLOAT = 23,

    T_ASSIGN = 24,
    T_COMMA = 25,
    T_RETURN = 26,
    T_ELSE = 27,

    T_BREAK = 28,
    T_CONTINUE = 29,

};

struct func_info_t {
    int return_type_num;
    size_t argc;
    int name_id;
    char *name;
};


#endif // AST_TRANSLATOR_H