#include "bind_coms.h"
#include "general.h"
#include "proc_err.h"
#include <cassert>
#include <climits>
#include <cstddef>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "stack_funcs.h"
#include "stack_err_proc.h"
#include "processor_func.h"
#include "string_funcs.h"
#include "proc_err.h"


void proc_data_t_ctor(proc_data_t *proc_data) {
    return;
}

size_t get_bin_code_real_sz(long long bin_code[], const size_t n) {
    for (size_t bin_code_idx = 0; bin_code_idx < n; bin_code_idx++) {
        if (bin_code[bin_code_idx] == HLT_COM) {
            return bin_code_idx + 1;
        }
    }
    return n;
}

size_t get_file_sz(const char *const path, proc_err *return_err) {
    struct stat buf = {};
    if (stat(path, &buf) != 0) {
        proc_add_err(return_err, PROC_ERR_FILE_STAT);
        DEBUG_STK_ERROR(STK_ERR_STAT)
        return 0;
    }

    return (size_t) buf.st_size;
}

int bin_code_read(const char path[], int code[], proc_err *return_err) {
    size_t com_idx = 0;
    FILE *bin_code_file_ptr = NULL;
    size_t bin_code_file_sz = 0;
    size_t com_n = 0;

    bin_code_file_sz = get_file_sz(path, return_err);
    if (*return_err != PROC_ERR_OK) {
        DEBUG_STK_ERROR(STK_ERR_STAT);
        CLEAR_MEMORY(exit_mark);
    }

    bin_code_file_ptr = fopen(path, "rb");
    if (bin_code_file_ptr == NULL) {
        proc_add_err(return_err, PROC_ERR_FILE_OPEN);
        DEBUG_STK_ERROR(STK_ERR_FILE_OPEN)
        CLEAR_MEMORY(exit_mark)
    }
    // printf("SIZE: %lu\n", bin_code_file_sz);
    com_n = bin_code_file_sz / sizeof(code[0]);
    fread(code, sizeof(code[0]), com_n, bin_code_file_ptr);

    fclose(bin_code_file_ptr);

    return (int) com_n;

    exit_mark:
    if (bin_code_file_ptr != NULL) {
        fclose(bin_code_file_ptr);
    }

    return -1;
}

void execute_code(proc_data_t *proc_data, proc_err *return_err) {
    long long ip = 0;
    long long com = 0;

    stk_err stk_last_err = STK_ERR_OK;
    int *code = proc_data->code;
    long long *PROC_RAM = proc_data->PROC_RAM;
    long long *REG_LIST = proc_data->REG_LIST;

    stack_t stk = {};  STACK_INIT(&stk, 0, sizeof(long long), NULL, &stk_last_err);
    stack_t call_stk = {}; STACK_INIT(&call_stk, 0, sizeof(long long), NULL, &stk_last_err);
    // printf("filter_mask\n");
    // fprintf_bin(stdout, filter_mask);
    while (1) {

        com = code[ip++];
        // printf("com[%d] : {%d}", ip - 1, com);
        // printf("com[%d]: '%d'\n", ip, com);
        // fprintf_bin(stdout, filter_mask & com);
        // printf("{%d} vs {%d}\n", com, filter_mask & com);
        long long argv_sum = 0;
        double double_argv = 0.0;
        double double_argv_sum = 0.0;
        long long argv = 0;
        long long argv1 = 0;
        long long argv2 = 0;
        long long argv3 = 0;

        long long reg_id = -1;
        long long addr = 0;

        long long call_label = -1;

        bool hlt_state = false;


        switch (com & filter_mask) {
            case JMP_COM:
                // printf("jump from {%d} to {%d}", ip, code[ip]);
                addr = code[ip++];
                ip = addr;

                break;
            case JA_COM:
                addr = code[ip++];

                stack_pop(&stk, &argv1, &stk_last_err); argv1 /= ACCURACY;
                stack_pop(&stk, &argv2, &stk_last_err); argv2 /= ACCURACY;
                if (argv1 > argv2) {
                    ip = addr;
                }

                break;
            case JAE_COM:
                addr = code[ip++];
                stack_pop(&stk, &argv1, &stk_last_err); argv1 /= ACCURACY;
                stack_pop(&stk, &argv2, &stk_last_err); argv2 /= ACCURACY;
                if (argv1 >= argv2) {
                    ip = addr;
                }
                break;
            case JB_COM:
                addr = code[ip++];
                stack_pop(&stk, &argv1, &stk_last_err); argv1 /= ACCURACY;
                stack_pop(&stk, &argv2, &stk_last_err); argv2 /= ACCURACY;
                if (argv1 < argv2) {
                    ip = addr;
                }
                break;
            case JBE_COM:
                addr = code[ip++];
                stack_pop(&stk, &argv1, &stk_last_err); argv1 /= ACCURACY;
                stack_pop(&stk, &argv2, &stk_last_err); argv2 /= ACCURACY;
                // printf("argv1: {%d}, argv2: {%d}", argv1, argv2);
                if (argv1 <= argv2) {
                    ip = addr;
                }
                break;
            case JE_COM:
                addr = code[ip++];
                stack_pop(&stk, &argv1, &stk_last_err); argv1 /= ACCURACY;
                stack_pop(&stk, &argv2, &stk_last_err); argv2 /= ACCURACY;
                if (argv1 == argv2) {
                    ip = addr;
                }
                break;
            case JNE_COM:
                addr = code[ip++];
                stack_pop(&stk, &argv1, &stk_last_err); argv1 /= ACCURACY;
                stack_pop(&stk, &argv2, &stk_last_err); argv2 /= ACCURACY;
                // printf("argv1: {%d}, argv2: {%d}\n", argv1, argv2);
                if (argv1 != argv2) {
                    ip = addr;
                }
                break;
            case IN_COM:
                double_argv = 0;
                scanf("%lg", &double_argv);
                // printf("push: {%d}", (stack_elem_t) (double_argv * ACCURACY));
                double_argv *= ACCURACY;
                argv = (long long) double_argv;

                stack_push(&stk, &argv, &stk_last_err);
                // printf("pop: {%d}", stack_pop(&stk, &stk_last_err));

                break;
            case OUT_COM:
                double_argv = 0;

                //argv = stack_pop(&stk, &stk_last_err);
                // printf("num: {%d}\n", argv);
                stack_pop(&stk, &argv, &stk_last_err);
                double_argv = ((double) argv) / ACCURACY;

                printf("%lg", double_argv);
                break;
            case OUTC_COM:
                stack_pop(&stk, &argv, &stk_last_err); argv /= ACCURACY;
                printf("%c", argv);
                break;
                // FIXME: после break пустая строка

            case ADD_COM:
                stack_pop(&stk, &argv1, &stk_last_err);
                stack_pop(&stk, &argv2, &stk_last_err);
                argv3 = argv1 + argv2;

                stack_push(&stk, &argv3, &stk_last_err);
                break;
            case SUB_COM:
                stack_pop(&stk, &argv1, &stk_last_err);
                stack_pop(&stk, &argv2, &stk_last_err);
                argv3 = argv1 - argv2;

                stack_push(&stk, &argv3, &stk_last_err);
                break;
            case DIV_COM:
                stack_pop(&stk, &argv1, &stk_last_err);
                stack_pop(&stk, &argv2, &stk_last_err);
                double_argv = (double) argv1 / argv2;
                argv = (long long) (double_argv * ACCURACY);

                stack_push(&stk, &argv, &stk_last_err);
                break;
            case SQRT_COM:
                stack_pop(&stk, &argv, &stk_last_err);

                double_argv = (double) argv;
                double_argv = sqrt(double_argv);

                argv = (long long) (double_argv * SQRT_ACCURACY);

                stack_push(&stk, &argv, &stk_last_err);
                break;
            case MULT_COM:
                stack_pop(&stk, &argv1, &stk_last_err);
                stack_pop(&stk, &argv2, &stk_last_err);

                double_argv = argv1 * argv2;
                argv = (long long) (double_argv * SQRT_ACCURACY);

                stack_push(&stk, &argv, &stk_last_err);
                break;
            case LABEL_COM:
                break;
            case CALL_COM:
                argv = ip + 1;
                stack_push(&call_stk, &argv, &stk_last_err);
                addr = code[ip++];
                ip = addr;
                break;
            case DRAW_COM:
                // printf("H/W {%ld}, {%ld}", CONSOLE_HEIGHT, CONSOLE_WIDTH);
                for (size_t i = 0; i < CONSOLE_HEIGHT; i++) {
                    for (size_t j = 0; j < CONSOLE_WIDTH; j++) {

                        printf("%c", PROC_RAM[i * CONSOLE_WIDTH + j] / ACCURACY);
                        // printf("%c", PROC_RAM[i * CONSOLE_WIDTH + j] / ACCURACY); // FIXME: чтобы круг был круглым, можно раскоментить эту строчку
                        // т.е. кажду. клетку дублировать
                    }
                    printf("\n");
                }
                break;
            case RET_COM:
                stack_pop(&call_stk, &call_label, &stk_last_err);
                ip = call_label;
                break;

            case PUSH_COM: // WARNING: asm PUSH doesn't support fractional numbers. It recieve long long, and push it * ACCURACY in stack
                if (com & MASK_MEM) {
                    argv_sum = 0;
                    if (com & MASK_REG) {
                        reg_id = code[ip++];
                        argv_sum += REG_LIST[reg_id] / ACCURACY;
                    }
                    if (com & MASK_IMMC) {
                        argv = code[ip++];
                        argv_sum += argv;
                    }
                    stack_push(&stk, &PROC_RAM[argv_sum], &stk_last_err);
                } else {
                    argv_sum = 0;
                    if (com & MASK_REG) {
                        reg_id = code[ip++];
                        argv_sum += REG_LIST[reg_id];
                    }
                    if (com & MASK_IMMC) {
                        argv = code[ip++];
                        argv_sum += argv * ACCURACY;
                    }
                    stack_push(&stk, &argv_sum, &stk_last_err);
                }
                break;
            case POP_COM:
                if (com & MASK_MEM) { // TODO: вынести в отдельную функцию getArg
                    argv_sum = 0;
                    // printf("argv_sum: [%d]\n", argv_sum);
                    if (com & MASK_REG) {
                        reg_id = code[ip++];
                        // printf("reg_id: [%d]\n", reg_id);
                        // printf("reg_list[%d]: [%d]\n", reg_id, REG_LIST[reg_id]);
                        argv_sum += REG_LIST[reg_id] / ACCURACY;
                    }
                    // printf("argv_sum: [%d]\n", argv_sum);
                    if (com & MASK_IMMC) {
                        argv = code[ip++];
                        argv_sum += argv;
                    }
                    // printf("&PROC_RAM[%d]: [%p]\n", argv_sum, &PROC_RAM[argv_sum]);
                    // printf("\n");

                    stack_pop(&stk, &PROC_RAM[argv_sum], &stk_last_err);
                } else {
                    if (com & MASK_REG) {
                        reg_id = code[ip++];
                        stack_pop(&stk, &REG_LIST[reg_id], &stk_last_err);
                    } else {
                        stack_pop(&stk, NULL, &stk_last_err);
                    }
                }
                break;

            case HLT_COM:
                hlt_state = true;
                break;
            default:
                debug("ip = {%d}, unknown command '%d' (tip: if com eq 0, you miss 'hlt')\n", ip, com & filter_mask);
                proc_add_err(return_err, PROC_UNKNOWN_COM);
                break;
        }
        if (*return_err != PROC_ERR_OK) {
            // DEBUG_STK_ERROR(*return_err)
            const size_t bit_len = sizeof(proc_err) * 8;
            char bit_str[bit_len + 1] = {};
            get_bit_str(*return_err, bit_str, bit_len);
            debug("ip: {%d}, proc_error: %s", ip, bit_str);
            stack_destroy(&stk);
            stack_destroy(&call_stk);
            return;
        }



        if (hlt_state) {
            break;
        }
    }

    stack_destroy(&stk);
    stack_destroy(&call_stk);

    return;
}

