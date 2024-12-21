#include <stdlib.h>
#include "error_processing.h"
#include "assembler_funcs.h"
#include "asm_args_proc.h"
#include "inc/assembler_funcs.h"

int main(const int argc, const char *argv[]) {
    main_config_t main_config = {};

    opt_data options[] =
    {
        {"-i", "--input", "%s", &main_config.input_file},
        {"-o", "--output", "%s", &main_config.output_file},
    };

    size_t n_options = sizeof(options) / sizeof(opt_data);

    get_options(argc, argv, options, n_options);

    // main_config_print(stdout, &main_config);

    asm_err last_err = ASM_ERR_OK;

    bin_code_t bin_code = bin_code_init();

    asm_code_t asm_code =  asm_code_read(main_config.input_file, &last_err);
    if (last_err != ASM_ERR_OK) {
        DEBUG_ERROR(last_err);
        return EXIT_FAILURE;
    }

    asm_commands_translate(&bin_code, &asm_code, &last_err);

    bin_code_write(main_config.output_file, bin_code, &last_err);
}
