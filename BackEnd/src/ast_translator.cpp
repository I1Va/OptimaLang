#include <stdlib.h>
#include <assert.h>

#include "general.h"
#include "AST_proc.h"
#include "AST_io.h"

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

void asm_write_op(FILE *stream, int op_num) {
    switch (op_num) {
        case AST_MUL: fprintf(stream, "mult;\n"); break;
        case AST_DIV: fprintf(stream, "div;\n"); break;
        case AST_SUB: fprintf(stream, "sub;\n"); break;
        case AST_ADD: fprintf(stream, "add;\n"); break;
        default: debug("unknown operation: '%d'", op_num); fprintf(stream, "?%d?\n", op_num); break;
    }
}

void asm_write_hlt(FILE *stream) {
    fprintf(stream, "hlt;\n");
}

void asm_write_out(FILE *stream) {
    fprintf(stream, "out;\n");
}

void asm_write_push_lval(FILE *stream, long long lval) {
    fprintf(stream, "push %Ld;\n", lval);
}

void asm_write_unknown_object(FILE *stream, int num) {
    fprintf(stream, "UNKNOWN_OBJECT(%d)\n", num);
}

void translate_ast_to_asm_code(FILE *stream, ast_tree_elem_t *node) {
    assert(node);
    static ast_tree_elem_t *root = node;

    if (node->data.type == NODE_NUM) {
        asm_write_push_lval(stream, node->data.value.lval);
    } else if (node->data.type == NODE_FUNC) {
        printf("!THERE IS SHOULD BE NODE_FUNC PROCESSING!\n");
    } else if (node->data.type == NODE_OP) {
        if (!node->left || !node->right) {
            debug("!node->left || !node->right. exit");
        }

        translate_ast_to_asm_code(stream, node->right);
        translate_ast_to_asm_code(stream, node->left);

        asm_write_op(stream, node->data.value.ival);
    } else {
        asm_write_unknown_object(stream, node->data.type);
    }

    if (node == root) {
        asm_write_out(stream);
        asm_write_hlt(stream);
        fclose(stream);
    }
}

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
