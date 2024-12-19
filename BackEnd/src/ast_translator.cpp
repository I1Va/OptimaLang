#include <cstdio>
#include <stdlib.h>
#include <assert.h>

#include "general.h"
#include "AST_proc.h"
#include "AST_io.h"
#include "stack_funcs.h"
#include "string_funcs.h"
#include "ast_translator.h"
#include "stack_output.h"


char bufer[BUFSIZ] = {};
#define DEBUG_NODE(node)                                           \
    {                                                              \
        get_node_string(bufer, node);                              \
        fprintf_title(stdout, "DEBUG_NODE", '-', STR_F_BORDER_SZ); \
        printf("debug_node: %s\n", bufer);                         \
        fprintf_border(stdout, '-', STR_F_BORDER_SZ, true);        \
    }

static FILE *asm_code_ptr = NULL;
stk_err stack_error = STK_ERR_OK;

const int INDENT_SPACES = 4;

int frame_pointer = 0;
int cur_scope_deep = 0;
static size_t while_cnt = 0;

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


void call_t_fprintf(FILE *stream, void *elem_ptr) {
    call_t call_info = *(call_t *) elem_ptr;
    switch (call_info.act_type) {
        case OLD_STACK_FRAME_RESTORE: fprintf(stream, "FRAME_RESTORE(ebp=%d, deep=%d)", call_info.frame_pointer, call_info.scope_deep); break;
        case INIT_LOC_VAR: fprintf(stream, "LOC VAR INIT(loc_ram[%d], type=%d, id=%d, deep=%d)",
            call_info.var_loc_ram, call_info.var_type, call_info.var_name_id, call_info.var_scope_deep); break;
        default: fprintf(stream, "UNKNOWN ACT_TYPE(%d)", call_info.act_type); break;
    }
}

void translate_ast_to_asm_code(const char path[], ast_tree_elem_t *root, stack_t *call_stack) {
    assert(path);
    assert(root);
    assert(call_stack);

    asm_code_ptr = fopen(path, "w");
    if (!asm_code_ptr) {
        debug("failed to open: '%s'", path);
        return;
    }
    translate_node_to_asm_code(root, call_stack);

    STACK_DUMP(call_stack, stdout, call_t_fprintf);
}

void translate_node_to_asm_code(ast_tree_elem_t *node, stack_t *call_stack)
{
    assert(node);
    assert(call_stack);

    static ast_tree_elem_t *root = node;





    if (node->data.type == NODE_SEMICOLON) {
        if (node->left) {
            translate_node_to_asm_code(node->left, call_stack);
        }
        if (node->right) {
            translate_node_to_asm_code(node->right, call_stack);
        }
        return;
    }

    if (node->data.type == NODE_SCOPE) {
        cur_scope_deep++;
        translate_node_to_asm_code(node->left, call_stack);
        return;
    }


    switch (node->data.type) {
        case NODE_FUNC_INIT: translate_function_init(node, call_stack); break;

    }

    // }
    // if ( == NODE_SCOPE) {
    //     loc_info.deep++;
    //     translate_node_to_asm_code(stdout, node->left, gl_space, loc_info, asm_info);
    //     return;
    // }

    // if (node->data.type == NODE_SEMICOLON) {
    //     if (node->left) {
    //         translate_node_to_asm_code(stream, node->left, gl_space, loc_info, asm_info);
    //     }
    //     if (node->right) {
    //         translate_node_to_asm_code(stream, node->right, gl_space, loc_info, asm_info);
    //     }
    //     return;
    // }

    // if (node->data.type == NODE_FUNC_INIT) {
    //     char *func_name       = node->right->data.value.sval;
    //     int   return_type_num = node->left->data.value.ival;

    //     node = node->right


    // }






    // if (node->data.type == NODE_NUM) {
    //     asm_write_push_lval(stream, node->data.value.lval);
    // } else if (node->data.type == NODE_FUNC) {
    //     printf("!THERE IS SHOULD BE NODE_FUNC PROCESSING!\n");
    // } else if (node->data.type == NODE_OP) {
    //     if (!node->left || !node->right) {
    //         debug("!node->left || !node->right. exit");
    //     }

    //     translate_node_to_asm_code(stream, node->right);
    //     translate_node_to_asm_code(stream, node->left);

    //     asm_write_op(stream, node->data.value.ival);
    // } else {
    //     asm_write_unknown_object(stream, node->data.type);
    // }

    // if (node == root) {
    //     asm_write_out(stream);
    //     asm_write_hlt(stream);
    //     fclose(stream);
    // }
}

void translate_func_args_init(size_t *argc, ast_tree_elem_t *node, stack_t *call_stack) {
    assert(node);
    assert(call_stack);
    assert(node->right);

    call_t call_info = {};

    int cur_arg_num = (int) *argc;

    (*argc)++;



    if (node->left) {
        translate_func_args_init(argc, node->left, call_stack);
    }

    node = node->right; // var_init

    call_info.act_type        = INIT_LOC_VAR;
    call_info.var_type        = node->left->data.type; // var_type
    call_info.var_name_id     = node->right->data.value.ival; // var_name_idx;
    call_info.var_scope_deep  = cur_scope_deep;

    asm_pop_mem_addr_plus_reg(asm_code_ptr, cur_arg_num, "AX", cur_scope_deep * INDENT_SPACES); // ram addr of var is its position in args

    stack_push(call_stack, &call_info, &stack_error);
}

void translate_function_init(ast_tree_elem_t *node, stack_t *call_stack) {
    assert(node);
    assert(call_stack);
    assert(node->data.type == NODE_FUNC_INIT);

    call_t call_info = {};
    size_t argc = 0;
    int   func_ret_type = 0;
    int   func_name_id = 0;
    char *func_name = NULL;


    call_info.act_type = OLD_STACK_FRAME_RESTORE;
    call_info.frame_pointer = frame_pointer;
    call_info.scope_deep = cur_scope_deep;
    cur_scope_deep = 1;

    stack_push(call_stack, &frame_pointer, &stack_error);
    frame_pointer = (int) call_stack->size - 1; // FIXME:

    func_ret_type = node->left->data.value.ival;
    func_name_id = node->right->data.value.ival;
    func_name = node->right->data.value.sval;

    asm_write_function_label(asm_code_ptr, func_name);


    node = node->right; // func_id

    translate_func_args_init(&argc, node->left, call_stack);
    // write_args_initialization



    // Ищем в таблице имен сколько переменненых ждет функция

}
// void WriteWhile(ast_tree_elem_t *cur_node, stack_t *stack) {
//     int type_of_oper = NODE_WHILE;

//     fprintf(asm_code_ptr,
//                     ";#===========While=========#\n"
//                     "jmp while_check_%lu\n"
//                     "while_start_%lu:\n\n", while_cnt, while_cnt);
//     stack_push(stack, &while_cnt, &stack_error);
//     stack_push(stack, &type_of_oper, &stack_error);
//     while_cnt++;

//     if (cur_node->right) {WriteRecTree(cur_node->right, stack);} //body

//     int save_counter  = 0;
//     int type_operator = 0;
//     stack_pop(stack, &type_of_oper, &stack_error);
//     stack_pop(stack, &save_counter, &stack_error);

//     fprintf(asm_code_ptr, "\nwhile_check_%d:\n\n", save_counter);

//     //Condition
//     WriteWhileCondition(myTree, cur_node->left, save_counter);
//     fprintf(FileProc,   "\n.while_end_%d:\n"
//                         ";#=======End=While========#\n", save_counter);
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

void asm_pop_mem_addr_plus_reg(FILE *stream, int addr, const char reg[], int indent) {
    fprintf(stream, "%*spop [%s+%d];\n", indent, "", reg, addr);
}

void asm_write_unknown_object(FILE *stream, int num) {
    fprintf(stream, "UNKNOWN_OBJECT(%d)\n", num);
}

void asm_write_function_label(FILE *stream, const char label[]) {
    fprintf(stream, "%s:\n", label);
}
