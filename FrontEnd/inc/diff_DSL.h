#ifndef DIFF_DSL_H
#define DIFF_DSL_H

#include "AST_proc.h"
#include "AST_io.h"

#define _VAR(name, name_table_idx)     ast_tree_create_node(NULL, NULL,             {NODE_VAR,         {name_idx, 0ll, 0.0, name}})
#define _NUM(val)                      ast_tree_create_node(NULL, NULL,             {NODE_NUM,         {0     ,   0ll, val, NULL}})
#define _OP(num, left, right)          ast_tree_create_node(left, right,            {NODE_OP,          {num, 0ll, 0.0, NULL}})
#define _ASSIGN(var, val)              ast_tree_create_node(var, val,               {NODE_ASSIGN,      {0, 0ll, 0.0, NULL}})
#define _VAR_INIT(type, var)           ast_tree_create_node(type, var,              {NODE_VAR_INIT,    {0, 0ll, 0.0, NULL}})
#define _FUNC_INIT(type, var)          ast_tree_create_node(type, var,              {NODE_FUNC_INIT,   {0, 0ll, 0.0, NULL}})
#define _TYPE(num)                     ast_tree_create_node(NULL, NULL,             {NODE_TYPE,        {num, 0ll, 0.0, NULL}})
#define _FUNC_ID(name, name_id)        ast_tree_create_node(NULL, NULL,             {NODE_FUNC_ID,     {name_id, 0ll, 0.0, name}})
#define _FUNC_BODY(return_val, body)   ast_tree_create_node(return_val, body,       {NODE_FUNC_BODY,   {0, 0ll, 0.0, NULL}})
#define _GLOBAL(gl_states)             ast_tree_create_node(gl_states, NULL,        {NODE_GLOBAL,      {0, 0ll, 0.0, NULL}})
#define _CALL(func_id, args)           ast_tree_create_node(func_id, args,          {NODE_CALL,        {0, 0ll, 0.0, NULL}})
#define _ELSE(if_body, else_body)      ast_tree_create_node(if_body, else_body,     {NODE_ELSE,        {0, 0ll, 0.0, NULL}})
#define _SCOPE(statement_list)         ast_tree_create_node(statement_list, NULL,   {NODE_SCOPE,       {0, 0ll, 0.0, NULL}})
#define _RETURN(return_node)           ast_tree_create_node(return_node, NULL,      {NODE_RETURN,      {0, 0ll, 0.0, NULL}})
#define _BREAK()                       ast_tree_create_node(NULL, NULL,             {NODE_BREAK,       {0, 0ll, 0.0, NULL}})
#define _CONTINUE()                    ast_tree_create_node(NULL, NULL,             {NODE_CONTINUE,    {0, 0ll, 0.0, NULL}})
#define _WHILE(expr, body)             ast_tree_create_node(expr, body,             {NODE_WHILE,       {0, 0ll, 0.0, NULL}})
#define _SEMICOLON(left, right)        ast_tree_create_node(left, right,            {NODE_SEMICOLON,   {0, 0ll, 0.0, NULL}})
#define _IF(cond, else_node)           ast_tree_create_node(cond, else_node,        {NODE_IF,          {0, 0ll, 0.0, NULL}})
#define _COMMA(copy_node, args_node)   ast_tree_create_node(copy_node, args_node,   {NODE_COMMA,       {0, 0ll, 0.0, NULL}})
// Можно сделать multiple type через divider: (например const long double)

#endif // DIFF_DSL_H