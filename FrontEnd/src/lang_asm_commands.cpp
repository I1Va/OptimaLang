#include "lang_global_space.h"
#include "general.h"

void PUSH_NUM(FILE *stream, const long long num) {
    fprintf(stream, "push %Ld;\n", num);
}

void WRITE_OPER(FILE *stream, const token_t oper) {
    switch (oper) {
        case T_MUL: fprintf(stream, "mult;\n"); break;
        case T_ADD: fprintf(stream, "add;\n"); break;
        case T_DIV: fprintf(stream, "div;\n"); break;
        case T_SUB: fprintf(stream, "sub;\n"); break;
        default: debug("'%s' isn't operation", oper); break;
    }
}

void OUT(FILE *stream) {
    fprintf(stream, "out;\n");
}

void HLT(FILE *stream) {
    fprintf(stream, "hlt;\n");
}
