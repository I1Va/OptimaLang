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
#include "diff_DSL.h"

const char LOG_FILE_PATH[] = "./logs/log.html";
const size_t CHUNK_SIZE = 1024;
const char DOT_DIR_PATH[] = "./logs";
const char DOT_FILE_NAME[] = "graph.dot";
const char DOT_IMG_NAME[] = "gr_img.png";
const size_t INDENT = 4;

int main(const int argc, const char *argv[]) {
    main_config_t main_config = {};

    opt_data options[] =
    {
        {"-i", "--input", "%s", &main_config.input_file},
        {"-o", "--output", "%s", &main_config.output_file},
    };

    size_t n_options = sizeof(options) / sizeof(opt_data);

    get_options(argc, argv, options, n_options);

    main_config_print(stdout, &main_config);

    str_storage_t *storage = str_storage_t_ctor(CHUNK_SIZE);
    str_t text = read_text_from_file(main_config.input_file);

    ast_tree_t tree = {};
    dot_code_t dot_code = {}; dot_code_t_ctor(&dot_code, LIST_DOT_CODE_PARS);
    dot_dir_t dot_dir = {}; dot_dir_ctor(&dot_dir, DOT_DIR_PATH, DOT_FILE_NAME, DOT_IMG_NAME);
    ast_tree_ctor(&tree, LOG_FILE_PATH);

    lexem_t lexem_list[LEXEM_LIST_MAX_SIZE] = {};
    key_name_t name_table[NAME_TABLE_MAX_SIZE] =
    {
        {"EMPTY_NAME", 10, T_EMPTY},
        {"if", 2, T_IF},
        {"while", 5, T_WHILE},
        {"int", 3, T_INT},
        {"float", 5, T_FLOAT},
        {"return", 6, T_RETURN},
        {"else", 4, T_ELSE},
    };

    parsing_block_t data = {};
    parsing_block_t_ctor(&data, text.str_ptr, name_table, lexem_list, &storage, main_config.output_file);


    lex_scanner(&data);


    tree.root = get_code_block(&data);
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
    sub_tree_dtor(tree.root);
    parsing_block_t_dtor(&data);
    ast_tree_dtor(&tree);


    return EXIT_FAILURE;
}