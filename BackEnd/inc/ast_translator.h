#ifndef AST_TRANSLATOR_H
#define AST_TRANSLATOR_H

#include <string.h>
#include "AST_io.h"

#include "AST_proc.h"
#include "stack_funcs.h"

const size_t MAX_FUNC_TABLE_SZ = 128;
const size_t ASM_BORDER_SIZE = 27;

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

struct reserved_func_info_t {
    const char *name;
    int return_type_num;
    size_t argc;
    void (*translate_func)(ast_tree_elem_t *node);
};

struct func_info_t {
    char *name;
    int return_type_num;
    size_t argc;
};

void translate_reserved_print_call(ast_tree_elem_t *node);
void translate_reserved_input_call(ast_tree_elem_t *node);

void init_stacks(FILE *log_file_ptr);
void translate_ast_to_asm_code(const char path[], ast_tree_t *tree);
void var_stack_remove_local_variables();

int get_func_idx_in_name_table(func_info_t func_info);
int get_func_idx_in_reserved_name_table(func_info_t func_info);
void add_function_to_name_table(func_info_t func_info);

var_t get_var_from_frame(int name_id);
void var_t_fprintf(FILE *stream, void *elem_ptr);
int add_var_into_frame(var_t var);
void translate_func_args_init(size_t *argc, ast_tree_elem_t *node);
void var_stack_restore_old_frame();
void translate_function_init(ast_tree_elem_t *node);
void translate_node_to_asm_code(ast_tree_elem_t *node);
void translate_while(ast_tree_elem_t *node);
void translate_while_condition(ast_tree_elem_t *node, int curr_counter);
void translate_if(ast_tree_elem_t *node);
void translate_if_condition(ast_tree_elem_t *node, int curr_counter);
void translate_func_call_args(size_t *argc, ast_tree_elem_t *node);
void translate_func_call(ast_tree_elem_t *node);
void translate_var_init(ast_tree_elem_t *node);
size_t count_node_type_in_subtreeas(ast_tree_elem_t *node, const enum node_types node_type);
void translate_return(ast_tree_elem_t *node);
void write_asm_tittle(FILE *stream, const char tittle[], const size_t bord_sz=ASM_BORDER_SIZE);
void fprintf_asm_border(FILE* stream, const char bord_char, const size_t bord_sz, bool new_line);

void translate_assign(ast_tree_elem_t *node);
void translate_scope(ast_tree_elem_t *node);
void translate_semicolon(ast_tree_elem_t *node);
void translate_var(ast_tree_elem_t *node);
void dump_global_info(FILE *stream=stdout);

#endif // AST_TRANSLATOR_H