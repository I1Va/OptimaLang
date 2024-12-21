#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "AST_io.h"
#include "assembler/inc/general.h"
#include "general.h"
#include "AST_proc.h"
#include "string_funcs.h"
#include "back_args_proc.h"
#include "ast_translator.h"


const char LOG_FILE_PATH[] = "./logs/log.html";
const size_t CHUNK_SIZE = 1024;
const char DOT_DIR_PATH[] = "./logs";
const char DOT_FILE_NAME[] = "graph.dot";
const char DOT_IMG_NAME[] = "gr_img.png";
const char ASM_CODE_PATH[] = "./asm_code.txt";
char bufer[BUFSIZ] = {};

int main(const int argc, const char *argv[]) {
    main_config_t main_config = {}; main_config_get(&main_config, argc, argv);
    ast_tree_t tree           = {}; ast_tree_ctor(&tree, LOG_FILE_PATH);
    dot_code_t dot_code       = {}; dot_code_t_ctor(&dot_code, LIST_DOT_CODE_PARS);
    dot_dir_t dot_dir         = {}; dot_dir_ctor(&dot_dir, DOT_DIR_PATH, DOT_FILE_NAME, DOT_IMG_NAME);

    str_storage_t *storage    = str_storage_t_ctor(CHUNK_SIZE);
    str_t text                = read_text_from_file(main_config.input_file);

    tree.root = load_ast_tree(text.str_ptr, &storage, bufer);

    translate_ast_to_asm_code(ASM_CODE_PATH, &tree);

    assembler_make_bin_code(ASM_CODE_PATH, main_config.output_file);

    convert_subtree_to_dot(tree.root, &dot_code, &storage);

    dot_code_render(&dot_dir, &dot_code);

    FREE(text.str_ptr);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);

    return EXIT_SUCCESS;
}
