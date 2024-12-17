#include "lang_global_space.h"
#include "lang_lexer.h"
#include "general.h"

bool parsing_block_t_ctor(parsing_block_t *data, char *text,
    keyword_t keywords_table[], const size_t keywords_table_sz, name_t *name_table,
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
    data->keywords_table_sz = keywords_table_sz;
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
