#ifndef DIFF_DSL_H
#define DIFF_DSL_H

#include "AST_proc.h"
#include "AST_io.h"

#define _VAR(name)                   ast_tree_create_node(NULL, NULL,        {NODE_VAR,  {0     , 0ll, 0.0, name}})
#define _NUM(val)                    ast_tree_create_node(NULL, NULL,        {NODE_NUM,  {0     , 0ll, val, NULL}})
#define _FUNC(n2, name)              ast_tree_create_node(NULL, n2,          {NODE_FUNC, {0     , 0ll, 0.0, name}})
#define _OP(num, left, right)        ast_tree_create_node(left, right,       {NODE_OP,   {num, 0ll, 0.0, NULL}})
#define _ASSIGN(var, val)            ast_tree_create_node(var, val,          {NODE_ASSIGN,   {0, 0ll, 0.0, NULL}})
#define _INIT(type, var)             ast_tree_create_node(type, var,         {NODE_INIT,     {0, 0ll, 0.0, NULL}})
#define _TYPE(num)                   ast_tree_create_node(NULL, NULL,        {NODE_TYPE,     {num, 0ll, 0.0, NULL}})
#define _FUNC_ID(name)               ast_tree_create_node(NULL, NULL,        {NODE_FUNC_ID,     {0, 0ll, 0.0, name}})
#define _FUNC_BODY(return_val, body) ast_tree_create_node(return_val, body,  {NODE_FUNC_BODY,   {0, 0ll, 0.0, NULL}})

// Можно сделать multiple type через divider: (например const long double)

#endif // DIFF_DSL_H