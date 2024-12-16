#ifndef DIFF_DSL_H
#define DIFF_DSL_H

#include "AST_proc.h"
#include "AST_io.h"

#define _VAR(name)              ast_tree_create_node(NULL, NULL,  {NODE_VAR,  {0     , 0ll, 0.0, name}})
#define _NUM(val)               ast_tree_create_node(NULL, NULL,  {NODE_NUM,  {0     , val, 0.0, NULL}})
#define _FUNC(n2, name)         ast_tree_create_node(NULL, n2,    {NODE_FUNC, {0     , 0ll, 0.0, name}})
#define _OP(num, left, right)   ast_tree_create_node(left, right, {NODE_OP,   {num, 0ll, 0.0, NULL}})

#endif // DIFF_DSL_H