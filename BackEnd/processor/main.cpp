#include <stdlib.h>
#include "general.h"
#include "proc_output.h"
#include "processor_func.h"

#include "proc_err.h"
#include "processor_args_proc.h"
#include "string_funcs.h"

int main(const int argc, const char *argv[]) {
    main_config_t main_config = {};

    opt_data options[] =
    {
        {"-i", "--input", "%s", &main_config.input_file},
    };

    size_t n_options = sizeof(options) / sizeof(opt_data);

    get_options(argc, argv, options, n_options);

    main_config_print(stdout, &main_config);

    proc_err proc_last_err = PROC_ERR_OK;
    proc_data_t proc_data = {};

    if (bin_code_read(main_config.input_file, proc_data.code, &proc_last_err) == -1) { // FIXME: есть баги
        debug("bin_code_read failed");
        return EXIT_FAILURE;
    }
    fprintf_title(stdout, "CODE_EXECUTION", '-', border_size);

    execute_code(&proc_data, &proc_last_err);

    fprintf(stdout, "\n");
    fprintf_title(stdout, "CODE_EXECUTION", '-', border_size);

    return EXIT_SUCCESS;
}
