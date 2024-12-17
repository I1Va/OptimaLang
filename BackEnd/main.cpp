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
#include "string_funcs.h"
#include "back_args_proc.h"


const char LOG_FILE_PATH[] = "./logs/log.html";
const size_t CHUNK_SIZE = 1024;
const char DOT_DIR_PATH[] = "./logs";
const char DOT_FILE_NAME[] = "graph.dot";
const char DOT_IMG_NAME[] = "gr_img.png";
const size_t INDENT = 4;

// {"in"    , IN_COM, write_simple_com},
// {"outc"   , OUTC_COM, write_simple_com},
// {"out"   , OUT_COM, write_simple_com},
// {"add"   , ADD_COM, write_simple_com},
// {"sub"   , SUB_COM, write_simple_com},
// {"mult"  , MULT_COM, write_simple_com},
// {"jmp"   , JMP_COM, write_jump},
// {"ja"    , JA_COM, write_conditional_jmp},
// {"jae"   , JAE_COM, write_conditional_jmp},
// {"jb"    , JB_COM, write_conditional_jmp},
// {"jbe"   , JBE_COM, write_conditional_jmp},
// {"je"    , JE_COM, write_conditional_jmp},
// {"jne"   , JNE_COM, write_conditional_jmp},
// {"hlt"   , HLT_COM, write_simple_com},
// {"call"  , CALL_COM, write_call_com},
// {"ret"   , RET_COM, write_simple_com},
// {"draw"  , DRAW_COM, write_simple_com},
// {"div"   , DIV_COM, write_simple_com},
// {"sqrt"  , SQRT_COM, write_simple_com},
// {"push" , PUSH_COM, write_universal_push},
// {"pop" , POP_COM, write_universal_pop},
// {"LABEL:", LABEL_COM, write_label}

// void asm_write_op(FILE *stream, int op_num) {
//     switch (op_num) {
//         case OP_MUL: fprintf(stream, "mult;\n"); break;
//         case OP_DIV: fprintf(stream, "div;\n"); break;
//         case OP_SUB: fprintf(stream, "sub;\n"); break;
//         case OP_ADD: fprintf(stream, "add;\n"); break;
//         default: debug("unknown operation: '%d'", op_num); fprintf(stream, "?%d?\n", op_num); break;
//     }
// }

// void asm_write_hlt(FILE *stream) {
//     fprintf(stream, "hlt;\n");
// }

// void asm_write_out(FILE *stream) {
//     fprintf(stream, "out;\n");
// }

// void asm_write_push_lval(FILE *stream, long long lval) {
//     fprintf(stream, "push %Ld;\n", lval);
// }

// void asm_write_unknown_object(FILE *stream, int num) {
//     fprintf(stream, "UNKNOWN_OBJECT(%d)\n", num);
// }

// void translate_ast_to_asm_code(FILE *stream, ast_tree_elem_t *node) {
//     assert(node);
//     static ast_tree_elem_t *root = node;

//     if (node->data.type == NODE_NUM) {
//         asm_write_push_lval(stream, node->data.value.lval);
//     } else if (node->data.type == NODE_FUNC) {
//         printf("!THERE IS SHOULD BE NODE_FUNC PROCESSING!\n");
//     } else if (node->data.type == NODE_OP) {
//         if (!node->left || !node->right) {
//             debug("!node->left || !node->right. exit");
//         }

//         translate_ast_to_asm_code(stream, node->right);
//         translate_ast_to_asm_code(stream, node->left);

//         asm_write_op(stream, node->data.value.ival);
//     } else {
//         asm_write_unknown_object(stream, node->data.type);
//     }

//     if (node == root) {
//         asm_write_out(stream);
//         asm_write_hlt(stream);
//         fclose(stream);
//     }
// }

// void assembler_make_bin_code(const char asm_code_path[], const char bin_code_path[]) {
//     char bufer[MEDIUM_BUFER_SZ] = {};

//     snprintf(bufer, MEDIUM_BUFER_SZ, "cd ./assembler && make launch -f Makefile LAUNCH_FLAGS=\"-i=./.%s -o=./.%s\"", asm_code_path, bin_code_path);
//     system(bufer);
// }

// void processor_execute_bin_code(const char bin_code_path[]) {
//     char bufer[MEDIUM_BUFER_SZ] = {};

//     snprintf(bufer, MEDIUM_BUFER_SZ, "cd ./processor && make launch -f Makefile LAUNCH_FLAGS=\"-i=./.%s\"", bin_code_path);
//     system(bufer);
// }

const char ASM_CODE_PATH[] = "./asm_code.txt";
const char BIN_CODE_PATH[] = "./bin_code.txt";

int main(const int argc, const char *argv[]) {
    char bufer[BUFSIZ] = {};

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

    ast_tree_t tree = {};
    dot_code_t dot_code = {}; dot_code_t_ctor(&dot_code, LIST_DOT_CODE_PARS);
    dot_dir_t dot_dir = {}; dot_dir_ctor(&dot_dir, DOT_DIR_PATH, DOT_FILE_NAME, DOT_IMG_NAME);
    ast_tree_ctor(&tree, LOG_FILE_PATH);

    tree.root = load_ast_tree(text.str_ptr, &storage, bufer);

    ast_tree_file_dump("ast_copy.txt", &tree, 4);

    FILE *asm_code_file_ptr = fopen(ASM_CODE_PATH, "w");
    if (!asm_code_file_ptr) {
        debug("open '%s' failed", main_config.output_file);
        CLEAR_MEMORY(exit_mark);
    }

    // translate_ast_to_asm_code(asm_code_file_ptr, tree.root);

    // assembler_make_bin_code(ASM_CODE_PATH, BIN_CODE_PATH);

    // processor_execute_bin_code(BIN_CODE_PATH);

    convert_subtree_to_dot(tree.root, &dot_code, &storage);

    dot_code_render(&dot_dir, &dot_code);

    FREE(text.str_ptr);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);

    return EXIT_SUCCESS;

    exit_mark:
    FREE(text.str_ptr);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);

    return EXIT_FAILURE;
}
