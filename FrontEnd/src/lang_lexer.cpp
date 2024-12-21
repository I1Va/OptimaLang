
#include <cstdio>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#include "AST_io.h"
#include "general.h"
#include "lang_logger.h"
#include "lang_global_space.h"
#include "string_funcs.h"
#include "lang_lexer.h"

const size_t STRING_PREVIEW_CNT = 10;

bool char_in_str_lex(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

int get_index_in_name_table(char *new_name, parsing_block_t *data) {
    for (size_t name_idx = 0; name_idx < data->name_table_sz; name_idx++) {
        if (strcmp(new_name, data->name_table[name_idx].name) == 0) {
            return (int) name_idx;
        }
    }
    return -1;
}

int get_index_in_keyword_table(char *new_name, parsing_block_t *data) {
    for (size_t name_idx = 0; name_idx < data->keywords_table_sz; name_idx++) {
        if (strcmp(new_name, data->keywords_table[name_idx].name) == 0) {
            return (int) name_idx;
        }
    }
    return -1;
}

int add_to_name_table(char *new_name, parsing_block_t *data) {
    assert(new_name);
    assert(data);

    data->name_table[data->name_table_sz++] = {new_name, strlen(new_name), AST_ID};
    return (int) data->name_table_sz - 1;
}

bool try_parse_num(lexem_t *lexem, char *str) {
    assert(lexem);
    assert(str);
    if (!isdigit(*str) && *str != '-') {return false;}

    long double val = 0.0;
    int         len = 0;

    if (!sscanf(str, "%Lg%n", &val, &len)) {
        return false;
    }

    lexem->token_type     = AST_NUM;
    lexem->token_val      = {};
    lexem->token_val.fval = val;
    lexem->text_pos       = {};
    lexem->len            = (size_t) len;
    lexem->key_word_state = false;

    return true;
}

bool try_parse_string_literal(parsing_block_t *data, lexem_t *lexem, char *str) {
    assert(data);
    assert(lexem);
    assert(str);
    if (*str != '"') {return false;}

    char *open_quote = str;
    char *close_quote = strchr(str + 1, '"');

    if (!close_quote) {
        debug("CLOSE_QUOTE ABSENT: '%*.s'", STRING_PREVIEW_CNT, str);
        lexem->token_type = AST_EOF;
        return true;
    }

    size_t len = (size_t) (close_quote - open_quote) + 1;

    lexem->token_type = AST_STR_LIT;
    lexem->len = len - 2;
    lexem->token_val.sval = get_new_str_ptr(data->storage, lexem->len);
    strncpy(lexem->token_val.sval, str + 1, len);

    lexem->key_word_state = false;

    return true;
}

bool try_parse_identificator(parsing_block_t *data, lexem_t *lexem, char *str, char *bufer) {
    assert(data);
    assert(lexem);
    assert(str);
    if (!char_in_str_lex(*str)) {return false;}

    size_t len = 0;
    while (char_in_str_lex(*(str + len))) {
        len++;
    }

    char *identificator = get_new_str_ptr(data->storage, len);
    strncpy(identificator, str, len);

    int keyword_idx = get_index_in_keyword_table(identificator, data);
    if (keyword_idx == -1) {
        int name_idx = get_index_in_name_table(identificator, data);
        if (name_idx == -1) {
            name_idx = add_to_name_table(identificator, data);
        }
        lexem->token_type = data->name_table[name_idx].token_type;
        lexem->token_val.ival = name_idx;
        lexem->key_word_state = false;
        lexem->len = data->name_table[name_idx].len;
    } else {
        lexem->token_type = data->keywords_table[keyword_idx].token_type;
        lexem->token_val.ival = keyword_idx;
        lexem->key_word_state = true;
        lexem->len = data->keywords_table[keyword_idx].len;
    }
    return true;
}

bool try_parse_single_sim(lexem_t *lexem, char *str) {
    assert(lexem);
    assert(str);
    #define _STR_CHECK_SINGLE_SIM(s, neg_s, type) \
        if (*str == s && *(str + 1) != neg_s) {   \
            *lexem = {type, {}, {}, 1};           \
            return true;                          \
        }

    _STR_CHECK_SINGLE_SIM('=', '=', AST_ASSIGN);
    _STR_CHECK_SINGLE_SIM('>', '=', AST_MORE);
    _STR_CHECK_SINGLE_SIM('<', '=', AST_LESS);

    switch (*str) {
        case '+':  *lexem = {AST_ADD, {}, {}, 1};         return true;
        case '-':  *lexem = {AST_SUB, {}, {}, 1};         return true;
        case '*':  *lexem = {AST_MUL, {}, {}, 1};         return true;
        case '(':  *lexem = {AST_O_BRACE, {}, {}, 1};     return true;
        case ')':  *lexem = {AST_C_BRACE, {}, {}, 1};     return true;
        case '{':  *lexem = {AST_O_FIG_BRACE, {}, {}, 1}; return true;
        case '}':  *lexem = {AST_C_FIG_BRACE, {}, {}, 1}; return true;
        case '\n': *lexem = {AST_EOL, {}, {}, 1};         return true;
        case ' ':  *lexem = {AST_SPACE, {}, {}, 1};       return true;
        case '/':  *lexem = {AST_DIV, {}, {}, 1};         return true;
        case '\t': *lexem = {AST_SPACE, {}, {}, 4};       return true;
        case '^':  *lexem = {AST_POW, {}, {}, 1};         return true;
        case EOF:  *lexem = {AST_EOF, {}, {}, 1};         return true;
        case '\0': *lexem = {AST_EOF, {}, {}, 1};         return true;
        case ';':  *lexem = {AST_SEMICOLON, {}, {}, 1};   return true;
        case ',':  *lexem = {AST_COMMA, {}, {}, 1};       return true;
        default: return false;
    }

    #undef _STR_CHECK_SINGLE_SIM
}

bool try_parse_double_sim(lexem_t *lexem, char *str) {
    assert(lexem);
    assert(str);
    #define _STR_CHECK_DOUBLE_SIM(s1, s2, type) \
        if (*str == s1 && *(str + 1) == s2) {   \
            *lexem = {type, {}, {}, 2};         \
            return true;                        \
        }

    _STR_CHECK_DOUBLE_SIM('>', '=', AST_MORE_EQ)
    _STR_CHECK_DOUBLE_SIM('<', '=', AST_LESS_EQ)
    _STR_CHECK_DOUBLE_SIM('=', '=', AST_EQ)

    #undef _STR_CHECK_DOUBLE_SIM

    return false;
}

lexem_t next_lexem(parsing_block_t *data) {
    char   *s        = data->text;
    size_t *p        = &data->text_idx;

    char    bufer[MEDIUM_BUFER_SZ] = {};
    lexem_t lexem                  = {};

    if (try_parse_num(&lexem, s + *p)) {
        *p += (size_t) lexem.len;
        return lexem;
    }
    if (try_parse_string_literal(data, &lexem, s + *p)) {
        *p += (size_t) lexem.len + 2; // quotes: +2
        return lexem;
    }
    if (try_parse_identificator(data, &lexem, s + *p, bufer)) {
        *p += (size_t) lexem.len;
        return lexem;
    }
    if (try_parse_single_sim(&lexem, s + *p)) {
        *p += (size_t) lexem.len;
        return lexem;
    }
    if (try_parse_double_sim(&lexem, s + *p)) {
        *p += (size_t) lexem.len;
        return lexem;
    }

    debug("UNKNOWN_SYMS: %*s", STRING_PREVIEW_CNT, s);
    return {AST_EOF};
}

void text_pos_update(text_pos_t *text_pos, const lexem_t lexem) {
    if (lexem.token_type == AST_EOL) {
        text_pos->lines++;
        text_pos->syms = 0;
        return;
    }

    text_pos->syms += lexem.len;
}

bool check_lextype_for_skip(const enum ast_token_t token_type) {
    return
    (
        token_type == AST_SPACE ||
        token_type == AST_EOL
    );
}

void lex_scanner(parsing_block_t *data) {
    assert(data != NULL);

    size_t token_idx = 0;
    text_pos_t cur_text_pos = {};

    // printf("text: '%s'\n", data->text);

    while (1) {
        lexem_t lexem = next_lexem(data);

        lexem.text_pos = cur_text_pos;
        text_pos_update(&cur_text_pos, lexem);

        if (!check_lextype_for_skip(lexem.token_type)) {
            data->lexem_list[token_idx++] = lexem;
        }

        if (lexem.token_type == AST_EOF) {
            break;
        }
    }

    data->lexem_list_size = token_idx;

    // lexem_list_dump(stdout, data);
}