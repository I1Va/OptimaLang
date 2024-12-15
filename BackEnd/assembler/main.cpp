#include <stdlib.h>
#include "error_processing.h"
#include "assembler_funcs.h"
#include "asm_args_proc.h"

int main(const int argc, const char *argv[]) {
    main_config_t main_config = {};

    opt_data options[] =
    {
        {"-i", "--input", "%s", &main_config.input_file},
        {"-o", "--output", "%s", &main_config.output_file},
    };

    size_t n_options = sizeof(options) / sizeof(opt_data);

    get_options(argc, argv, options, n_options);

    main_config_print(stdout, &main_config);

    asm_err last_err = ASM_ERR_OK;

    asm_data_t asm_data = {}; asm_data_t_ctor(&asm_data);

    asm_code_read(&asm_data.asm_code, main_config.input_file, &last_err);
    if (last_err != ASM_ERR_OK) {
        DEBUG_ERROR(last_err);
        return EXIT_FAILURE;
    }

    asm_commands_translate(&asm_data, &last_err);

    bin_code_write(main_config.output_file, asm_data.bin_code, &last_err);
}
