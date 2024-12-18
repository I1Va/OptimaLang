#ifndef LANG_GRAMMAR_H
#define LANG_GRAMMAR_H

#include "lang_lexer.h"
#include "AST_proc.h"
#include "lang_global_space.h"

#include "string_funcs.h"

void start_parser_err(parsing_block_t *data, lexem_t lexem, enum grammar_rule_num grule);
void add_grule_to_parser_err(parser_err_t *parser_err, enum grammar_rule_num grule);
void clear_parser_err(parser_err_t *parser_err);
bool check_parser_err(FILE *stream, parsing_block_t *data);

ast_tree_elem_t *get_code_block(parsing_block_t *data);
ast_tree_elem_t *get_additive_expression(parsing_block_t *data);
ast_tree_elem_t *get_multiplicative_expression(parsing_block_t *data);
ast_tree_elem_t *get_direct_declarator(parsing_block_t *data);
ast_tree_elem_t *get_statement(parsing_block_t *data);
ast_tree_elem_t *get_selection_statement(parsing_block_t *data);
ast_tree_elem_t *get_logical_expression(parsing_block_t *data);
ast_tree_elem_t *get_function_call(parsing_block_t *data);
ast_tree_elem_t *get_primary_expression(parsing_block_t *data);
ast_tree_elem_t *get_constant(parsing_block_t *data);
ast_tree_elem_t *get_variable(parsing_block_t *data);
ast_tree_elem_t *get_while(parsing_block_t *data);
ast_tree_elem_t *get_expression(parsing_block_t *data);
ast_tree_elem_t *get_assignment(parsing_block_t *data);
ast_tree_elem_t *get_variable_initialization_with_assignment(parsing_block_t *data);
ast_tree_elem_t *get_variable_initialization(parsing_block_t *data);
ast_tree_elem_t *get_func_identificator(parsing_block_t *data);
ast_tree_elem_t *get_grule_divided_list(parsing_block_t *data, int *empty, ast_tree_elem_t *(*grule_func)(parsing_block_t *data));
ast_tree_elem_t *get_function_initialization(parsing_block_t *data);
ast_tree_elem_t *get_global_statement(parsing_block_t *data);
ast_tree_elem_t *try_grule(parsing_block_t *data, ast_tree_elem_t *(*grule_func)(parsing_block_t *data));
bool check_token_on_logical_class(const token_t token);
bool check_token_on_additive_class(const token_t token);
bool check_token_on_multiplicative_class(const token_t token);
bool check_token_on_type_class(const token_t token);
ast_tree_elem_t *get_type(parsing_block_t *data);
ast_tree_elem_t* get_func_separated_init_args(parsing_block_t*);
ast_tree_elem_t *get_scope(parsing_block_t *data, int *empty);
ast_tree_elem_t *get_cont_ret_break(parsing_block_t *data);

void dump_last_lex(parsing_block_t *data);

#endif // LANG_GRAMMAR_H