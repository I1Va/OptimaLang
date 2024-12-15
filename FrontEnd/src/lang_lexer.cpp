
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "general.h"
#include "lang_logger.h"
#include "lang_global_space.h"
#include "lang_lexer.h"

bool char_in_str_lex(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

size_t add_to_name_table(char *new_name, key_name_t *name_table, size_t *name_table_sz) {
    assert(new_name);
    assert(name_table);
    assert(name_table_sz);

    for (size_t name_idx = 0; name_idx < *name_table_sz; name_idx++) {
        if (strcmp(new_name, name_table[name_idx].name) == 0) {
            return name_idx;
        }
    }
    name_table[(*name_table_sz)++] = {new_name, strlen(new_name), T_ID};

    return (*name_table_sz) - 1;
}

lexem_t next_lexem(parsing_block_t *data) {
    char *s = data->text;
    size_t *p = &data->text_idx;

    int c = s[(*p)++];

    long long lval = 0;
    char bufer[MEDIUM_BUFER_SZ] = {};
    size_t bufer_idx = 0;
    char *str = NULL;

    lexem_t lexem = {};

    if (isdigit(c)) {
        lexem.token_type = T_NUM;
        size_t start_p = *p;

        while (isdigit(c)) {
            lval = 10 * lval + c - '0';
            c = s[(*p)++];
        }
        (*p)--;

        lexem.token_val.lval = lval;
        lexem.len = (*p) + 1 - start_p;

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

        size_t name_idx = add_to_name_table(str, data->name_table, &data->name_table_sz);

        lexem.token_type = data->name_table[name_idx].token_type;
        lexem.token_val.ival = (int) name_idx;
        lexem.len = data->name_table[name_idx].len;

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
        default: ScannerError(*p, s[*p])
    }
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

    printf("text: '%s'\n", data->text);

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

size_t get_name_table_sz(key_name_t *name_table) {
    size_t sz = 0;

    while (name_table && name_table->name) {
        sz++;
        name_table++;
    }

    return sz;
}