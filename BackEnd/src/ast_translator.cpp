#include <cstring>
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
int while_counter = 0;
int if_counter = 0;

// int return_type_num;
//     size_t argc;
//     int name_id;
//     char *name;

void translate_reserved_print_call(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_CALL);

    char *func_name = node->left->data.value.sval;
    if (strcmp(func_name, "print") != 0) {
        RAISE_TR_ERROR("reserve call error: expected 'print', got '%s'", func_name);
        return;
    }

    fprintf(asm_code_ptr, ";#==========Call===========#\n"
                          "\nout;\n"
                          ";#=========End=Call========#\n"
                        );
}

reserved_func_info_t reserved_func_name_table[] =
{
    {"print", AST_VOID, 1, translate_reserved_print_call},
};

size_t reserved_func_name_table_sz = sizeof(reserved_func_name_table) / sizeof(reserved_func_info_t);

func_info_t func_name_table[MAX_FUNC_TABLE_SZ] = {};
size_t func_table_sz = 0;

stack_t cond_stack = {};
stack_t var_stack = {};





void init_stacks(FILE *log_file_ptr) {

    STACK_INIT(&cond_stack, 0, sizeof(int), log_file_ptr, NULL);
    STACK_INIT(&var_stack, 0, sizeof(var_t), log_file_ptr, NULL);
}

void translate_ast_to_asm_code(const char path[], ast_tree_t *tree) {

    assert(path);
    assert(tree);

    init_stacks(tree->log_file_ptr);

    asm_code_ptr = fopen(path, "w");
    if (!asm_code_ptr) {

        debug("failed to open: '%s'", path);
        return;
    }

    setbuf(asm_code_ptr, 0); // FIXME: remove after debugging

    fprintf(asm_code_ptr, "jmp __MAIN_LAUNCH:\n\n");
    translate_node_to_asm_code(tree->root);
    fprintf(asm_code_ptr,
                         "\n\n__MAIN_LAUNCH:\n"
                         "call main:\n"
                         "hlt;\n"
                         );
}

size_t count_node_type_in_subtreeas(ast_tree_elem_t *node, const enum node_types node_type) {

    assert(node);

    size_t count = (node->data.type == node_type);

    if (node->left) {

        count += count_node_type_in_subtreeas(node->left, node_type);
    }
    if (node->right) {

        count += count_node_type_in_subtreeas(node->right, node_type);
    }

    return count;
}

void var_stack_remove_local_variables() {

    var_t last_elem = {};
    stack_get_elem(&var_stack, &last_elem, var_stack.size - 1);

    while (last_elem.deep > cur_scope_deep && var_stack.size) {
        stack_pop(&var_stack);
        stack_get_elem(&var_stack, &last_elem, var_stack.size - 1);
    }
}

int get_var_from_frame(int name_id) {
    var_t var_info = {};

    for (int i = (int) var_stack.size; i >= cur_frame_ptr; i--) {

        stack_get_elem(&var_stack, &var_info, (size_t) i);
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

int add_var_into_frame(var_t var) {

    int stack_idx = get_var_from_frame(var.name_id);
    if (stack_idx != -1) {

        RAISE_TR_ERROR("variable '%s' redefenition", var.name);
        return -1;
    }
    var_t prev_var = {};
    if (var_stack.size) {
        stack_get_elem(&var_stack, &prev_var, var_stack.size - 1);
    } else {
        prev_var.loc_addr = -1;
    }

    var.loc_addr = prev_var.loc_addr + 1;

    stack_push(&var_stack, &var);
    return var.loc_addr;
}

void translate_func_args_init(size_t *argc, ast_tree_elem_t *node) {

    assert(node);
    assert(node->right);
    CHECK_NODE_TYPE(node, NODE_COMMA);

    var_t var_info = {};
    ast_tree_elem_t *var_init_node = node;

    (*argc)++;

    var_init_node = node->right; // var_init

    if (node->left) {

        translate_func_args_init(argc, node->left);
    }

    var_info.type    = var_init_node->left->data.type;
    var_info.name_id = var_init_node->right->data.value.ival;
    var_info.deep    = cur_scope_deep;
    var_info.name    = var_init_node->right->data.value.sval;

    var_info.loc_addr = add_var_into_frame(var_info);

    fprintf(asm_code_ptr,
                        "pop [rbp+%d]; // '%s' init\n"
                        "push rsp;\n"
                        "push 1;\n"
                        "pop rsp; stack_ptr++\n",
                        var_info.loc_addr, var_info.name);

}

void var_stack_restore_old_frame() {

    var_t last_elem = {};
    stack_get_elem(&var_stack, &last_elem, var_stack.size - 1);
    for (int i = (int) var_stack.size; i >= cur_frame_ptr; i--) {

        stack_pop(&var_stack);
    }
}

void translate_function_init(ast_tree_elem_t *node) {

    assert(node);
    assert(node->data.type == NODE_FUNC_INIT);

    size_t argc = 0;
    func_info_t func_info;

    func_info.return_type_num = node->left->data.value.ival;
    func_info.name = node->right->data.value.sval;

    add_function_to_name_table(func_info);

    fprintf(asm_code_ptr,"\n;#=========Function========#\n"
                         "%s:\n"
                         ";#=======Input=Action======#\n"
                         "push rbp\n"
                         "push rsp\n"
                         "pop rbp\n"
                         ";#=======End=Action========#\n"
                         "\n;#=========Init=Args=======#\n",
                         func_info.name);

    node = node->right; // func_id
    CHECK_NODE_TYPE(node, NODE_FUNC_ID)

    if (node->left) {

        translate_func_args_init(&argc, node->left); // write_args_initialization
    }

    fprintf(asm_code_ptr, ";#========End=Init=========#\n");

    fprintf(asm_code_ptr, "\n;#========Func=Body========#\n");
    translate_node_to_asm_code(node->right); //func_body;
    fprintf(asm_code_ptr, ";#========End=Body=========#\n");

    if (count_node_type_in_subtreeas(node->right, NODE_RETURN) != 1) {

        RAISE_TR_ERROR("function '%s' hasn't <return>", func_info.name);
        return;
    }

    fprintf(asm_code_ptr,   "\n;#=======Leave=Action======#\n"
                        "push rbp;\n"
                        "pop rsp; stack_pointer = frame_pointer\n"
                        "pop  rbp;\n"
                        "ret;\n"
                        ";#=======End=Function======#\n");

    var_stack_restore_old_frame(); // call stack loc vars clearing + restore old_frame
}

void translate_scope(ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_SCOPE);

    cur_scope_deep++;
    translate_node_to_asm_code(node->left);
    cur_scope_deep--;

    var_stack_remove_local_variables();
}

void translate_semicolon(ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_SEMICOLON);

    if (node->left) {

        translate_node_to_asm_code(node->left);
    }
    if (node->right) {

        translate_node_to_asm_code(node->right);
    }
}

void translate_num(ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_NUM);

    fprintf(asm_code_ptr, "push %Lg;\n", node->data.value.fval);
}

void translate_op(ast_tree_elem_t *node) {

    assert(node);

    if (!node->left && !node->right) {

        if (node->data.type == NODE_NUM) {

            translate_num(node);
        } else if (node->data.type == NODE_VAR) {

            translate_var(node);
        } else {
            RAISE_TR_ERROR("translate_op doesn't support node_type: {%d}", node->data.type);
            return;
        }
        return;
    }

    CHECK_NODE_TYPE(node, NODE_OP);


    translate_op(node->right);
    translate_op(node->left);

    switch (node->data.value.ival) {

        case AST_ADD: fprintf(asm_code_ptr, "add;\n"); break;
        case AST_MUL: fprintf(asm_code_ptr, "mult;\n"); break;
        case AST_SUB: fprintf(asm_code_ptr, "sub;\n"); break;
        case AST_DIV: fprintf(asm_code_ptr, "div;\n"); break;
    }
}

void translate_node_to_asm_code(ast_tree_elem_t *node) {

    assert(node);
    switch (node->data.type) {

        case NODE_EMPTY: RAISE_TR_ERROR("incorrect AST: <NODE_EMPTY>")
            break;
        case NODE_VAR: translate_var(node);
            break;
        case NODE_NUM: translate_num(node);
            break;
        case NODE_OP: translate_op(node);
            break;
        case NODE_TYPE: fprintf_red(stdout, "there is should be <NODE_TYPE> translation\n");
            break;
        case NODE_ASSIGN: translate_assign(node);
            break;
        case NODE_VAR_INIT: translate_var_init(node);
            break;
        case NODE_FUNC_ID: RAISE_TR_ERROR(
            "<NODE_FUNC_ID> should be processed in"
            "<translate_func_call/translate_function_init>")
            break;
        case NODE_CALL: translate_func_call(node);
            break;
        case NODE_ELSE: RAISE_TR_ERROR(
            "<NODE_ELSE> should be processed in"
            "<translate_if>")
            break;
        case NODE_SCOPE: translate_scope(node);
            break;
        case NODE_RETURN: translate_return(node);
            break;
        case NODE_BREAK: fprintf_red(stdout, "there is should be <NODE_BREAK> translation");
            break;
        case NODE_CONTINUE: fprintf_red(stdout, "there is should be <NODE_CONTINUE> translation");
            break;
        case NODE_WHILE: translate_while(node);
            break;
        case NODE_FUNC_INIT: translate_function_init(node);
            break;
        case NODE_IF: translate_if(node);
            break;
        case NODE_SEMICOLON: translate_semicolon(node);
            break;
        case NODE_COMMA: RAISE_TR_ERROR(
            "<NODE_COMMA> should be processed in"
            "<translate_func_call>/<translate_func_args_init>")
            break;
        default: RAISE_TR_ERROR("incorrect AST: <UNKNOWN_NODE(%d)>", node->data.type)
            break;
    }
}

void translate_while(ast_tree_elem_t *node) {

    int cur_node_type = NODE_WHILE;

    assert(node);

    CHECK_NODE_TYPE(node, cur_node_type);
    assert(node->left);
    assert(node->right);

    fprintf(asm_code_ptr,";#===========While=========#\n"
                         "jmp while_check_%d:\n"
                         "while_start_%d:\n\n", while_counter, while_counter);

    stack_push(&cond_stack, &while_counter);

    while_counter++;

    if (node->right) {

        translate_node_to_asm_code(node->right); //body
    }

    int save_counter  = 0;
    stack_pop(&cond_stack, &save_counter);     //pop index

    fprintf(asm_code_ptr, "\nwhile_check_%d:\n\n", save_counter);

    //Condition

    translate_while_condition(node->left, save_counter);
    fprintf(asm_code_ptr, "\nwhile_end_%d:\n"
                          ";#=======End=While========#\n", save_counter);
}

void translate_while_condition(ast_tree_elem_t *node, int curr_counter) {

    assert(node);

    translate_node_to_asm_code(node); // stackp push cond

    fprintf(asm_code_ptr,   ";#========Condition========#\n"
                            "\npush 0\n"
                            "jne while_start_%d:\n",
                            curr_counter);
}

void translate_if(ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_IF);
    int save_counter = 0;

    fprintf(asm_code_ptr,";#=============If==========#\n"
                         "jmp if_check_%d:\n"
                         "if_start_%d:\n", if_counter, if_counter);

    stack_push(&cond_stack, &if_counter);
    if_counter++;

    // if body
    translate_node_to_asm_code(node->left);


    stack_pop(&cond_stack, &save_counter);

    fprintf(asm_code_ptr, "jmp if_end_%d:\n\n", save_counter);

    // else body
    fprintf(asm_code_ptr, "else_start_%d:\n\n", save_counter);
    if (node->right) {


        translate_node_to_asm_code(node->left);

        stack_pop(&cond_stack, &save_counter);
    }
    fprintf(asm_code_ptr, "jmp if_end_%d:\n\n", save_counter);



    fprintf(asm_code_ptr, "if_check_%d:\n\n", save_counter);


    //Condition

    translate_if_condition(node->left, save_counter);

    fprintf(asm_code_ptr, "if_end_%d:\n"
                          ";#=========End=IF=========#\n",
                          save_counter);
}

void translate_if_condition(ast_tree_elem_t *node, int curr_counter) {

    assert(node);

    translate_node_to_asm_code(node); // stack push cond

    fprintf(asm_code_ptr,   ";#========Condition========#\n"
                            "\npush 0\n"
                            "jne if_start_%d:\n"
                            "je  else_start_%d:\n"
                            "#======End=Condition=======#\n",
                            curr_counter, curr_counter);
}

int get_func_idx_in_reserved_name_table(func_info_t func_info) {
    for (size_t i = 0; i < reserved_func_name_table_sz; i++) {
        if (strcmp(reserved_func_name_table[i].name, func_info.name) == 0) {
            return (int) i;
        }
    }
    return -1;
}

int get_func_idx_in_name_table(func_info_t func_info) {
    for (size_t i = 0; i < func_table_sz; i++) {
        if (strcmp(func_name_table[i].name, func_info.name) == 0) {
            return (int) i;
        }
    }

    return -1;
}

void add_function_to_name_table(func_info_t func_info) {
    if (get_func_idx_in_name_table(func_info) != -1 || get_func_idx_in_reserved_name_table(func_info) != -1) {
        RAISE_TR_ERROR("function '%s' redefenition", func_info.name);
        return;
    }
    func_name_table[func_table_sz++] = func_info;
}

void translate_func_call(ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_CALL);

    size_t argc = 0;

    if (node->right) {
        translate_func_call_args(&argc, node->right);
    }
    func_info_t func_info = {};
    func_info.name     = node->left->data.value.sval;

    int func_reserved_table_idx = get_func_idx_in_reserved_name_table(func_info);
    if (func_reserved_table_idx != -1) {
        reserved_func_info_t res_func = reserved_func_name_table[func_reserved_table_idx];
        if (argc != res_func.argc) {
            RAISE_TR_ERROR("%s call error: expected %lu arguments, got '%lu'", res_func.name, res_func.argc, argc);
            return;
        }
        res_func.translate_func(node);
        return;
    }

    int func_name_table_idx = get_func_idx_in_name_table(func_info);
    if (func_name_table_idx == -1) {
        RAISE_TR_ERROR("func '%s' not initialized", func_info.name);
        return;
    }
    func_info_t call_func_info = func_name_table[func_name_table_idx];
    if (call_func_info.argc != argc) {
        RAISE_TR_ERROR("%s call error: expected %lu arguments, got '%lu'", call_func_info.name, call_func_info.argc, argc);
        return;
    }

    fprintf(asm_code_ptr, ";#==========Call===========#\n"
                          "\ncall %s:\n"
                          ";#=========End=Call========#\n",
                          func_info.name);
}

void translate_func_call_args(size_t *argc, ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_COMMA);

    (*argc)++;

    if (node->left) {

        translate_func_args_init(argc, node->left);
    }

    translate_node_to_asm_code(node->right); // push call args in reversed order
}

void dump_global_info(FILE *stream) {

    fprintf_title(stream, "GLOBAL_INFO", '=', STR_F_BORDER_SZ);
    fprintf(stream,
                   "cur_scope_deep: %d\n"
                   "cur_frame: %d\n",
                   cur_scope_deep, cur_frame_ptr);

    fprintf(stream, "var_stack: \n");
    STACK_DUMP(&var_stack, stream, var_t_fprintf);
}

void translate_assign(ast_tree_elem_t *node) {

    assert(node);
    assert(node->right);
    assert(node->left);
    CHECK_NODE_TYPE(node, NODE_ASSIGN);

    var_t var_info = {};
    var_info.name = node->left->data.value.sval;
    var_info.name_id = node->left->data.value.ival;
    var_info.deep = cur_scope_deep;

    translate_node_to_asm_code(node->right); // push right part of assign

    var_info.loc_addr = get_var_from_frame(var_info.name_id);
    if (var_info.loc_addr == -1) {

        RAISE_TR_ERROR("var '%s' not initialized", var_info.name);
        return;
    }

    fprintf(asm_code_ptr,
                         "pop [rbp+%d]; // '%s' assinment\n",
                         var_info.loc_addr, var_info.name);
}

void translate_var_init(ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_VAR_INIT);

    fprintf(asm_code_ptr,   ";#========Var=Init=========#\n");

    var_t var_info = {};
    bool with_assignment = false;
    var_info.type    = node->left->data.type;
    var_info.deep    = cur_scope_deep;

    ast_tree_elem_t *name_node = node->right;

    if (name_node->data.type == NODE_ASSIGN) {

        with_assignment = true;
        var_info.name_id = name_node->left->data.value.ival;
        var_info.name    = name_node->left->data.value.sval;
        translate_node_to_asm_code(name_node->right); // push right part of assignment
    } else {
        var_info.name_id = name_node->data.value.ival;
        var_info.name = name_node->data.value.sval;
    }

    var_info.loc_addr = add_var_into_frame(var_info);

    fprintf(asm_code_ptr,
                        "; // '%s' init, loc_addr: %d\n"
                        "push rsp;\n"
                        "push 1;\n"
                        "add;\n"
                        "pop rsp; stack_ptr++\n",
                        var_info.name, var_info.loc_addr);

    fprintf(asm_code_ptr, ";#========End=Init=========#\n");
    if (with_assignment) {

        fprintf(asm_code_ptr,
                             ";#====Init_Var=Assinment===#\n"
                             "pop [rbp+%d]; // '%s' assinment\n"
                             ";#======End=Assinment======#\n",
                             var_info.loc_addr, var_info.name);
    }
}

void translate_var(ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_VAR);

    var_t var_info = {};
    var_info.name = node->data.value.sval;
    var_info.name_id = node->data.value.ival;
    var_info.deep = cur_scope_deep;

    var_info.loc_addr = get_var_from_frame(var_info.name_id);

    if (var_info.loc_addr == -1) {

        RAISE_TR_ERROR("var '%s' not initialized", var_info.name);
        return;
    }

    fprintf(asm_code_ptr,
                         "push [rbp+%d]; // acces to '%s'\n",
                         var_info.loc_addr, var_info.name);
}

void translate_return(ast_tree_elem_t *node) {

    assert(node);
    CHECK_NODE_TYPE(node, NODE_RETURN);

    fprintf(asm_code_ptr, ";#========Var=Return=======#\n");

    if (!node->left) {

        RAISE_TR_ERROR("<return> hasn't arg");
        return;
    }

    translate_node_to_asm_code(node->left);

    fprintf(asm_code_ptr, "pop rax\n"); // writes return arg into rax register

    fprintf(asm_code_ptr, ";#========End=Return=======#\n");
}

void fprintf_asm_title(FILE *stream, const char tittle[], const size_t bord_sz) {

    assert(tittle != NULL);

    size_t tittle_sz = strlen(tittle);
    if (bord_sz < tittle_sz) {

        return;
    }
    size_t len = bord_sz - tittle_sz;
    fprintf_border(stream, '=', len / 2, false);
    fprintf(stream, "%s", tittle);
    fprintf_border(stream, '=', (len + 1) / 2, true);
}

#undef RAISE_TR_ERROR