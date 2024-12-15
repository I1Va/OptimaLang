#ifndef DIFF_DSL_H
#define DIFF_DSL_H

#define _DIV(n1, n2)    bin_tree_create_node(n1, n2,     {NODE_OP,   {OP_DIV, 0ll, 0.0, NULL}})
#define _ADD(n1, n2)    bin_tree_create_node(n1, n2,     {NODE_OP,   {OP_ADD, 0ll, 0.0, NULL}})
#define _SUB(n1, n2)    bin_tree_create_node(n1, n2,     {NODE_OP,   {OP_SUB, 0ll, 0.0, NULL}})
#define _MUL(n1, n2)    bin_tree_create_node(n1, n2,     {NODE_OP,   {OP_MUL, 0ll, 0.0, NULL}})
#define _VAR(name)      bin_tree_create_node(NULL, NULL, {NODE_VAR,  {0     , 0ll, 0.0, name}})
#define _NUM(val)       bin_tree_create_node(NULL, NULL, {NODE_NUM,  {0     , val, 0.0, NULL}})
#define _FUNC(n2, name) bin_tree_create_node(NULL, n2,   {NODE_FUNC, {0     , 0ll, 0.0, name}})

#endif // DIFF_DSL_H