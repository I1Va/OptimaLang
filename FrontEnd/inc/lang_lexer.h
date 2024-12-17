#ifndef LANG_LEXER_H
#define LANG_LEXER_H

#include <stdio.h>
#include <string.h>
#include "lang_global_space.h"

#define ScannerError(p, c) {debug("ScannerError: text[%d] : '%c'", p, c); fprintf(stderr, WHT); abort();}

const size_t LEXEM_LIST_MAX_SIZE = BUFSIZ;
const size_t NAME_TABLE_MAX_SIZE = 1024;

bool char_in_str_lex(int c);

int get_index_in_name_table(char *new_name, parsing_block_t *data);
int get_index_in_keyword_table(char *new_name, parsing_block_t *data);
int add_to_name_table(char *new_name, parsing_block_t *data);
lexem_t next_lexem(parsing_block_t *data);
void text_pos_update(text_pos_t *text_pos, const lexem_t lexem);
void lex_scanner(parsing_block_t *data);
size_t scan_lval(long long int*, char*);

#endif // LANG_LEXER_H