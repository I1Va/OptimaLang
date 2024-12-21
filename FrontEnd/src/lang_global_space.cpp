#include "lang_global_space.h"
#include "lang_lexer.h"
#include "general.h"
#include <cstddef>

bool keywords_table_fill(keyword_t keywords_table[], size_t *keywords_table_size) {
    keyword_t temp_table[] =
    {
        {"EMPTY_NAME", 10, AST_EMPTY},
        {"if", 2, AST_IF},
        {"while", 5, AST_WHILE},
        {"int", 3, AST_INT},
        {"float", 5, AST_FLOAT},
        {"return", 6, AST_RETURN},
        {"else", 4, AST_ELSE},
        {"void", 4, AST_VOID},
    };

    *keywords_table_size = sizeof(temp_table) / sizeof(keyword_t);
    if (*keywords_table_size > KEY_WORD_TABLE_MAX_SIZE) {
        return false;
    }

    for (size_t i = 0; i < *keywords_table_size; i++) {
        keywords_table[i] = temp_table[i];
    }

    return true;
}

bool parsing_block_t_ctor(parsing_block_t *data, char *text,
    keyword_t keywords_table[], name_t *name_table,
    lexem_t *lexem_list, str_storage_t **storage, const char asm_code_file_path[])
{
    data->text_idx = 0;
    data->text = text;

    data->lexem_list = lexem_list;
    data->lexem_list_idx = 0;
    data->lexem_list_size = 0;

    data->parser_err = {};

    data->storage = storage;
    data->keywords_table = keywords_table;
    data->name_table = name_table;
    keywords_table_fill(data->keywords_table, &data->keywords_table_sz);

    data->name_table = name_table;
    data->name_table_sz = 0;

    data->asm_code_file_ptr = fopen(asm_code_file_path, "w");
    if (!data->asm_code_file_ptr) {
        debug("failed to open '%s'", asm_code_file_path);
        return false;
    }

    return true;
}

void parsing_block_t_dtor(parsing_block_t *data) {
    if (data && data->asm_code_file_ptr) {
        fclose(data->asm_code_file_ptr);
    }
}
