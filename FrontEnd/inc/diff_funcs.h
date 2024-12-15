#ifndef DIFF_FUNCS_H
#define DIFF_FUNCS_H

#include "diff_tree.h"
#include <string.h>
#include "string_funcs.h"

const char VALID_OPERATIONS[] = "+/*-";
const size_t VALID_OPERATIONS_CNT = strlen(VALID_OPERATIONS);

const size_t CHUNK_SIZE = 2048;
const size_t MAX_NODE_WRAP_SZ = 64;

enum node_types {
    NODE_EMPTY = 0,
    NODE_VAR = 1,
    NODE_NUM = 2,
    NODE_OP = 3,
    NODE_FUNC = 4,
};

enum opers {
    OP_ADD = 0,
    OP_DIV = 1,
    OP_MUL = 2,
    OP_SUB = 3,
};

// void get_constantode_type(enum node_types *type, long double *value, char *name);
void get_node_string(char *bufer, bin_tree_elem_t *node);
size_t seg_char_cnt(char *left, char *right, char c);
void diff_infix_print(FILE *stream, bin_tree_elem_t *node);
void fprintf_seg(FILE *stream, char *left, char *right);
// void node_dump(FILE *log_file, bin_tree_elem_t *node);

#endif // DIFF_FUNCS_H