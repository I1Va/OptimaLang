#include "AST_io.h"
#include "lang_global_space.h"
#include "lang_grammar.h"
#include "lang_logger.h"
#include "general.h"
#include "string_funcs.h"

#include <assert.h>

void lexem_dump(FILE *stream, parsing_block_t *data, lexem_t lexem) {
    #define T_DESCR_(stream, lex, fmt, val) case lex: fprintf(stream, #lex"(" fmt ")", val); break;
    fprintf(stream, "[l:%3lu, s:%3lu]: ", lexem.text_pos.lines + 1, lexem.text_pos.syms + 1);
    switch (lexem.token_type) {
        T_DESCR_(stream, AST_EOF, "%s", "")
        T_DESCR_(stream, AST_EMPTY, "%s", "")
        T_DESCR_(stream, AST_NUM, "%Lg", lexem.token_val.fval)
        T_DESCR_(stream, AST_ADD, "%c", '+')
        T_DESCR_(stream, AST_MUL, "%c", '*')
        T_DESCR_(stream, AST_SUB, "%c", '-')
        T_DESCR_(stream, AST_DIV, "%c", '/')
        T_DESCR_(stream, AST_O_BRACE, "%c", '(')
        T_DESCR_(stream, AST_C_BRACE, "%c", ')')
        T_DESCR_(stream, AST_O_FIG_BRACE, "%c", '{')
        T_DESCR_(stream, AST_C_FIG_BRACE, "%c", '}')
        T_DESCR_(stream, AST_EOL, "%s", "\\n")
        T_DESCR_(stream, AST_SPACE, "%c", ' ')
        T_DESCR_(stream, AST_POW, "%s", "^")
        T_DESCR_(stream, AST_ID, "%s", data->name_table[lexem.token_val.ival].name)
        T_DESCR_(stream, AST_IF, "%s", data->keywords_table[lexem.token_val.ival].name)
        T_DESCR_(stream, AST_WHILE, "%s", data->keywords_table[lexem.token_val.ival].name)
        T_DESCR_(stream, AST_SEMICOLON, "%c", ';')
        T_DESCR_(stream, AST_LESS, "%c", '<')
        T_DESCR_(stream, AST_MORE, "%c", '>')
        T_DESCR_(stream, AST_LESS_EQ, "%s", "<=")
        T_DESCR_(stream, AST_MORE_EQ, "%s", ">=")
        T_DESCR_(stream, AST_EQ, "%s", "==")
        T_DESCR_(stream, AST_INT, "%s", "int")
        T_DESCR_(stream, AST_FLOAT, "%s", "float")
        T_DESCR_(stream, AST_ASSIGN, "%s", "=")
        T_DESCR_(stream, AST_COMMA, "%c", ',')
        T_DESCR_(stream, AST_RETURN, "%s", "return")
        T_DESCR_(stream, AST_ELSE, "%s", "else")
        T_DESCR_(stream, AST_VOID, "%s", "void")
        T_DESCR_(stream, AST_STR_LIT, "\"%s\"", lexem.token_val.sval)


        default: fprintf(stream, "UNKNOWN_LEX(%d) ", lexem.token_type); break;
    }
    #undef T_DESCR_
}

void lexem_list_dump(FILE *stream, parsing_block_t *data) {
    fprintf_title(stream, "LEXEM_LIST_DUMP", '-', STR_F_BORDER_SZ);

    printf("len: [%lu]\n", data->lexem_list_size);
    for (size_t i = 0; i < data->lexem_list_size; i++) {
        lexem_t lexem = data->lexem_list[i];
        lexem_dump(stream, data, lexem);
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");

    fprintf_border(stream, '-', STR_F_BORDER_SZ, true);
}

void name_table_dump(FILE *stream, parsing_block_t *data) {
    fprintf(stream, "name_table[%p]\n{\n", data->name_table);

    for (size_t i = 0; i < data->name_table_sz; i++) {
        fprintf(stream, "    [%lu] = {'%s', [%d]}\n", i, data->name_table[i].name, data->name_table[i].token_type);
    }

    fprintf(stream, "}\n");
}

void grule_dump(FILE *stream, enum grammar_rule_num grule) {
    #define GR_DESCR_(stream, grule) case grule: fprintf(stream, #grule); break;

    switch (grule) {
        GR_DESCR_(stream, EMPTY_GRULE)
        GR_DESCR_(stream, GET_SYNTAX_ANALYSIS)
        GR_DESCR_(stream, GET_ADDITIVE_EXPRESSION)
        GR_DESCR_(stream, GET_MULTIPLICATIVE_EXPRESSION)
        GR_DESCR_(stream, GET_DIRECT_DECLARATOR)
        GR_DESCR_(stream, GET_ONE_ARG_FUNCTION_CALL)
        GR_DESCR_(stream, GET_PRIMARY_EXPRESSION)
        GR_DESCR_(stream, GET_CONSTANT)
        GR_DESCR_(stream, GET_VARIABLE)
        GR_DESCR_(stream, GET_SELECTION_STATEMENT)
        GR_DESCR_(stream, GET_STATEMENT)
        GR_DESCR_(stream, GET_ASSIGNMENT)
        GR_DESCR_(stream, GET_LOGICAL_EXPRESSION)
        GR_DESCR_(stream, GET_EXPRESSION)
        GR_DESCR_(stream, GET_WHILE)
        GR_DESCR_(stream, GET_VARIABLE_INITIALIZATION)
        GR_DESCR_(stream, GET_VARIABLE_INITIALIZATION_WITH_ASSIGNMENT)
        GR_DESCR_(stream, GET_FUNC_SEPARATED_INIT_ARGS)
        GR_DESCR_(stream, GET_FUNCTION_INITIALIZATION)
        GR_DESCR_(stream, GET_FUNC_IDENTIFICATOR)
        GR_DESCR_(stream, GET_STATEMENT_LIST)
        GR_DESCR_(stream, GET_GLOBAL_STATEMENT)
        GR_DESCR_(stream, GET_GRULE_DIVIDED_LIST)
        GR_DESCR_(stream, GET_FUNCTION_CALL)
        GR_DESCR_(stream, GET_SCOPE)
        GR_DESCR_(stream, GET_TYPE)

        default: fprintf(stream, "UNKNOWN_GRULE(%d) ", grule); break;
    }
    #undef GR_DESCR_
}
