#ifndef LANG_LOGGER_H
#define LANG_LOGGER_H

#include "lang_grammar.h"

void lexem_dump(FILE *stream, key_name_t *name_table, lexem_t lexem);
void lexem_list_dump(FILE *stream, parsing_block_t *data);
void name_table_dump(FILE *stream, key_name_t *name_table, const size_t name_table_sz);
void grule_dump(FILE *stream, enum grammar_rule_num grule);

void fprintf_border(FILE* stream, const char bord_char, const size_t bord_sz, bool new_line);
void fprintf_title(FILE *stream, const char tittle[], const char bord_char, const size_t bord_sz);

#endif // LANG_LOGGER_H