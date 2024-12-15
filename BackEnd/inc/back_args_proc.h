#ifndef FRONT_ARGS_PROC_H
#define FRONT_ARGS_PROC_H

#include <string.h>
#include <stdio.h>

struct opt_data {
    const char *short_name;
    const char *long_name;

    const char *fmt;
    void *val_ptr;

    bool exist;
};

const size_t MAX_OPT_NAME_SZ = 64;
const size_t MAX_CONFIG_NAME_SIZE = 64;

void opt_data_ctor(opt_data *option, const char *const short_name_src, const char *const long_name_src,
    const char *const fmt_src, void *val_ptr_src);

void opt_data_dtor(opt_data *option);

opt_data *option_list_ptr(const char *name, opt_data opts[], const size_t n_opts);

void get_options(const int argc, const char* argv[], opt_data opts[], const size_t n_opts);

// MODES/CONFIG ZONE

struct main_config_t {
    char input_file[MAX_CONFIG_NAME_SIZE];
    char output_file[MAX_CONFIG_NAME_SIZE];
};

void main_config_ctor(main_config_t *conf);

void main_config_dtor(main_config_t *conf);

void main_config_print(FILE *stream, main_config_t *conf);

// #define ASSERT(error, end_instruction)                                                                                    \
//     fprintf_red(stderr, "{%s} [%s: %d]: descr{%s}\n", __FILE_NAME__, __PRETTY_FUNCTION__, __LINE__, get_descr(error));    \
//     end_instruction;

enum arg_err_code {
    ARG_ERR_OK = 0,
    ARG_ERR_NULLPTR = 1, // TODO: подравняй нумерацию
    ARG_ERR_CALLOC = 2,
    ARG_ERR_MEM = 3,
    ARG_ERR_UNKNOWN = 4,
    ARG_ERR_STAT = 5,
    ARG_ERR_INPUT_DATA = 6,
    ARG_ERR_FILE_OPEN = 7,
    ARG_ERR_FILE_CLOSE = 8,
    ARG_ERR_ARGS = 9,
    ARG_ERR_ARG_NOT_EXIST = 10,
};

const char *get_descr(enum arg_err_code err);

#endif // FRONT_ARGS_PROC_H