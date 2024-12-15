#ifndef PROCESSOR_FUNC_H
#define PROCESSOR_FUNC_H

#include <stdio.h>
#include <limits.h>
#include "proc_err.h"

#include "bind_coms.h"

const size_t CONSOLE_HEIGHT = 50;
const size_t CONSOLE_WIDTH = CONSOLE_HEIGHT * 2;
const size_t REG_LIST_SZ = 4;
const long long ACCURACY = 100;
const long long SQRT_ACCURACY = 10;
const size_t PROC_RAM_MAX_SZ = CONSOLE_WIDTH * CONSOLE_HEIGHT + (1u << 15);

struct proc_data_t {
    int code[MAX_BIN_CODE_SZ];
    long long REG_LIST[REG_LIST_SZ];
    long long PROC_RAM[PROC_RAM_MAX_SZ];
};

void proc_data_t_ctor(proc_data_t *proc_data);
int bin_code_read(const char path[], int code[], proc_err *return_err);
void execute_code(proc_data_t *proc_data, proc_err *return_err);
size_t get_bin_code_real_sz(int bin_code[], const size_t n);

#endif // PROCESSOR_FUNC_H