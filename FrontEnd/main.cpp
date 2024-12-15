#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "general.h"
#include "diff_tree.h"
#include "lang_global_space.h"
#include "diff_funcs.h"
#include "lang_lexer.h"
#include "string_funcs.h"
#include "lang_grammar.h"
#include "diff_DSL.h"

const char LOG_FILE_PATH[] = "./logs/log.html";
const char DOT_FILE_NAME[] = "graph.dot";
const char DOT_IMG_NAME[] = "gr_img.png";
const char CODE_FILE_PATH[] = "./code.txt";
const char ASM_CODE_FILE_PATH[] = "./asm_code.txt";

int main() {
    str_storage_t *storage = str_storage_t_ctor(CHUNK_SIZE);
    str_t text = read_text_from_file(CODE_FILE_PATH);

    bin_tree_t tree = {};
    bin_tree_ctor(&tree, LOG_FILE_PATH);

    lexem_t lexem_list[LEXEM_LIST_MAX_SIZE] = {};
    key_name_t name_table[NAME_TABLE_MAX_SIZE] =
    {
        {"EMPTY_NAME", 10, T_EMPTY},
        {"if", 2, T_IF},
        {"while", 5, T_WHILE},
    };

    parsing_block_t data = {};
    parsing_block_t_ctor(&data, text.str_ptr, name_table, lexem_list, &storage, ASM_CODE_FILE_PATH);


    lex_scanner(&data);



    tree.root = get_code_block(&data);
    if (check_parser_err(stdout, &data)) {
        CLEAR_MEMORY(exit_mark);
    }



    FREE(text.str_ptr);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);
    parsing_block_t_dtor(&data);

    return EXIT_SUCCESS;


    exit_mark:
    FREE(text.str_ptr);
    str_storage_t_dtor(storage);
    sub_tree_dtor(tree.root);
    parsing_block_t_dtor(&data);
    bin_tree_dtor(&tree);


    return EXIT_FAILURE;
}
