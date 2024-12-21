#include <cstddef>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "front_args_proc.h"
#include "general.h"

opt_data *option_list_ptr(const char *name, opt_data opts[], const size_t n_opts) {
    for (size_t i = 0; i < n_opts; i++) {
        if (strcmp(name, opts[i].short_name) == 0 || strcmp(name, opts[i].long_name) == 0) {
            return &opts[i];
        }
    }
    return NULL;
}

void get_options(const int argc, const char* argv[], opt_data opts[], const size_t n_opts) {
    assert(argc >= 0);

    for (int i = 1; i < argc; i++) {
        char name[MAX_OPT_NAME_SZ];
        char value[MAX_OPT_NAME_SZ];
        sscanf(argv[i], "%[^=]%s", name, value);

        opt_data *ptr = option_list_ptr(name, opts, n_opts);

        if (ptr != NULL) {
            sscanf(value + 1, (ptr->fmt), ptr->val_ptr); // FIXME: исправить warning. Мб использовать __atribute__
            ptr->exist = true;
        }
    }
}

// // MODES/CONFIG ZONE

void main_config_get(main_config_t *main_config, const int argc, const char *argv[]) {
    opt_data options[] =
    {
        {"-i", "--input", "%s",  &main_config->input_file},
        {"-o", "--output", "%s", &main_config->output_file},
    };

    size_t n_options = sizeof(options) / sizeof(opt_data);

    get_options(argc, argv, options, n_options);
}

void main_config_print(FILE *stream, main_config_t *conf) {
    fprintf_red(stream, RED "main_config_t: \n");
    fprintf(stream, "conf_name: %s\n", conf->input_file);
    fprintf(stream, "conf_name: %s\n", conf->output_file);
}

#define DESCR_(code) case code : return #code;

const char *get_descr(enum arg_err_code err) {
    switch (err) {
        DESCR_(ARG_ERR_OK)
        DESCR_(ARG_ERR_UNKNOWN)
        DESCR_(ARG_ERR_CALLOC)
        DESCR_(ARG_ERR_NULLPTR)
        DESCR_(ARG_ERR_STAT)
        DESCR_(ARG_ERR_INPUT_DATA)
        DESCR_(ARG_ERR_MEM)
        DESCR_(ARG_ERR_FILE_CLOSE)
        DESCR_(ARG_ERR_FILE_OPEN)
        DESCR_(ARG_ERR_ARGS)
        DESCR_(ARG_ERR_ARG_NOT_EXIST)
        default: return "VERY STRANGE ERROR:(";
    }
}
#undef DESCR_

