#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "general.h"
#include "diff_tree.h"
#include "diff_funcs.h"
#include "string_funcs.h"
#include "diff_DSL.h"

// void get_constantode_type(enum node_types *type, long double *value, char *name) {
//     if (sscanf(name, "%Lf", value)) {
//         *type = NODE_NUM;
//         return;
//     }

//     if (strlen(name) == 1) {
//         *type = NODE_OP;

//         for (size_t i = 0; i < VALID_OPERATIONS_CNT; i++) {
//             if (*name == VALID_OPERATIONS[i]) {
//                 *value = i;
//                 return;
//             }
//         }
//     }

//     *type = NODE_VAR;
//     *value = 0;
// }

void get_node_string(char *bufer, bin_tree_elem_t *node) {
    if (node == NULL) {
        snprintf(bufer, BUFSIZ, "NULL");
        return;
    }

    if (node->data.type == NODE_OP) {
        char res = '\0';

        switch (node->data.value.ival) {
            case OP_ADD: res = '+'; break;
            case OP_DIV: res = '/'; break;
            case OP_MUL: res = '*'; break;
            case OP_SUB: res = '-'; break;
            default: res = '?'; break;
        }
        snprintf(bufer, BUFSIZ, "%c", res);
    } else if (node->data.type == NODE_NUM) {
        snprintf(bufer, BUFSIZ, "%Ld", node->data.value.lval);
    } else if (node->data.type == NODE_VAR) {
        snprintf(bufer, BUFSIZ, "%s", node->data.value.sval);
    } else if (node->data.type == NODE_FUNC) {
        snprintf(bufer, BUFSIZ, "%s", node->data.value.sval);
    }

    else {
        snprintf(bufer, BUFSIZ, "?");
    }
}

size_t seg_char_cnt(char *left, char *right, char c) {
    size_t cnt = 0;

    while (left <= right) {
        cnt += (*left++ == c);
    }

    return cnt;
}

void diff_infix_print(FILE *stream, bin_tree_elem_t *node) {
    assert(node != NULL);

    if (node->left) {
        fprintf(stream, "(");
        diff_infix_print(stream, node->left);
        fprintf(stream, ")");
    }

    char bufer[MINI_BUFER_SZ] = {};
    get_node_string(bufer, node);
    fprintf(stream, "%s", bufer);

    if (node->right) {
        fprintf(stream, "(");
        diff_infix_print(stream, node->right);
        fprintf(stream, ")");
    }
}

void fprintf_seg(FILE *stream, char *left, char *right) {
    for (;left <= right; left++) {
        fputc(*left, stream);
    }
}

// void node_dump(FILE *log_file, bin_tree_elem_t *node) {
//     assert(log_file != NULL);
//     assert(node != NULL);

//     char bufer[BUFSIZ] = {};
//     size_t indent_sz = 4;
//     size_t indent_step = 4;

//     fprintf(log_file, "node[%p]\n{\n", node);

//     fprintf_str_block(log_file, indent_sz, dot_code_pars_block_sz, "left_");
//     get_node_string(bufer, node->left);
//     fprintf(log_file, " = ([%p]; '%s')\n", node->left, bufer);

//     fprintf_str_block(log_file, indent_sz, dot_code_pars_block_sz, "right_");
//     get_node_string(bufer, node->right);
//     fprintf(log_file, " = ([%p]; '%s')\n", node->right, bufer);

//     fprintf_str_block(log_file, indent_sz, dot_code_pars_block_sz, "prev_");
//     get_node_string(bufer, node->prev);
//     fprintf(log_file, " = ([%p]; '%s')\n", node->prev, bufer);

//     fprintf_str_block(log_file, indent_sz, dot_code_pars_block_sz, "is_left_son_");
//     fprintf(log_file, " = (%d)\n", node->is_node_left_son);

//     fprintf_str_block(log_file, indent_sz, dot_code_pars_block_sz, "data_");
//     get_node_string(bufer, node);
//     fprintf(log_file, " = ('%s')\n", bufer);

//     fprintf(log_file, "}\n");
// }