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

// CONSTANTS
const var_t POISON_VAR = {-1, -1, -1, NULL};
const char FRAME_PTR_REG_PLUS[] = "rbp+";
const reserved_func_info_t reserved_func_name_table[] =
{
    {"print",          AST_VOID, 1, translate_reserved_print_call},
    {"input",          AST_INT, 0, translate_reserved_input_call},
    {"sqrt",           AST_FLOAT, 1, translate_reserved_sqrt_call},
    {"print_string",   AST_VOID, 1, translate_reserved_print_string_call}
};
const size_t reserved_func_name_table_sz = sizeof(reserved_func_name_table) / sizeof(reserved_func_info_t);

// GLOBAL VARIABLES
FILE *asm_code_ptr = NULL;
int cur_scope_deep = 0;
int cur_frame_ptr  = 0;
int while_counter  = 0;
int if_counter     = 0;
bool func_init     = false;
bool void_func     = false;

func_info_t func_name_table[MAX_FUNC_TABLE_SZ] = {};
size_t func_table_sz = 0;

stack_t cond_stack         = {};
stack_t var_stack          = {};
stack_t global_var_stack   = {};
stack_t str_lit_lens_stack = {};

void translate_reserved_input_call(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_CALL);

    char *func_name = node->left->data.value.sval;
    if (strcmp(func_name, "input") != 0) {
        RAISE_TR_ERROR("reserve call error: expected 'input', got '%s'", func_name);
        return;
    }

    fprintf(asm_code_ptr,
                          "in; call input\n"
                        );
}

void translate_reserved_sqrt_call(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_CALL);

    char *func_name = node->left->data.value.sval;
    if (strcmp(func_name, "sqrt") != 0) {
        RAISE_TR_ERROR("reserve call error: expected 'sqrt', got '%s'", func_name);
        return;
    }

    fprintf(asm_code_ptr,
                          "sqrt; call sqrt\n"
                        );
}

void translate_reserved_print_call(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_CALL);

    char *func_name = node->left->data.value.sval;
    if (strcmp(func_name, "print") != 0) {
        RAISE_TR_ERROR("reserve call error: expected 'print', got '%s'", func_name);
        return;
    }

    fprintf(asm_code_ptr, ";call print\n"
                          "    out;\n"
                          "    push 10;\n"
                          "    outc;\n"
                        );
}

void assembler_make_bin_code(const char asm_code_path[], const char bin_code_path[]) {
    char bufer[MEDIUM_BUFER_SZ] = {};

    snprintf(bufer, MEDIUM_BUFER_SZ, "cd ./assembler && make launch -f Makefile LAUNCH_FLAGS=\"-i=./.%s -o=./.%s\"", asm_code_path, bin_code_path);
    system(bufer);
}

void translate_reserved_print_string_call(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_CALL);

    int lit_len = 0;
    char *func_name = node->left->data.value.sval;
    if (strcmp(func_name, "print_string") != 0) {
        RAISE_TR_ERROR("reserve call error: expected 'print', got '%s'", func_name);
        return;
    }
    stack_pop(&str_lit_lens_stack, &lit_len);
    fprintf(asm_code_ptr, "; print_string call\n");
    for (size_t i = 0; i < (size_t) lit_len; i++) {
        fprintf(asm_code_ptr, "    outc;\n");
    }
    fprintf(asm_code_ptr,
                          "    push 10;\n"
                          "    outc;\n"
                        );
}

void init_stacks(FILE *log_file_ptr) {
    STACK_INIT(&str_lit_lens_stack, 0, sizeof(int), log_file_ptr, NULL);
    STACK_INIT(&cond_stack, 0, sizeof(int), log_file_ptr, NULL);
    STACK_INIT(&var_stack, 0, sizeof(var_t), log_file_ptr, NULL);
    STACK_INIT(&global_var_stack, 0, sizeof(ast_tree_elem_t *), log_file_ptr, NULL);
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
    // fprintf(asm_code_ptr, "jmp __MAIN_LAUNCH:\n\n");
    translate_node_to_asm_code(tree->root);

    // __MAIN_LAUNCH:
    // fprintf(asm_code_ptr,
    //                  "\n\n__MAIN_LAUNCH:\n"
    //                  ";#=====Global=Vars=Init====#\n");

    // translate_global_vars_init();

    fprintf(asm_code_ptr,
                        "call main:\n"
                        "hlt;\n");
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
        if (last_elem.global) {
            break; // don't remove global variables
        }
        stack_pop(&var_stack);
        stack_get_elem(&var_stack, &last_elem, var_stack.size - 1);
    }
}

var_t get_var_from_frame(int name_id) {
    var_t var_info = {};

    if (var_stack.size == 0) {
        return POISON_VAR;
    }
    for (int i = (int) var_stack.size - 1; i >= cur_frame_ptr; i--) {
        stack_get_elem(&var_stack, &var_info, (size_t) i);

        if (var_info.name_id == name_id) {
            return var_info;
        }
    }
    return POISON_VAR;
}

void var_t_fprintf(FILE *stream, void *elem_ptr) {

    var_t var = *(var_t *) elem_ptr;
    fprintf(stream, "LOC VAR INIT(loc_ram[%d], type=%d, name='%s', id=%d, deep=%d)",
        var.loc_addr, var.type, var.name, var.name_id, var.deep);
}

bool var_t_equal(const var_t v1, const var_t v2) {
    if (v1.name == NULL && v2.name != NULL) {
        return false;
    }
    if (v1.name != NULL && v2.name == NULL) {
        return false;
    }

    bool name_eq = (v1.name == NULL) && (v2.name == NULL);
    if (v1.name != NULL && v2.name != NULL) {
        name_eq = name_eq || strcmp(v1.name, v2.name);
    }

    return v1.deep == v2.deep               &&
           v1.loc_addr == v2.loc_addr       &&
           v1.name_id  == v2.deep           &&
           name_eq                          &&
           v1.type == v2.type;
}

int add_var_into_frame(var_t var) {
    var_t found_var = get_var_from_frame(var.name_id);
    if (!var_t_equal(found_var, POISON_VAR)) {
        dump_global_info(stderr);
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
                        "add;\n"
                        "pop rsp; stack_ptr++\n",
                        var_info.loc_addr, var_info.name);

}

void var_stack_restore_old_frame() {
    var_t last_elem = {};
    stack_get_elem(&var_stack, &last_elem, var_stack.size - 1);
    for (int i = (int) var_stack.size; i >= cur_frame_ptr; i--) {
        if (last_elem.global) {
            break; // don't remove global variables
        }
        stack_pop(&var_stack);
    }
}

void translate_function_init(ast_tree_elem_t *node) {

    assert(node);
    assert(node->data.type == NODE_FUNC_INIT);

    size_t argc = 0;
    func_info_t func_info = {};

    func_info.return_type_num = node->left->data.value.ival;
    func_info.name = node->right->data.value.sval;

    fprintf(asm_code_ptr,

                         "\n;#=========Function========#\n"
                         "jmp %s_end:;\n"
                         "%s:\n"
                         ";#=======Input=Action======#\n"
                         "push rbp;\n"
                         "pop rbx;\n" // save of prev rpb into register
                         "push rsp;\n"
                         "pop rbp;\n"
                         ";#=======End=Action========#\n"
                         "\n;#=========Init=Args=======#\n",
                         func_info.name, func_info.name);

    node = node->right; // func_id
    CHECK_NODE_TYPE(node, NODE_FUNC_ID)

    if (node->left) {
        translate_func_args_init(&argc, node->left); // write_args_initialization
    }
    fprintf(asm_code_ptr, "push rbx;\n"); // save of prev rpb into stack

    func_info.argc = argc;
    add_function_to_name_table(func_info);

    fprintf(asm_code_ptr, ";#========End=Init=========#\n");

    fprintf(asm_code_ptr, "\n;#========Func=Body========#\n");
    translate_node_to_asm_code(node->right); //func_body;
    fprintf(asm_code_ptr, ";#========End=Body=========#\n");

    size_t return_num = count_node_type_in_subtreeas(node->right, NODE_RETURN);
    void_func = func_info.return_type_num == AST_VOID;

    if (return_num == 0 && !void_func) {
        RAISE_TR_ERROR("non void function '%s' hasn't <return>", func_info.name);
        return;
    }

    fprintf(asm_code_ptr,   "\n;#=======Leave=Action======#\n"
                        "push rbp;\n"
                        "pop rsp; stack_pointer = frame_pointer\n"
                        "pop  rbp;\n"
                        "ret;\n"
                        "%s_end:;\n"
                        ";#=======End=Function======#\n",
                        func_info.name);

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

void translate_string_literal(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_STR_LIT);

    stack_push(&str_lit_lens_stack, &node->data.value.ival);
    for (int i = node->data.value.ival - 1; i >= 0; i--) {
        fprintf(asm_code_ptr, "push %d;\n", node->data.value.sval[i]);
    }
}

void translate_op(ast_tree_elem_t *node) {
    assert(node);

    if (node->data.type == NODE_NUM) {
        translate_num(node);
        return;
    } else if (node->data.type == NODE_VAR) {
        translate_var(node);
        return;
    } else if (node->data.type == NODE_STR_LIT) {
        RAISE_TR_ERROR("translate_op doesn't support node_type: {%d}", node->data.type);
        return;
    } else if (node->data.type == NODE_CALL) {
        translate_func_call(node);
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
        case AST_LESS: fprintf(asm_code_ptr, "less;\n"); break;
        case AST_LESS_EQ: fprintf(asm_code_ptr, "lesseq;\n"); break;
        case AST_MORE: fprintf(asm_code_ptr, "more;\n"); break;
        case AST_MORE_EQ: fprintf(asm_code_ptr, "moreeq;\n"); break;
        case AST_EQ: fprintf(asm_code_ptr, "eq;\n"); break;
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
        case NODE_FUNC_INIT: func_init = true; translate_function_init(node); func_init = false;
            break;
        case NODE_IF: translate_if(node);
            break;
        case NODE_SEMICOLON: translate_semicolon(node);
            break;
        case NODE_COMMA: RAISE_TR_ERROR(
            "<NODE_COMMA> should be processed in"
            "<translate_func_call>/<translate_func_args_init>")
            break;
        case NODE_STR_LIT: translate_string_literal(node);
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
    ast_tree_elem_t *node_cond = node->left;
    node = node->right; // else_node

    fprintf(asm_code_ptr,";#=============If==========#\n"
                         "jmp if_check_%d:\n"
                         "if_start_%d:\n", if_counter, if_counter);

    stack_push(&cond_stack, &if_counter);
    stack_push(&cond_stack, &if_counter);
    if_counter++;

    // if body
    write_asm_tittle(asm_code_ptr, "If_body");
    translate_node_to_asm_code(node->left);
    stack_pop(&cond_stack, &save_counter);
    write_asm_tittle(asm_code_ptr, "End_body");
    fprintf(asm_code_ptr, "jmp if_end_%d:\n", save_counter);

    // else body
    fprintf(asm_code_ptr, "else_start_%d:\n", save_counter);
    write_asm_tittle(asm_code_ptr, "Else_body");
    if (node->right) {
        translate_node_to_asm_code(node->right);
        stack_pop(&cond_stack, &save_counter);
    }
    write_asm_tittle(asm_code_ptr, "End_body");
    fprintf(asm_code_ptr, "jmp if_end_%d:\n", save_counter);

    fprintf(asm_code_ptr, "if_check_%d:\n\n", save_counter);


    //Condition
    translate_if_condition(node_cond, save_counter);

    fprintf(asm_code_ptr, "if_end_%d:\n"
                          ";#=========End=IF=========#\n",
                          save_counter);
}

void translate_if_condition(ast_tree_elem_t *node, int curr_counter) {

    assert(node);

    translate_node_to_asm_code(node); // stack push cond

    fprintf(asm_code_ptr,   ";#========Condition========#\n"
                            "push 0\n"
                            "je else_start_%d:\n"
                            "jmp if_start_%d:\n"
                            ";#======End=Condition=======#\n",
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
        RAISE_TR_ERROR("'%s' call error: expected %lu arguments, got '%lu'", call_func_info.name, call_func_info.argc, argc);
        return;
    }

    fprintf(asm_code_ptr,
                          "call %s:\n"
                          "push rax; push return value\n",
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

    char *var_name = node->left->data.value.sval;
    int var_name_id = node->left->data.value.ival;
    bool global_state = false;

    translate_node_to_asm_code(node->right); // push right part of assign

    var_t found_var = get_var_from_frame(var_name_id);
    global_state = found_var.global;
    if (var_t_equal(found_var, POISON_VAR)) {
        RAISE_TR_ERROR("var '%s' not initialized", var_name);
        return;
    }

    const char *REG_PLUS = FRAME_PTR_REG_PLUS;
    if (global_state) {REG_PLUS = "";}

    fprintf(asm_code_ptr,
                         "pop [%s%d]; // '%s' assinment\n",
                        REG_PLUS, found_var.loc_addr, found_var.name);
}

void translate_var_init(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_VAR_INIT);

    var_t var_info = {};
    bool with_assignment = false;
    ast_tree_elem_t *name_node = node->right;
    const char *REG_PLUS = FRAME_PTR_REG_PLUS;
    var_info.type    = node->left->data.type;
    var_info.deep    = cur_scope_deep;

    var_info.name_id = name_node->left->data.value.ival;
    var_info.name    = name_node->left->data.value.sval;
    var_info.global  = (var_info.deep == 0 && !func_init);
    var_info.loc_addr = add_var_into_frame(var_info);
    if (name_node->data.type == NODE_ASSIGN) {
        with_assignment = true;
        translate_node_to_asm_code(name_node->right); // push right part of assignment
    }

    fprintf(asm_code_ptr,
                        "; // '%s' init, loc_addr: %d\n"
                        "push rsp;\n"
                        "push 1;\n"
                        "add;\n"
                        "pop rsp; stack_ptr++\n",
                        var_info.name, var_info.loc_addr);

    if (var_info.global) {REG_PLUS = "";}

    if (with_assignment) {
        fprintf(asm_code_ptr,
                            ";#====Init_Var=Assinment===#\n"
                            "pop [%s%d]; // '%s' assinment\n"
                            ";#======End=Assinment======#\n",
                            REG_PLUS, var_info.loc_addr, var_info.name);
    }
}

void translate_var(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_VAR);

    char *var_name = node->data.value.sval;
    int var_name_id = node->data.value.ival;

    var_t found_var = get_var_from_frame(var_name_id);
    bool global_state = found_var.global;

    if (var_t_equal(found_var, POISON_VAR)) {
        RAISE_TR_ERROR("var '%s' not initialized", var_name);
        return;
    }

    const char *REG_PLUS = FRAME_PTR_REG_PLUS;
    if (global_state) {REG_PLUS = "";}

    fprintf(asm_code_ptr,
                         "push [%s%d]; // access to '%s'\n",
                         REG_PLUS, found_var.loc_addr, found_var.name);
}

void translate_return(ast_tree_elem_t *node) {
    assert(node);
    CHECK_NODE_TYPE(node, NODE_RETURN);

    fprintf(asm_code_ptr, ";#========Var=Return=======#\n");

    if (!void_func) {
        if (!node->left) {
            RAISE_TR_ERROR("non void function hasn't return value");
            return;
        }
        translate_node_to_asm_code(node->left);
        fprintf(asm_code_ptr, "pop rax\n"); // writes return value into rax register
    } else {
        if (node->left) {
            RAISE_TR_ERROR("void function has return value");
            return;
        }
    }
    fprintf(asm_code_ptr,
                        "push rbp;\n"
                        "pop rsp; stack_pointer = frame_pointer\n"
                        "pop  rbp;\n"
                        "ret;\n"
                        ";#========End=Return=======#\n"
                        );
}

void fprintf_asm_border(FILE* stream, const char bord_char, const size_t bord_sz, bool new_line) {
    assert(stream);
    fputc('#', stream);
    for (size_t i = 0; i < bord_sz - 1; i++) {
        fputc(bord_char, stream);
    }
    fputc('#', stream);
    if (new_line) {
        fputc('\n', stream);
    }
}

void write_asm_tittle(FILE *stream, const char tittle[], const size_t bord_sz) {
    assert(tittle != NULL);

    size_t tittle_sz = strlen(tittle);
    if (bord_sz < tittle_sz + 1) {
        return;
    }
    size_t len = bord_sz - tittle_sz - 1;
    fputc(';', stream);
    fprintf_asm_border(stream, '=', len / 2, false);
    fprintf(stream, "%s", tittle);
    fprintf_asm_border(stream, '=', (len + 1) / 2, true);
}

#undef RAISE_TR_ERROR