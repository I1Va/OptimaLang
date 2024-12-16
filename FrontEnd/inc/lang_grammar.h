#ifndef LANG_GRAMMAR_H
#define LANG_GRAMMAR_H

#include "lang_lexer.h"
#include "AST_proc.h"
#include "lang_global_space.h"

#include "string_funcs.h"

void start_parser_err(parser_err_t *parser_err, lexem_t lexem, enum grammar_rule_num grule);
void add_grule_to_parser_err(parser_err_t *parser_err, enum grammar_rule_num grule);
void clear_parser_err(parser_err_t *parser_err);
bool check_parser_err(FILE *stream, parsing_block_t *data);

ast_tree_elem_t *get_code_block(parsing_block_t *data);
ast_tree_elem_t *get_additive_expression(parsing_block_t *data);
ast_tree_elem_t *get_multiplicative_expression(parsing_block_t *data);
ast_tree_elem_t *get_direct_declarator(parsing_block_t *data);
ast_tree_elem_t *get_statement(parsing_block_t *data);
ast_tree_elem_t *get_selection_statement(parsing_block_t *data);

ast_tree_elem_t *get_function(parsing_block_t *data);
ast_tree_elem_t *get_primary_expression(parsing_block_t *data);
ast_tree_elem_t *get_constant(parsing_block_t *data);
ast_tree_elem_t *get_identificator(parsing_block_t *data);

#endif // LANG_GRAMMAR_H