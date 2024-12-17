#include "lang_global_space.h"
#include "lang_grammar.h"
#include "lang_logger.h"
#include "general.h"
#include "string_funcs.h"

#include <assert.h>

void lexem_dump(FILE *stream, key_name_t *name_table, lexem_t lexem) {
    #define T_DESCR_(stream, lex, fmt, val) case lex: fprintf(stream, #lex"(" fmt ")", val); break;
    fprintf(stream, "[l:%3lu, s:%3lu]: ", lexem.text_pos.lines + 1, lexem.text_pos.syms + 1);
    switch (lexem.token_type) {
        T_DESCR_(stream, T_EOF, "%s", "")
        T_DESCR_(stream, T_EMPTY, "%s", "")
        T_DESCR_(stream, T_NUM, "%Lg", lexem.token_val.fval)
        T_DESCR_(stream, T_ADD, "%c", '+')
        T_DESCR_(stream, T_MUL, "%c", '*')
        T_DESCR_(stream, T_SUB, "%c", '-')
        T_DESCR_(stream, T_DIV, "%c", '/')
        T_DESCR_(stream, T_O_BRACE, "%c", '(')
        T_DESCR_(stream, T_C_BRACE, "%c", ')')
        T_DESCR_(stream, T_O_FIG_BRACE, "%c", '{')
        T_DESCR_(stream, T_C_FIG_BRACE, "%c", '}')
        T_DESCR_(stream, T_EOL, "%s", "\\n")
        T_DESCR_(stream, T_SPACE, "%c", ' ')
        T_DESCR_(stream, T_POW, "%s", "^")
        T_DESCR_(stream, T_ID, "%s", name_table[lexem.token_val.ival].name)
        T_DESCR_(stream, T_IF, "%s", name_table[lexem.token_val.ival].name)
        T_DESCR_(stream, T_WHILE, "%s", name_table[lexem.token_val.ival].name)
        T_DESCR_(stream, T_DIVIDER, "%c", ';')
        T_DESCR_(stream, T_LESS, "%c", '<')
        T_DESCR_(stream, T_MORE, "%c", '>')
        T_DESCR_(stream, T_LESS_EQ, "%s", "<=")
        T_DESCR_(stream, T_MORE_EQ, "%s", ">=")
        T_DESCR_(stream, T_EQ, "%s", "==")
        T_DESCR_(stream, T_INT, "%s", "int")
        T_DESCR_(stream, T_FLOAT, "%s", "float")
        T_DESCR_(stream, T_ASSIGN, "%s", "=")
        T_DESCR_(stream, T_COMMA, "%c", ',')
        T_DESCR_(stream, T_RETURN, "%s", "return")

        default: fprintf(stream, "UNKNOWN_LEX(%d) ", lexem.token_type); break;
    }
    #undef T_DESCR_
}

void lexem_list_dump(FILE *stream, parsing_block_t *data) {
    fprintf_title(stream, "LEXEM_LIST_DUMP", '-', STR_F_BORDER_SZ);

    printf("len: [%lu]\n", data->lexem_list_size);
    for (size_t i = 0; i < data->lexem_list_size; i++) {
        lexem_t lexem = data->lexem_list[i];
        lexem_dump(stream, data->name_table, lexem);
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");

    fprintf_border(stream, '-', STR_F_BORDER_SZ, true);
}

void name_table_dump(FILE *stream, key_name_t *name_table, const size_t name_table_sz) {
    fprintf(stream, "name_table[%p]\n{\n", name_table);

    for (size_t i = 0; i < name_table_sz; i++) {
        fprintf(stream, "    [%lu] = {'%s', [%d]}\n", i, name_table[i].name, name_table[i].token_type);
    }

    fprintf(stream, "}\n");
}

void grule_dump(FILE *stream, enum grammar_rule_num grule) {
    #define GR_DESCR_(stream, grule) case grule: fprintf(stream, #grule); break;

    switch (grule) {
        GR_DESCR_(stream, EMPTY_GRULE)
        GR_DESCR_(stream, GET_CODE_BLOCK)
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

        default: fprintf(stream, "UNKNOWN_GRULE(%d) ", grule); break;
    }
    #undef GR_DESCR_
}
