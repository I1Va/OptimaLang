#include <assert.h>
#include <cstddef>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <sys/types.h>

#include "AST_io.h"
#include "AST_proc.h"
#include "general.h"
#include "lang_global_space.h"
#include "lang_logger.h"
#include "string_funcs.h"
#include "lang_grammar.h"

#include "diff_DSL.h"

#define CATCH_PARSE_ERROR(data, grule)                     \
    if (data->parser_err.err_state) {                      \
        add_grule_to_parser_err(&data->parser_err, grule); \
        return NULL;                                       \
    }

#define STEP_OVER_TOKEN_WITH_CHECK(data, grule, token)                                       \
    if (data->lexem_list[data->lexem_list_idx].token_type != token) {                       \
        start_parser_err(&data->parser_err, data->lexem_list[data->lexem_list_idx], grule); \
        return NULL;                                                                        \
    }                                                                                       \
    data->lexem_list_idx++;                                                                 \

void start_parser_err(parser_err_t *parser_err, lexem_t lexem, enum grammar_rule_num grule) {
    assert(parser_err);
    assert(!parser_err->err_state);
    assert(parser_err->lex.token_type == T_EMPTY);
    assert(!parser_err->grule_list_size);

    parser_err->err_state = true;
    parser_err->lex = lexem;
    parser_err->grule_list[0] = grule;
    parser_err->grule_list_size++;
}

void add_grule_to_parser_err(parser_err_t *parser_err, enum grammar_rule_num grule) {
    assert(parser_err);
    assert(parser_err->err_state);

    parser_err->grule_list[parser_err->grule_list_size++] = grule;
}

void clear_parser_err(parser_err_t *parser_err) {
    assert(parser_err);
    assert(parser_err->err_state);

    parser_err->grule_list_size = 0;
    parser_err->grule_list[0] = EMPTY_GRULE;
    parser_err->err_state = false;
    parser_err->lex = {};
    parser_err->lex.token_type = T_EMPTY;
}

bool check_parser_err(FILE *stream, parsing_block_t *data) {
    assert(stream);
    assert(data);



    if (!data->parser_err.err_state) {
        return false;
    }

    fprintf_title(stream, "SYNTAX_ERROR", '-', STR_F_BORDER_SZ);

    fprintf(stream, "ERROR_LEXEM: ");
    lexem_dump(stream, data->name_table, data->parser_err.lex);
    printf("\n");

    fprintf(stream, "ERROR_PATH: \n");
    for (size_t i = 0; i < data->parser_err.grule_list_size; i++) {
        grule_dump(stream, data->parser_err.grule_list[i]);
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");

    fprintf_border(stream, '-', STR_F_BORDER_SZ, true);

    return true;
}

ast_tree_elem_t *get_code_block(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    ast_tree_elem_t *prev_node = NULL;
    ast_tree_elem_t *node = NULL;

    if (tl[*tp].token_type != T_O_FIG_BRACE) {
        start_parser_err(&data->parser_err, tl[*tp], GET_CODE_BLOCK);
        return NULL;
    }

    (*tp)++;


    node = get_statement(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_CODE_BLOCK);
        return NULL;
    }
    // debug_lex(tl[*tp], data);
    if (tl[*tp].token_type != T_DIVIDER) {
        start_parser_err(&data->parser_err, tl[*tp], GET_CODE_BLOCK);
        return NULL;
    }
    (*tp)++;

    while (tl[*tp].token_type != T_C_FIG_BRACE) {
        prev_node = node;
        node = get_statement(data);

        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_CODE_BLOCK);
            return NULL;
        }

        node = _OP(T_DIVIDER, prev_node, node);

        if (tl[*tp].token_type != T_DIVIDER) {
            start_parser_err(&data->parser_err, tl[*tp], GET_CODE_BLOCK);
            return NULL;
        }
        (*tp)++;

    }
    (*tp)++;

    if (tl[*tp].token_type != T_EOF) {
        start_parser_err(&data->parser_err, tl[*tp], GET_CODE_BLOCK);
        return NULL;
    }

    (*tp)++;

    return node;
}

void debug_lex(lexem_t lex, parsing_block_t *data) {
    assert(data);

    fprintf_title(stdout, "DEBUG_LEX", '-', STR_F_BORDER_SZ);
    lexem_dump(stdout, data->name_table, lex);
    printf("\n");
    fprintf_border(stdout, '-', STR_F_BORDER_SZ, true);
}

ast_tree_elem_t *get_expression(parsing_block_t *data) {
    assert(data);

    ast_tree_elem_t *node = get_logical_expression(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_EXPRESSION);
        return NULL;
    }

    return node;
}

ast_tree_elem_t *try_grule(parsing_block_t *data, ast_tree_elem_t *(*grule_func)(parsing_block_t *data)) {
    assert(data);
    assert(grule_func);

    size_t tp = data->lexem_list_idx;
    ast_tree_elem_t *node = NULL;

    node = grule_func(data);
    if (!data->parser_err.err_state) {
        return node;
    }
    clear_parser_err(&data->parser_err);
    data->lexem_list_idx = tp;
    return NULL;
}

ast_tree_elem_t *get_statement(parsing_block_t *data) {
    assert(data != NULL);

    size_t tp = data->lexem_list_idx;

    ast_tree_elem_t *val = NULL;

    val = try_grule(data, get_variable_initialization_with_assignment);
    if (val) { return val; }

    val = try_grule(data, get_variable_initialization);
    if (val) { return val; }

    val = get_assignment(data);
    if (!data->parser_err.err_state) {
        return val;
    }
    clear_parser_err(&data->parser_err);
    data->lexem_list_idx = tp;

    val = get_expression(data);
    if (!data->parser_err.err_state) {
        return val;
    }
    clear_parser_err(&data->parser_err);
    data->lexem_list_idx = tp;

    val = get_while(data);
    if (!data->parser_err.err_state) {
        return val;
    }
    clear_parser_err(&data->parser_err);
    data->lexem_list_idx = tp;


    val = get_selection_statement(data);

    if (!data->parser_err.err_state) {
        return val;
    }

    add_grule_to_parser_err(&data->parser_err, GET_STATEMENT);
    return val;
}

ast_tree_elem_t *get_selection_statement(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    ast_tree_elem_t *left_node = NULL;
    ast_tree_elem_t *copy_node = NULL;
    ast_tree_elem_t *right_node = NULL;

    if (tl[*tp].token_type != T_IF) {
        start_parser_err(&data->parser_err, tl[*tp], GET_SELECTION_STATEMENT);
        return NULL;
    }

    (*tp)++;


    if (tl[*tp].token_type != T_O_BRACE) {
        start_parser_err(&data->parser_err, tl[*tp], GET_SELECTION_STATEMENT);
        return NULL;
    }

    (*tp)++;

    left_node = get_expression(data);
    // check_parser_err(stdout, data);

    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_SELECTION_STATEMENT);
        return NULL;
    }
    if (tl[*tp].token_type != T_C_BRACE) {
        start_parser_err(&data->parser_err, tl[*tp], GET_SELECTION_STATEMENT);
        return NULL;
    }

    (*tp)++;

    if (tl[*tp].token_type != T_O_FIG_BRACE) {
        start_parser_err(&data->parser_err, tl[*tp], GET_SELECTION_STATEMENT);
        return NULL;
    }

    (*tp)++;

    bool cycled = false;



    while (tl[*tp].token_type != T_C_FIG_BRACE) {
        cycled = true;

        copy_node = right_node;
        right_node = get_statement(data);

        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_SELECTION_STATEMENT);
            return NULL;
        }

        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_SELECTION_STATEMENT);
            return NULL;
        }

        right_node = _OP(T_DIVIDER, copy_node, right_node);

        if (tl[*tp].token_type != T_DIVIDER) {
            start_parser_err(&data->parser_err, tl[*tp], GET_SELECTION_STATEMENT);
            return NULL;
        }

        (*tp)++;
    }
    if (!cycled) {
        start_parser_err(&data->parser_err, tl[*tp], GET_SELECTION_STATEMENT);
        return NULL;
    }
    (*tp)++;


    return _OP(T_IF, left_node, right_node);
}

ast_tree_elem_t *get_logical_expression(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    ast_tree_elem_t *val = get_additive_expression(data);

    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_LOGICAL_EXPRESSION);
        return val;
    }

    while (
            tl[*tp].token_type == T_LESS    ||
            tl[*tp].token_type == T_MORE    ||
            tl[*tp].token_type == T_MORE_EQ ||
            tl[*tp].token_type == T_LESS_EQ ||
            tl[*tp].token_type == T_EQ
        ) {

        int op_num = tl[*tp].token_type;
        (*tp)++;

        ast_tree_elem_t *val2 = get_additive_expression(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_LOGICAL_EXPRESSION);
            return val;
        }

        val = _OP(op_num, val, val2);
    }

    return val;
}

ast_tree_elem_t *get_additive_expression(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    ast_tree_elem_t *val = get_multiplicative_expression(data);


    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_ADDITIVE_EXPRESSION);
        return val;
    }

    while (tl[*tp].token_type == T_ADD || tl[*tp].token_type == T_SUB) {
        int op_num = tl[*tp].token_type;
        (*tp)++;

        ast_tree_elem_t *val2 = get_multiplicative_expression(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_ADDITIVE_EXPRESSION);
            return val;
        }

        val = _OP(op_num, val, val2);
    }

    return val;
}

ast_tree_elem_t *get_multiplicative_expression(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    ast_tree_elem_t *val = get_direct_declarator(data);



    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_MULTIPLICATIVE_EXPRESSION);
        return val;
    }

    while (tl[*tp].token_type == T_MUL || tl[*tp].token_type == T_DIV) {
        int op_num = tl[*tp].token_type;
        (*tp)++;

        ast_tree_elem_t *val2 = get_direct_declarator(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_MULTIPLICATIVE_EXPRESSION);
            return val;
        }

        val = _OP(op_num, val, val2);
    }

    return val;
}

ast_tree_elem_t *get_direct_declarator(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    if (tl[*tp].token_type == T_O_BRACE) {
        (*tp)++;
        ast_tree_elem_t *val = get_additive_expression(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_DIRECT_DECLARATOR);
            return NULL;
        }

        if (tl[*tp].token_type != T_C_BRACE) {
            start_parser_err(&data->parser_err, tl[*tp], GET_DIRECT_DECLARATOR);
            return val;
        }

        (*tp)++;
        return val;
    } else if (tl[*tp].token_type == T_ID && tl[(*tp) + 1].token_type == T_O_BRACE) {
        ast_tree_elem_t *func_node = get_one_arg_function_call(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_DIRECT_DECLARATOR);
            return func_node;
        }

        return func_node;
    } else if (tl[*tp].token_type == T_ID || tl[*tp].token_type == T_NUM) {

        ast_tree_elem_t *prim_node = get_primary_expression(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_DIRECT_DECLARATOR);
            return prim_node;
        }

        return prim_node;
    } else {
        start_parser_err(&data->parser_err, tl[*tp], GET_DIRECT_DECLARATOR);
        return NULL;
    }
}

ast_tree_elem_t *get_type(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    ast_tree_elem_t *type_node = NULL;

    if (!(tl[*tp].token_type == T_INT || tl[*tp].token_type == T_FLOAT)) {
        start_parser_err(&data->parser_err, tl[*tp], GET_VARIABLE_INITIALIZATION_WITH_ASSIGNMENT);
        return NULL;
    }

    type_node = _TYPE(tl[*tp].token_type);
    (*tp)++;

    return type_node;
}

ast_tree_elem_t *get_variable_initialization(parsing_block_t *data) {
    assert(data != NULL);

    ast_tree_elem_t *type_node = NULL;
    ast_tree_elem_t *main_node = NULL;

    type_node = get_type(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_VARIABLE_INITIALIZATION);
        return NULL;
    }

    main_node = get_variable(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_VARIABLE_INITIALIZATION);
        return NULL;
    }

    return _INIT(type_node, main_node);
}

ast_tree_elem_t *get_variable_initialization_with_assignment(parsing_block_t *data) {
    assert(data != NULL);

    ast_tree_elem_t *type_node = NULL;
    ast_tree_elem_t *main_node = NULL;

    type_node = get_type(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_VARIABLE_INITIALIZATION_WITH_ASSIGNMENT);
        return NULL;
    }

    main_node = get_assignment(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_VARIABLE_INITIALIZATION_WITH_ASSIGNMENT);
        return NULL;
    }

    return _INIT(type_node, main_node);
}

ast_tree_elem_t *get_func_separated_init_args(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    ast_tree_elem_t *init_args = NULL;
    ast_tree_elem_t *copy_node = NULL;


    while (tl[*tp].token_type != T_C_BRACE) {
        copy_node = init_args;
        init_args = get_variable_initialization(data);

        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_FUNC_SEPARATED_INIT_ARGS);
            return NULL;
        }

        if (!(tl[*tp].token_type == T_COMMA || tl[*tp].token_type == T_C_BRACE)) {
            start_parser_err(&data->parser_err, tl[*tp], GET_FUNC_SEPARATED_INIT_ARGS);
            return NULL;
        }

        init_args = _OP(T_COMMA, copy_node, init_args);
    }

    return init_args;
}

ast_tree_elem_t *get_statement_list(parsing_block_t *data) { // there is should be at least one statement
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    ast_tree_elem_t *copy_node = NULL;
    ast_tree_elem_t *right_node = NULL;

    ast_tree_elem_t *statement_node = NULL;

    bool cycled = false;

    while ((statement_node = try_grule(data, get_statement))) {
        cycled = true;
        copy_node = right_node;
        right_node = statement_node;

        if (tl[*tp].token_type != T_DIVIDER) {
            start_parser_err(&data->parser_err, tl[*tp], GET_STATEMENT_LIST);
            return NULL;
        }
        (*tp)++;

        right_node = _OP(T_DIVIDER, copy_node, right_node);

    }

    if (!cycled) {
        start_parser_err(&data->parser_err, tl[*tp], GET_STATEMENT_LIST);
        return NULL;
    }

    return right_node;
}

ast_tree_elem_t *get_function_initialization(parsing_block_t *data) {
    assert(data != NULL);

    ast_tree_elem_t *return_type_node = NULL;
    ast_tree_elem_t *func_identificator = NULL;
    ast_tree_elem_t *args_node = NULL;
    ast_tree_elem_t *func_body_node = NULL;
    ast_tree_elem_t *return_node = NULL;

    return_type_node = get_type(data);
    CATCH_PARSE_ERROR(data, GET_FUNCTION_INITIALIZATION)

    func_identificator = get_func_identificator(data);
    CATCH_PARSE_ERROR(data, GET_FUNCTION_INITIALIZATION)

    STEP_OVER_TOKEN_WITH_CHECK(data, GET_FUNCTION_INITIALIZATION, T_O_BRACE)

    args_node = get_func_separated_init_args(data);
    CATCH_PARSE_ERROR(data, GET_FUNCTION_INITIALIZATION)

    STEP_OVER_TOKEN_WITH_CHECK(data, GET_FUNCTION_INITIALIZATION, T_C_BRACE)

    STEP_OVER_TOKEN_WITH_CHECK(data, GET_FUNCTION_INITIALIZATION, T_O_FIG_BRACE)

    func_body_node = _FUNC_BODY(NULL, NULL);
    func_body_node->right = get_statement_list(data);

    CATCH_PARSE_ERROR(data, GET_FUNCTION_INITIALIZATION)

    STEP_OVER_TOKEN_WITH_CHECK(data, GET_FUNCTION_INITIALIZATION, T_RETURN);
    func_body_node->left = try_grule(data, get_expression);
    STEP_OVER_TOKEN_WITH_CHECK(data, GET_FUNCTION_INITIALIZATION, T_DIVIDER);

    STEP_OVER_TOKEN_WITH_CHECK(data, GET_FUNCTION_INITIALIZATION, T_C_FIG_BRACE)

    func_identificator->left = args_node;
    func_identificator->right = func_body_node;

    return _INIT(return_type_node, func_identificator);
}

ast_tree_elem_t *get_assignment(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    ast_tree_elem_t *left = get_variable(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_ASSIGNMENT);
        return NULL;
    }

    if (tl[*tp].token_type != T_ASSIGN) {
        start_parser_err(&data->parser_err, tl[*tp], GET_ASSIGNMENT);
        return NULL;
    }

    (*tp)++;
    ast_tree_elem_t *right = get_expression(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_ASSIGNMENT);
        return NULL;
    }


    return _ASSIGN(left, right);
}

ast_tree_elem_t *get_one_arg_function_call(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type == T_ID && tl[(*tp) + 1].token_type == T_O_BRACE) {
        lexem_t func_lexem = tl[*tp];
        (*tp) += 2;

        ast_tree_elem_t *val = get_additive_expression(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_ONE_ARG_FUNCTION_CALL);
            return NULL;
        }

        if (tl[*tp].token_type != T_C_BRACE) {
            start_parser_err(&data->parser_err, tl[*tp], GET_ONE_ARG_FUNCTION_CALL);
            return val;
        }

        (*tp)++;
        return _FUNC(val, data->name_table[func_lexem.token_val.ival].name);
    } else {
        start_parser_err(&data->parser_err, tl[*tp], GET_ONE_ARG_FUNCTION_CALL);
        return NULL;
    }
}

ast_tree_elem_t *get_primary_expression(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type == T_ID) {
        ast_tree_elem_t *id_node = get_variable(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_PRIMARY_EXPRESSION);
            return id_node;
        }

        return id_node;
    } else if (tl[*tp].token_type == T_NUM) {
        ast_tree_elem_t *const_node = get_constant(data);
        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_PRIMARY_EXPRESSION);
            return const_node;
        }
        return const_node;
    } else {
        start_parser_err(&data->parser_err, tl[*tp], GET_PRIMARY_EXPRESSION);
        return NULL;
    }
}

ast_tree_elem_t *get_while(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    ast_tree_elem_t *left_node = NULL;
    ast_tree_elem_t *copy_node = NULL;
    ast_tree_elem_t *right_node = NULL;

    if (tl[*tp].token_type != T_WHILE) {
        start_parser_err(&data->parser_err, tl[*tp], GET_WHILE);
        return NULL;
    }
    (*tp)++;

    if (tl[*tp].token_type != T_O_BRACE) {
        start_parser_err(&data->parser_err, tl[*tp], GET_WHILE);
        return NULL;
    }
    (*tp)++;

    left_node = get_expression(data);
    if (data->parser_err.err_state) {
        add_grule_to_parser_err(&data->parser_err, GET_WHILE);
        return NULL;
    }

    if (tl[*tp].token_type != T_C_BRACE) {
        start_parser_err(&data->parser_err, tl[*tp], GET_WHILE);
        return NULL;
    }
    (*tp)++;

    if (tl[*tp].token_type != T_O_FIG_BRACE) {
        start_parser_err(&data->parser_err, tl[*tp], GET_WHILE);
        return NULL;
    }
    (*tp)++;

    bool cycled = false;
    while (tl[*tp].token_type != T_C_FIG_BRACE) {
        cycled = true;

        copy_node = right_node;
        right_node = get_statement(data);

        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_SELECTION_STATEMENT);
            return NULL;
        }

        if (data->parser_err.err_state) {
            add_grule_to_parser_err(&data->parser_err, GET_SELECTION_STATEMENT);
            return NULL;
        }

        right_node = _OP(T_DIVIDER, copy_node, right_node);

        if (tl[*tp].token_type != T_DIVIDER) {
            start_parser_err(&data->parser_err, tl[*tp], GET_SELECTION_STATEMENT);
            return NULL;
        }

        (*tp)++;
    }
    if (!cycled) {
        start_parser_err(&data->parser_err, tl[*tp], GET_SELECTION_STATEMENT);
        return NULL;
    }
    (*tp)++;


    return _OP(T_WHILE, left_node, right_node);





}

ast_tree_elem_t *get_constant(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    long double val = 0;

    if (tl[*tp].token_type != T_NUM) {
        start_parser_err(&data->parser_err, tl[*tp], GET_CONSTANT);
        return NULL;
    }

    val = tl[*tp].token_val.fval;
    (*tp)++;

    return _NUM(val);
}

ast_tree_elem_t *get_variable(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type != T_ID) {
        start_parser_err(&data->parser_err, tl[*tp], GET_VARIABLE);
        return NULL;
    }

    char *var_name = data->name_table[tl[*tp].token_val.ival].name;
    (*tp)++;

    return _VAR(var_name);
}

ast_tree_elem_t *get_func_identificator(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type != T_ID) {
        start_parser_err(&data->parser_err, tl[*tp], GET_FUNC_IDENTIFICATOR);
        return NULL;
    }

    char *func_id = data->name_table[tl[*tp].token_val.ival].name;
    (*tp)++;

    return _FUNC_ID(func_id);
}

#undef CATCH_PARSE_ERROR
#undef STEP_OVER_TOKEN_WITH_CHECK