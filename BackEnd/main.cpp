#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "AST_io.h"
#include "assembler/inc/general.h"
#include "general.h"
#include "AST_proc.h"
#include "stack_err_proc.h"
#include "stack_funcs.h"
#include "string_funcs.h"
#include "back_args_proc.h"
#include "ast_translator.h"


const char LOG_FILE_PATH[] = "./logs/log.html";
const size_t CHUNK_SIZE = 1024;
const char DOT_DIR_PATH[] = "./logs";
const char DOT_FILE_NAME[] = "graph.dot";
const char DOT_IMG_NAME[] = "gr_img.png";
const size_t INDENT = 4;
const char ASM_CODE_PATH[] = "./asm_code.txt";
const char BIN_CODE_PATH[] = "./bin_code.txt";

int main(const int argc, const char *argv[]) {
    setbuf(stderr, 0);

    char bufer[BUFSIZ] = {};
    stk_err stack_error = STK_ERR_OK;

    main_config_t main_config = {};

    opt_data options[] =
    {
        {"-i", "--input", "%s", &main_config.input_file},
        {"-o", "--output", "%s", &main_config.output_file},
    };

    size_t n_options = sizeof(options) / sizeof(opt_data);

    get_options(argc, argv, options, n_options);

    main_config_print(stdout, &main_config);
    // fprintf_grn(stdout, "POINT\n");

    str_storage_t *storage = str_storage_t_ctor(CHUNK_SIZE);
    str_t text = read_text_from_file(main_config.input_file);

    ast_tree_t tree = {}; ast_tree_ctor(&tree, LOG_FILE_PATH);
    dot_code_t dot_code = {}; dot_code_t_ctor(&dot_code, LIST_DOT_CODE_PARS);
    dot_dir_t dot_dir = {}; dot_dir_ctor(&dot_dir, DOT_DIR_PATH, DOT_FILE_NAME, DOT_IMG_NAME);
    stack_t call_stack = {};;


    tree.root = load_ast_tree(text.str_ptr, &storage, bufer);

    translate_ast_to_asm_code(ASM_CODE_PATH, &tree);



    // assembler_make_bin_code(ASM_CODE_PATH, BIN_CODE_PATH);

    // processor_execute_bin_code(BIN_CODE_PATH);

    convert_subtree_to_dot(tree.root, &dot_code, &storage);

    dot_code_render(&dot_dir, &dot_code);

    stack_destroy(&call_stack);
    FREE(text.str_ptr);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);

    return EXIT_SUCCESS;

    exit_mark:
    FREE(text.str_ptr);
    stack_destroy(&call_stack);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);

    return EXIT_FAILURE;
}
