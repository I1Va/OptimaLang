#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "general.h"
#include "AST_proc.h"
#include "AST_io.h"
#include "stack_funcs.h"
#include "string_funcs.h"
#include "ast_translator.h"
#include "stack_output.h"

#define CHECK_NODE_TYPE(node, exp_type)                                            \
    if (node->data.type != exp_type) {                                             \
        RAISE_TR_ERROR("invalid_node: {%d}, expected: "#exp_type, node->data.type) \
    }


#define RAISE_TR_ERROR(str_, ...) fprintf_red(stderr, "{%s} [%s: %d]: translator_error{" str_ "}\n", __FILE_NAME__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); abort();

static FILE *asm_code_ptr = NULL;
int cur_scope_deep = 0;
int cur_frame_ptr = 0;
func_info_t func_name_table[MAX_FUNC_TABLE_SZ] = {};
size_t func_table_sz = 0;

void translate_ast_to_asm_code(const char path[], ast_tree_elem_t *root, stack_t *var_stack) {
    assert(path);
    assert(root);
    assert(var_stack);

    asm_code_ptr = fopen(path, "w");
    if (!asm_code_ptr) {
        debug("failed to open: '%s'", path);
        return;
    }
    translate_node_to_asm_code(root, var_stack);
}

void var_stack_remove_local_variables(stack_t *var_stack) {
    assert(var_stack);

    var_t last_elem = {};
    stack_get_elem(var_stack, &last_elem, var_stack->size - 1);
    while (last_elem.deep > cur_scope_deep) {
        stack_pop(var_stack);
        stack_get_elem(var_stack, &last_elem, var_stack->size - 1);
    }
}

int get_func_idx_in_name_table(func_info_t func_info) {
    for (size_t i = 0; i < func_table_sz; i++) {
        if (func_name_table[i].name_id == func_info.name_id) {
            return (int) i;
        }
    }
    return -1;
}

void add_function_to_name_table(func_info_t func_info) {
    if (get_func_idx_in_name_table(func_info) != -1) {
        RAISE_TR_ERROR("function '%s' redefenition", func_info.name);
        return;
    }
    func_name_table[func_table_sz++] = func_info;
}

int get_var_in_frame(int name_id, stack_t *var_stack) {
    assert(var_stack);

    var_t var_info = {};

    for (int i = (int) var_stack->size; i > cur_frame_ptr; i--) {
        stack_get_elem(var_stack, &var_info, (size_t) i);
        if (var_info.name_id == name_id) {
            return i;
        }
    }
    return -1;
}

void var_t_fprintf(FILE *stream, void *elem_ptr) {
    var_t var = *(var_t *) elem_ptr;
    fprintf(stream, "LOC VAR INIT(loc_ram[%d], type=%d, name='%s', id=%d, deep=%d)",
        var.loc_addr, var.type, var.name, var.name_id, var.deep);
}

int add_var_into_frame(var_t var, stack_t *var_stack) {
    assert(var_stack);

    int stack_idx = get_var_in_frame(var.name_id, var_stack);
    if (stack_idx != -1) {
        RAISE_TR_ERROR("variable '%s' redefenition", var.name);
        return -1;
    }
    var_t prev_var = {};
    stack_get_elem(var_stack, &prev_var, var_stack->size - 1);
    var.loc_addr = prev_var.loc_addr + 1;

    stack_push(var_stack, &var);
    STACK_DUMP(var_stack, stdout, var_t_fprintf);
    return var.loc_addr;
}

void translate_func_args_init(size_t *argc, ast_tree_elem_t *node, stack_t *var_stack) {
    assert(node);
    assert(var_stack);
    assert(node->right);

    var_t var_info = {};
    ast_tree_elem_t *var_init_node = node;

    (*argc)++;

    var_init_node = node->right; // var_init

    if (node->left) {
        translate_func_args_init(argc, node->left, var_stack);
    }

    var_info.type    = var_init_node->left->data.type;
    var_info.name_id = var_init_node->right->data.value.ival;
    var_info.deep    = cur_scope_deep;
    var_info.name    = var_init_node->right->data.value.sval;

    var_info.loc_addr = add_var_into_frame(var_info, var_stack);

    fprintf(asm_code_ptr,
                        "pop [AX+%d]; // '%s' init\n"
                        "push BX;\n"
                        "push 1;\n"
                        "pop BX; stack_ptr++\n",
                        var_info.loc_addr, var_info.name);

}

void var_stack_restore_old_frame(stack_t *var_stack) {
    assert(var_stack);

    var_t last_elem = {};
    stack_get_elem(var_stack, &last_elem, var_stack->size - 1);
    for (int i = (int) var_stack->size; i > cur_frame_ptr; i--) {
        stack_pop(var_stack);
    }
}

void translate_function_init(ast_tree_elem_t *node, stack_t *var_stack) {
    assert(node);
    assert(var_stack);
    assert(node->data.type == NODE_FUNC_INIT);

    var_t var_info = {};
    size_t argc = 0;
    func_info_t func_info;

    func_info.return_type_num = node->left->data.value.ival;
    func_info.name_id = node->right->data.value.ival;
    func_info.name = node->right->data.value.sval;

    add_function_to_name_table(func_info);

    fprintf(asm_code_ptr,"\n;#=========Function========#\n"
                         "%s:\n"
                         ";#=======Input=Action======#\n"
                         "push AX\n"
                         "push BX\n"
                         "pop AX\n"
                         ";#=======End=Action========#\n"
                         "\n;#========Init=Local=======#\n",
                         func_info.name);

    node = node->right; // func_id

    translate_func_args_init(&argc, node->left, var_stack); // write_args_initialization
    fprintf(asm_code_ptr, ";#========End=Init=========#\n");
    fprintf(asm_code_ptr, "\n;#========Func=Body========#\n");
    fprintf(asm_code_ptr, ";#========End=Body=========#\n");

    translate_node_to_asm_code(node->right, var_stack); //func_body;

    fprintf(asm_code_ptr,   "\n;#=======Leave=Action======#\n"
                        "push AX;\n"
                        "pop BX; stack_pointer = frame_pointer\n"
                        "ret;"
                        ";#=======End=Function======#\n");

    var_stack_restore_old_frame(var_stack); // call stack loc vars clearing + restore old_frame
}

void translate_node_to_asm_code(ast_tree_elem_t *node, stack_t *var_stack) {
    assert(node);
    assert(var_stack);

    static ast_tree_elem_t *root = node;



    if (node->data.type == NODE_SEMICOLON) {
        if (node->left) {
            translate_node_to_asm_code(node->left, var_stack);
        }
        if (node->right) {
            translate_node_to_asm_code(node->right, var_stack);
        }
        return;
    }

    if (node->data.type == NODE_SCOPE) {
        cur_scope_deep++;
        translate_node_to_asm_code(node->left, var_stack);
        cur_scope_deep--;
        var_stack_remove_local_variables(var_stack);
        return;
    }


    switch (node->data.type) {
        case NODE_FUNC_INIT: translate_function_init(node, var_stack); break;

    }

}








#undef RAISE_TR_ERROR