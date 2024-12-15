#ifndef LANG_ASM_COMMANDS_H
#define LANG_ASM_COMMANDS_H

#include <stdlib.h>
#include <stdio.h>

#include "lang_global_space.h"

void PUSH_NUM(FILE *stream, const long long num);

void WRITE_OPER(FILE *stream, const token_t oper);

void OUT(FILE *stream);

void HLT(FILE *stream);

#endif // LANG_ASM_COMMANDS_H