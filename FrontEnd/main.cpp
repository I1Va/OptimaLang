#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "AST_io.h"
#include "general.h"
#include "AST_proc.h"
#include "lang_global_space.h"
#include "lang_lexer.h"
#include "string_funcs.h"
#include "lang_grammar.h"
#include "front_args_proc.h"

const char LOG_FILE_PATH[] = "./logs/log.html";
const size_t CHUNK_SIZE = 1024;
const char DOT_DIR_PATH[] = "./logs";
const char DOT_FILE_NAME[] = "graph.dot";
const char DOT_IMG_NAME[] = "gr_img.png";
const size_t INDENT = 4;

int main(const int argc, const char *argv[]) {
    main_config_t main_config = {}; main_config_get(&main_config, argc, argv);
    ast_tree_t tree           = {}; ast_tree_ctor(&tree, LOG_FILE_PATH);
    dot_code_t dot_code       = {}; dot_code_t_ctor(&dot_code, LIST_DOT_CODE_PARS);
    dot_dir_t dot_dir         = {}; dot_dir_ctor(&dot_dir, DOT_DIR_PATH, DOT_FILE_NAME, DOT_IMG_NAME);
    str_storage_t *storage    = str_storage_t_ctor(CHUNK_SIZE);
    str_t text                = read_text_from_file(main_config.input_file);

    lexem_t lexem_list[LEXEM_LIST_MAX_SIZE]           = {};
    keyword_t keywords_table[KEY_WORD_TABLE_MAX_SIZE] = {};
    name_t name_table[NAME_TABLE_MAX_SIZE]            = {};

    parsing_block_t data = {};
    parsing_block_t_ctor(&data, text.str_ptr, keywords_table, name_table, lexem_list, &storage, main_config.output_file);


    lex_scanner(&data);

    tree.root = get_syntax_analysis(&data);
    if (check_parser_err(stdout, &data)) {
        CLEAR_MEMORY(exit_mark);
    }

    convert_subtree_to_dot(tree.root, &dot_code, &storage);
    dot_code_render(&dot_dir, &dot_code);
    ast_tree_file_dump(main_config.output_file, &tree, INDENT);


    FREE(text.str_ptr);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);
    parsing_block_t_dtor(&data);
    return EXIT_SUCCESS;

    exit_mark:
    FREE(text.str_ptr);
    str_storage_t_dtor(storage);
    parsing_block_t_dtor(&data);
    return EXIT_FAILURE;
}
