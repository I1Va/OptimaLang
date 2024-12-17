
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#include "general.h"
#include "lang_logger.h"
#include "lang_global_space.h"
#include "lang_lexer.h"

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

    data->name_table[data->name_table_sz++] = {new_name, strlen(new_name), T_ID};
    return (int) data->name_table_sz - 1;
}

size_t scan_lval(long long *lval, char *p) {
    size_t len = 0;

    while (isdigit(*p)) {
        *lval = 10 * (*lval) + *p - '0';
        len++;
        p++;
    }

    return len;
}

lexem_t next_lexem(parsing_block_t *data) {
    char *s = data->text;
    size_t *p = &data->text_idx;

    int c = s[(*p)++];

    char bufer[MEDIUM_BUFER_SZ] = {};
    size_t bufer_idx = 0;
    char *str = NULL;

    lexem_t lexem = {};

    if (isdigit(c)) {
        lexem.token_type = T_NUM;

        long long l_part = 0;
        long long frac_part = 0;
        long double fval = 0;

        size_t len_l_part = 0;
        size_t len_frac_part = 0;

        (*p)--;

        len_l_part = scan_lval(&l_part, s + *p);
        (*p) += len_l_part;

        if (s[*p] == '.') {
            (*p)++;
            len_frac_part = scan_lval(&frac_part, s + *p);
            (*p) += len_frac_part;
        }
        printf("vals: %Ld, %Ld, %lu, %lu\n", l_part, frac_part, len_l_part, len_frac_part);
        fval = (long double) l_part + (long double) (frac_part) / pow(10, len_frac_part);

        lexem.token_val.fval = fval;
        lexem.len = len_l_part + (len_frac_part > 0) + len_frac_part;

        return lexem;
    }

    if (char_in_str_lex(c)) {
        while (char_in_str_lex(c)) {
            bufer[bufer_idx++] = (char) c;
            c = s[(*p)++];
        }
        (*p)--;

        str = get_new_str_ptr(data->storage, bufer_idx);
        strncpy(str, bufer, bufer_idx); // FIXME: можно ускорить

        int keyword_idx = get_index_in_keyword_table(str, data);
        if (keyword_idx == -1) {
            int name_idx = get_index_in_name_table(str, data);
            if (name_idx == -1) {
                name_idx = add_to_name_table(str, data);
            }

            lexem.token_type = data->name_table[name_idx].token_type;
            lexem.token_val.ival = name_idx;
            lexem.key_word_state = false;
            lexem.len = data->name_table[name_idx].len;
        } else { // FIXME: фу, кринж, копипаст
            lexem.token_type = data->keywords_table[keyword_idx].token_type;
            lexem.token_val.ival = keyword_idx;
            lexem.key_word_state = true;
            lexem.len = data->keywords_table[keyword_idx].len;
        }

        return lexem;
    }

    switch (c) {
        case '+': return {T_ADD, {}, {}, 1}; case '-': return {T_SUB, {}, {}, 1};
        case '*': return {T_MUL, {}, {}, 1};
        case '(': return {T_O_BRACE, {}, {}, 1}; case ')': return {T_C_BRACE, {}, {}, 1};
        case '{': return {T_O_FIG_BRACE, {}, {}, 1}; case '}': return {T_C_FIG_BRACE, {}, {}, 1};
        case '\n': return {T_EOL, {}, {}, 1};
        case ' ': return {T_SPACE, {}, {}, 1};
        case '/': return {T_DIV, {}, {}, 1};
        case '\t': return {T_SPACE, {}, {}, 4};
        case '^': return {T_POW, {}, {}, 1};
        case EOF: return {T_EOF, {}, {}, 1};
        case '\0': return {T_EOF, {}, {}, 1};
        case ';': return {T_DIVIDER, {}, {}, 1};
        case '=': return {T_ASSIGN, {}, {}, 1};
        case ',': return {T_COMMA, {}, {}, 1};
        default: break;
    }

    char c_next = s[(*p)];
    if (c == '>' && c_next == '=') {
        (*p)++;
        return {T_MORE_EQ, {}, {}, 1};
    }
    if (c == '<' && c_next == '=') {
        (*p)++;
        return {T_LESS_EQ, {}, {}, 1};
    }
    if (c == '=' && c_next == '=') {
        (*p)++;
        return {T_EQ, {}, {}, 1};
    }
    if (c == '>' && c_next != '=') {
        return {T_MORE, {}, {}, 1};
    }
    if (c == '<' && c_next != '=') {
        return {T_LESS, {}, {}, 1};
    }

    debug("UNKNOWN_SYMS: %c %c", c, c_next);
    return {T_EOF};
}

void text_pos_update(text_pos_t *text_pos, const lexem_t lexem) {
    if (lexem.token_type == T_EOL) {
        text_pos->lines++;
        text_pos->syms = 0;
        return;
    }

    text_pos->syms += lexem.len;
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

        if (lexem.token_type != T_SPACE && lexem.token_type != T_EOL) {
            data->lexem_list[token_idx++] = lexem;
        }
        if (lexem.token_type == T_EOF) {
            break;
        }
    }
    data->lexem_list_size = token_idx;

    lexem_list_dump(stdout, data);
}