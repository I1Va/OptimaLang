#include <stdlib.h>
#include <assert.h>

#include "general.h"
#include "AST_proc.h"
#include "AST_io.h"
#include "stack_funcs.h"

enum token_t {
    T_EOF = -1,
    T_EMPTY = 0,

    T_NUM = 1,

    T_ADD = 2,
    T_MUL = 3,
    T_SUB = 4,
    T_DIV = 5,
    T_POW = 6,

    T_O_BRACE = 7,
    T_C_BRACE = 8,
    T_O_FIG_BRACE = 9,
    T_C_FIG_BRACE = 10,
    T_EOL = 11, // '\n'
    T_SPACE = 12,
    T_ID = 13,

    T_IF = 14, // key_words
    T_WHILE = 15,

    T_DIVIDER = 16,

    T_MORE = 17,
    T_LESS = 18,
    T_MORE_EQ = 19,
    T_LESS_EQ = 20,
    T_EQ = 21,

    T_INT = 22,
    T_FLOAT = 23,

    T_ASSIGN = 24,
    T_COMMA = 25,
    T_RETURN = 26,
    T_ELSE = 27,

    T_BREAK = 28,
    T_CONTINUE = 29,

};

struct func_info_t {
    size_t init_addr;
    int return_type_num;

    size_t argc;
    int   *arg_type_nums;
    int   *arg_name_ids;

    char *name;
};

struct tr_global_t {
    func_info_t *func_table;
    stack_t var_stack;
};

struct tr_local_t {
    size_t deep;
};

struct asm_info_t {

};

enum act_nums {
    OLD_STACK_FRAME_RESTORE = 0,
    INIT_LOC_VAR = 1,
};

struct call_t {
    act_nums act_type;
    int frame_pointer;
    int scope_deep;

    int var_type;
    int var_name_id;
    int var_scope_deep;
    int var_loc_ram;

};

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


void translate_ast_to_asm_code(ast_tree_elem_t *node, stack_t *call_stack);





void translate_func_args_init(size_t *argc, ast_tree_elem_t *node, stack_t *call_stack);

void translate_ast_to_asm_code(const char path[], ast_tree_elem_t *root, stack_t *call_stack);
void translate_function_init(ast_tree_elem_t *node, stack_t *call_stack);
void translate_node_to_asm_code(ast_tree_elem_t *node, stack_t *call_stack);

void asm_write_op(FILE *stream, int op_num);
void asm_write_hlt(FILE *stream);
void asm_write_out(FILE *stream);
void asm_write_push_lval(FILE *stream, long long lval);
void asm_pop_mem_addr_plus_reg(FILE *stream, int addr, const char reg[], int indent = 0);
void asm_write_unknown_object(FILE *stream, int num);
void asm_write_function_label(FILE *stream, const char label[]);