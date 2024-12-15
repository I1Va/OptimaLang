#ifndef DIFF_TREE_H
#define DIFF_TREE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ERROR_PROCESSING \\-------------------------------------------------------------------------------->

enum bin_tree_err_t {
    BT_ERR_OK                       = 0ull,
    BT_ERR_FILE_OPEN                = 1ull << 0,
    BT_ERR_ALLOC                    = 1ull << 1,
    BT_ERR_SYSTEM                   = 1ull << 2,
    BT_ERR_CYCLED                   = 1ull << 4,
};


const size_t ERR_DESCR_MAX_SIZE = 128;
const size_t MAX_NODES_CNT = 1ull << 17;

void bin_tree_err_get_descr(enum bin_tree_err_t err_code, char err_descr_str[]);
void bin_tree_err_add(enum bin_tree_err_t *dest, enum bin_tree_err_t add);

#ifdef _DEBUG
    #define DEBUG_BT_LIST_ERROR(err_code, str_, ...) {                                                      \
        char BT_list_err_desr_str[ERR_DESCR_MAX_SIZE];                                                    \
        bin_tree_err_get_descr(err_code, BT_list_err_desr_str); \
        printf("DESCR: '%s'\n", BT_list_err_desr_str);                                                      \
        fprintf_red(stderr, "{%s} [%s: %d]: err_descr: {%s}, message: {" str_ "}\n",              \
             __FILE_NAME__, __PRETTY_FUNCTION__, __LINE__, BT_list_err_desr_str, ##__VA_ARGS__);                    \
        fprintf(stderr, WHT); \
    }

    #define ON_DEBUG(...) __VA_ARGS__
#else
    #define DEBUG_BT_LIST_ERROR(err_code, str_, ...) ;
#endif // _DEBUG


// TREE_PROCESSING \\-------------------------------------------------------------------------------->

const size_t MAX_NODE_STRING_SZ = 128;
const size_t MAX_LOG_FILE_PATH_SZ = 128;
const size_t NODE_LABEL_MAX_SZ = 128;

struct multi_val_t {
    int          ival;
    long long    lval;
    long double  fval;
    char        *sval;
};

struct bin_tree_elem_value_t {
    int type;
    multi_val_t value;
};

struct subtree_info_t {
    size_t sz;
    size_t divop_cnt;
    size_t addop_cnt;
    size_t subop_cnt;
    size_t mulop_cnt;
    size_t height;
};

struct bin_tree_elem_t {
    bin_tree_elem_t *prev;
    bool is_node_left_son;

    bin_tree_elem_t *left;
    bin_tree_elem_t *right;

    bin_tree_elem_value_t data;

    void *tree;
    bool constant_state;

    subtree_info_t subtree_info;
};

struct bin_tree_t {
    bin_tree_elem_t *root;

    size_t n_nodes;

    FILE *log_file_ptr;
    char log_file_path[MAX_LOG_FILE_PATH_SZ];
};

bool bin_tree_ctor(bin_tree_t *tree, const char log_path[]);
bin_tree_elem_t *bin_tree_create_node(bin_tree_elem_t *left, bin_tree_elem_t *right, const bin_tree_elem_value_t data);
void bin_tree_print(bin_tree_elem_t *node, void (*outp_func)(char *dest, const size_t maxn_n, const bin_tree_elem_t *node));
void bin_tree_rec_nodes_cnt(bin_tree_elem_t *node, size_t *nodes_cnt);
void bin_tree_verify(const bin_tree_t tree, bin_tree_err_t *return_err);
bin_tree_elem_t *get_node_copy(bin_tree_elem_t *node);
bin_tree_elem_t *get_msubtree_copy(bin_tree_elem_t *root);
void sub_tree_dtor(bin_tree_elem_t *root);
void mark_subtree(bin_tree_elem_t *root, bin_tree_t *tree);
void bin_tree_dtor(bin_tree_t *tree);
size_t place_subtrees_sz(bin_tree_elem_t *root);

#endif // DIFF_TREE_H