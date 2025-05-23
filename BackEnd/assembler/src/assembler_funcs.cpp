#include <assert.h>
#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "assembler_funcs.h"
#include "error_processing.h"
#include "general.h"
#include "string_funcs.h"

const int DEFAULT_label_VALUE = -1;

struct com_t {
    const char com_name[max_com_sz];
    enum asm_coms_nums com_num;
    void (*write_func)(bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err);
};

struct reg_t
{
    int id;
    const char *name;
};

struct label_t
{
    int addr;
    char name[max_label_name_sz];
};

struct fix_up_obj_t
{
    size_t label_bin_code_idx;
    // FIXME: add label_number = ..
    char label_name[max_label_name_sz]; // FIXME: делать поиск метки по номерк, а не по названию
};

size_t empty_fix_up_idx = 0;
fix_up_obj_t fix_up_table[fix_up_table_max_sz] = {};

size_t empty_label_idx = 0;
label_t label_list[label_list_max_sz] = {};

reg_t reg_list[] =
{
    {0, "rbp"},
    {1, "rsp"},
    {2, "rax"},
    {3, "rbx"},
};

const size_t reg_list_sz = sizeof(reg_list) / sizeof(reg_t);

asm_code_t asm_code_init() {
    asm_code_t asm_code = {};

    asm_code.asm_idx = 0;
    asm_code.code_sz = max_asm_commands_n;
    return asm_code;
}

bin_code_t bin_code_init() {
    bin_code_t bin_code = {};

    bin_code.bin_idx = 0;

    return bin_code;
}


int get_reg_id(const char name[]) {
    for (size_t reg_idx = 0; reg_idx < reg_list_sz; reg_idx++) {
        if (strcmp(reg_list[reg_idx].name, name) == 0) {
            return reg_list[reg_idx].id;
        }
    }
    return -1;
}

void label_list_dump(FILE *stream) {
    fprintf_title(stream, "label_LIST", '-', STR_F_BORDER_SZ);
    fprintf(stream, "empty_label_idx: {%lu}\n", empty_label_idx);
    for (size_t label_idx = 0; label_idx < empty_label_idx; label_idx++) {
        fprintf(stream, "label[%2lu]: '%10s' addr = %d\n", label_idx, label_list[label_idx].name, label_list[label_idx].addr);
    }
    fprintf_border(stream, '-', STR_F_BORDER_SZ, true);
}

void bin_code_dump(FILE *stream, bin_code_t bin_code) {
    fprintf_title(stream, "BIN_CODE", '-', STR_F_BORDER_SZ);
    fprintf(stream, "sz: {%lu}, last_idx: {%lu}\n", bin_code.bin_idx, bin_code.bin_idx);
    size_t chars_cnt = 0;
    for (size_t idx = 0; idx < bin_code.bin_idx; idx++) {
        if (chars_cnt >= STR_F_BORDER_SZ) {
            fprintf(stream, "\n");
            chars_cnt = 0;
        }
        fprintf(stream, "%d ", bin_code.code[idx]);
        const size_t buf_sz = 15;
        char com_str[buf_sz];  // буфер, в которую запишем число
        snprintf(com_str, buf_sz, "%d", bin_code.code[idx]);
        chars_cnt += strlen(com_str) + 1;
    }
    fprintf(stream, "\n");
    fprintf_border(stream, '-', STR_F_BORDER_SZ, true);
}

void fill_label_list(int default_addr) {
    for (size_t label_idx = 0; label_idx < label_list_max_sz; label_idx++) {
        label_list[label_idx].addr = default_addr;
    }
}

int add_label_to_list(int addr, const char *name) {
    assert(empty_label_idx < label_list_max_sz);
    assert(name != NULL);

    label_list[empty_label_idx].addr = addr;
    strcpy(label_list[empty_label_idx].name, name);
    empty_label_idx++;

    return (int) empty_label_idx - 1;
}

int get_label_addr_from_list(const char *name) {
    assert(name != NULL);

    for (size_t label_idx = 0; label_idx < empty_label_idx; label_idx++) {
        if (strcmp(label_list[label_idx].name, name) == 0) {
            return (int) label_list[label_idx].addr;
        }
    }
    return -1;
}

void fix_up_table_add(const size_t label_bin_code_idx, const char name[]) {
    assert(empty_fix_up_idx < fix_up_table_max_sz);

    fix_up_table[empty_fix_up_idx].label_bin_code_idx = label_bin_code_idx;
    strcpy(fix_up_table[empty_fix_up_idx].label_name, name);
    empty_fix_up_idx++;
}

void fix_up_table_pull_up(bin_code_t *bin_code, asm_err *return_err) {
    for (size_t fix_idx = 0; fix_idx < empty_fix_up_idx; fix_idx++) {
        int label_addr = get_label_addr_from_list(fix_up_table[fix_idx].label_name);
        if (label_addr == -1) {
            asm_add_err(return_err, ASM_ERR_INVALID_LABEL);
            DEBUG_ERROR(ASM_ERR_INVALID_LABEL);
            debug("Invalid label : '%s'", fix_up_table[fix_idx].label_name);
        }
        bin_code->code[fix_up_table[fix_idx].label_bin_code_idx] = label_addr;
    }
}

bool check_label_elem(const char com[]) {
    int len_rec_com = 0;
    char label[max_label_name_sz] = {};
    sscanf(com, "%s%n", label, &len_rec_com);
    if (label[len_rec_com - 1] == ':') {
        return true;
    }
    return false;
}

void process_label(bin_code_t *bin_code, asm_code_t *asm_code, asm_err *return_err)
{
    assert(bin_code != NULL);
    assert(asm_code != NULL);
    assert(return_err != NULL);

    if (asm_code->asm_idx >= asm_code->code_sz) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(*return_err)
        debug("bin_code overflow. Label isn't exist");
        return;
    }
    if (check_label_elem(asm_code->code[asm_code->asm_idx])) {
        char label_name[max_label_name_sz] = {};

        sscanf(asm_code->code[asm_code->asm_idx++], "%s", label_name);
        int label_addr = get_label_addr_from_list(label_name);
        if (label_addr == -1) {
            fix_up_table_add(bin_code->bin_idx++, label_name);
            return;
        }
        bin_code->code[bin_code->bin_idx++] = label_addr;
    }
}

void write_register(bin_code_t *bin_code, const char register_str[], asm_err *return_err)
{
    assert(bin_code != NULL);
    assert(register_str != NULL);

    int reg_id = get_reg_id(register_str);

    if (reg_id == -1) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(*return_err)
        debug("invalid register name {%s}]", register_str);
        return;
    }

    bin_code->code[bin_code->bin_idx++] = reg_id;
}


struct arg_t {
    int arg_mask;
    int immc;
    char reister_str[register_max_sz];
    bool err;
};

arg_t parse_push_arg(const char arg_str[]) {
    int scanned_chars = 0;
    arg_t argv = {};
    // TODO: упростить парсинг, сначала проверять на [], потом на аргументы
    if (sscanf(arg_str, " [ %d ] %n", &argv.immc, &scanned_chars) == 1 && scanned_chars != 0) {
        argv.arg_mask |= MASK_MEM | MASK_IMMC;
        return argv;
    }
    argv = {};

    if (sscanf(arg_str, " [ %[^] +] ] %n", argv.reister_str, &scanned_chars) == 1 && scanned_chars != 0) {
        argv.arg_mask |= MASK_MEM + MASK_REG;
        return argv;
    }
    argv = {};

    if (sscanf(arg_str, " [ %[^] +] + %d ] %n", argv.reister_str, &argv.immc, &scanned_chars) == 2 && scanned_chars != 0) {
        argv.arg_mask |= MASK_MEM | MASK_IMMC | MASK_REG;
        return argv;
    }
    argv = {};

    if (sscanf(arg_str, "%d%n", &argv.immc, &scanned_chars) == 1) {
        argv.arg_mask |= MASK_IMMC;
        return argv;
    }
    argv = {};

    if (sscanf(arg_str, " %[^ +] + %d %n", argv.reister_str, &argv.immc, &scanned_chars) == 2) {
        argv.arg_mask |= MASK_REG | MASK_IMMC;
        return argv;
    }
    argv = {};

    if (sscanf(arg_str, " %s %n", argv.reister_str, &scanned_chars) == 1) {
        argv.arg_mask |= MASK_REG;
        return argv;
    }
    argv = {};

    argv.err = true;
    return argv;
}

void fprintf_bin(FILE *stream, int mask) {
    fprintf(stream, "bin[%d]: '", mask);
    while (mask > 0) {
        fprintf(stream, "%d", mask % 2);
        mask /= 2;
    }
    fprintf(stream, "'\n");
}

void write_universal_push(bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err) {
    assert(bin_code != NULL);
    assert(asm_code != NULL);
    assert(return_err != NULL);

    asm_code->asm_idx++;

    if (asm_code->asm_idx >= asm_code->code_sz) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(*return_err)
        debug("push hasn't required arg {%d} {%d}", asm_code->asm_idx, asm_code->code_sz);
        return;
    }

    arg_t argv = parse_push_arg(asm_code->code[asm_code->asm_idx++]); // REG IMMC

    if (argv.err) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(ASM_ERR_SYNTAX);
        debug("Invalid arg: '%s'", asm_code->code[asm_code->asm_idx]);
        return;
    }
    // printf("immc_argv: {%d}, reg_argv: {%s}, mem: {%d}\n", argv.immc, argv.reister_str, argv.arg_mask);

    // fprintf_bin(stdout, argv.arg_mask);
    // fprintf_bin(stdout, MASK_IMMC);
    // fprintf_bin(stdout, (argv.arg_mask & MASK_IMMC));

    bin_code->code[bin_code->bin_idx++] = PUSH_COM | argv.arg_mask;
    if (argv.arg_mask & MASK_REG) {
        write_register(bin_code, argv.reister_str, return_err);
    }
    if (argv.arg_mask & MASK_IMMC) {
        bin_code->code[bin_code->bin_idx++] = argv.immc;
    }
}

void write_universal_pop(bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err) {
    assert(bin_code != NULL);
    assert(asm_code != NULL);
    assert(return_err != NULL);

    asm_code->asm_idx++;

    if (asm_code->asm_idx >= asm_code->code_sz) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(*return_err)
        debug("pop hasn't required arg {%d} {%d}", asm_code->asm_idx, asm_code->code_sz);
        return;
    }

    arg_t argv = parse_push_arg(asm_code->code[asm_code->asm_idx++]); // REG IMMC

    if (argv.err) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(ASM_ERR_SYNTAX);
        debug("Invalid arg: '%s'", asm_code->code[asm_code->asm_idx]);
        return;
    }

    //printf("immc_argv: {%d}, reg_argv: {%s}, mem: {%d}\n", argv.immc, argv.reister_str, argv.arg_mask);
    // fprintf_bin(stdout, argv.arg_mask);
    // fprintf_bin(stdout, MASK_IMMC);
    // fprintf_bin(stdout, (argv.arg_mask & MASK_IMMC));

    bin_code->code[bin_code->bin_idx++] = POP_COM | argv.arg_mask;
    if (((argv.arg_mask & MASK_REG) && (argv.arg_mask & MASK_IMMC) && !(argv.arg_mask & MASK_MEM)) || \
        (!(argv.arg_mask & MASK_REG) && (argv.arg_mask & MASK_IMMC) && !(argv.arg_mask & MASK_MEM))) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        debug("pop invalid argv. (pop can't process only imc or imc+reg without mem)");
        DEBUG_ERROR(ASM_ERR_SYNTAX)
    }
    if (argv.arg_mask & MASK_REG) {
        write_register(bin_code, argv.reister_str, return_err);
    }
    if (argv.arg_mask & MASK_IMMC) {
        bin_code->code[bin_code->bin_idx++] = argv.immc;
    }
}

bool asm_end_idx(const asm_code_t *asm_code) {
    return asm_code->asm_idx >= asm_code->code_sz;
}

void write_jump(bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err)
{
    assert(bin_code != NULL);
    assert(asm_code != NULL);
    assert(return_err != NULL);

    bin_code->code[bin_code->bin_idx++] = JMP_COM;
    asm_code->asm_idx++;

    if (asm_code->asm_idx >= asm_code->code_sz) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(*return_err)
        debug("JMP hasn't required arg");
        return;
    }

    process_label(bin_code, asm_code, return_err);
}

void write_label(bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err)
{
    char label_name[max_label_name_sz] = {};

    sscanf(asm_code->code[asm_code->asm_idx++], "%s", label_name);
    if (get_label_addr_from_list(label_name) != -1) {
        *return_err = ASM_ERR_SYNTAX;
        DEBUG_ERROR(ASM_ERR_SYNTAX);
        debug("label redefenition: '%s'", label_name);
        return;
    }

    add_label_to_list((int) bin_code->bin_idx, label_name);
    bin_code->code[bin_code->bin_idx++] = LABEL_COM;
}

void write_conditional_jmp(bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err)
{
    bin_code->code[bin_code->bin_idx++] = com_num;
    asm_code->asm_idx++;

    process_label(bin_code, asm_code, return_err);
}

void write_simple_com(bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err)
{
    bin_code->code[bin_code->bin_idx++] = com_num;
    asm_code->asm_idx++;
}

void write_call_com(bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err)
{
    bin_code->code[bin_code->bin_idx++] = CALL_COM;
    asm_code->asm_idx++;

    if (asm_code->asm_idx >= asm_code->code_sz) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(*return_err)
        debug("CALL hasn't required arg");
        return;
    }

    if (check_label_elem(asm_code->code[asm_code->asm_idx])) {
        char *label_name = asm_code->code[asm_code->asm_idx++];

        // FIXME: вместо sscanf можно strncpy
        int label_addr = get_label_addr_from_list(label_name);
        if (label_addr == -1) {
            fix_up_table_add(bin_code->bin_idx++, label_name);
            return;
        }

        bin_code->code[bin_code->bin_idx++] = label_addr;
        return;
    }

    // char *end_ptr = NULL;
    // int num = (int) strtol(asm_commands[*bin_idx], &end_ptr, 10);
    // if (*end_ptr != '\0') {
    //     asm_add_err(return_err, ASM_ERR_SYNTAX);
    //     DEBUG_ERROR(ASM_ERR_SYNTAX);
    //     debug("Can't convert command arg to int. Arg: '%s'", asm_commands[*bin_idx - 1]);
    //     return;
    // }
    // bin_code[*bin_idx] = num;
    // (*bin_idx)++;
}

com_t asm_com_list[] =
{
    {"in"    , IN_COM, write_simple_com},
    {"outc"   , OUTC_COM, write_simple_com},
    {"out"   , OUT_COM, write_simple_com},
    {"add"   , ADD_COM, write_simple_com},
    {"sub"   , SUB_COM, write_simple_com},
    {"mult"  , MULT_COM, write_simple_com},
    {"jmp"   , JMP_COM, write_jump},
    {"ja"    , JA_COM, write_conditional_jmp},
    {"jae"   , JAE_COM, write_conditional_jmp},
    {"jb"    , JB_COM, write_conditional_jmp},
    {"jbe"   , JBE_COM, write_conditional_jmp},
    {"je"    , JE_COM, write_conditional_jmp},
    {"jne"   , JNE_COM, write_conditional_jmp},
    {"hlt"   , HLT_COM, write_simple_com},
    {"call"  , CALL_COM, write_call_com},
    {"ret"   , RET_COM, write_simple_com},
    {"draw"  , DRAW_COM, write_simple_com},
    {"div"   , DIV_COM, write_simple_com},
    {"sqrt"  , SQRT_COM, write_simple_com},
    {"less"  , LESS_COM, write_simple_com},
    {"lesseq", LESSEQ_COM, write_simple_com},
    {"more"  , MORE_COM, write_simple_com},
    {"moreeq", MOREEQ_COM, write_simple_com},
    {"eq"    , EQ_COM, write_simple_com},
    {"push" , PUSH_COM, write_universal_push},
    {"pop" , POP_COM, write_universal_pop},

    {"LABEL:", LABEL_COM, write_label}
};






void asm_com_launch(int asm_com_idx, bin_code_t *bin_code, asm_code_t *asm_code, const enum asm_coms_nums com_num, asm_err *return_err)
{
    assert(bin_code != NULL);
    assert(asm_code != NULL);
    assert(return_err != NULL);

    asm_coms_nums asm_com_num = asm_com_list[asm_com_idx].com_num;
    asm_com_list[asm_com_idx].write_func(bin_code, asm_code, com_num, return_err);
}

const size_t asm_com_list_sz = sizeof(asm_com_list) / sizeof(com_t);

int get_bin_idx_from_list(const char name[]) {
    assert(name != NULL);

    for (size_t bin_idx = 0; bin_idx < asm_com_list_sz; bin_idx++) {
        if (strcmp(name, asm_com_list[bin_idx].com_name) == 0) {
            return (int) bin_idx;
        }
    }
    if (check_label_elem(name)) {
        return (int) asm_com_list_sz - 1;
    }
    return -1;
}

bool asm_getline(FILE* stream, char line[], const size_t n) {
    assert(stream != NULL);
    assert(line != NULL);
    assert(n != 0);

    size_t idx = 0;
    bool EOF_state = true;
    bool write_state = true;

    while (int cur = fgetc(stream)) {
        if (idx >= n || cur == EOF || cur == '\n') {
            return !EOF_state || (cur == '\n');
        }

        if (cur == ';') {
            write_state = false;
        }

        if (write_state) {
            line[idx++] = (char) cur;
        }

        EOF_state = false;

    }
    return write_state;
}

void fprint_asm_commands_list(FILE *stream, asm_code_t *asm_code) {
    fprintf_title(stream, "COMMANDS_LIST", '-', STR_F_BORDER_SZ);
    size_t line_content_len = 0;

    for (size_t bin_idx = 0; bin_idx < asm_code->code_sz; bin_idx++) {
        size_t cur_len = strlen(asm_code->code[bin_idx]);
        if (cur_len + line_content_len > STR_F_BORDER_SZ) {
            fputc('\n', stream);
            line_content_len = 0;
        }
        line_content_len += cur_len + 5;
        fprintf(stream, "'%s'; ", asm_code->code[bin_idx]);
    }

    fputc('\n', stream);
    fprintf_border(stream, '#', STR_F_BORDER_SZ, true);
}

asm_code_t asm_code_read(const char path[], asm_err *return_err) {
    asm_code_t asm_code = asm_code_init();

    FILE *asm_file_ptr = fopen(path, "r");

    size_t asm_temp_idx = 0;

    if (asm_file_ptr == NULL) {
        asm_add_err(return_err, ASM_ERR_FILE_OPEN);
        DEBUG_ERROR(ASM_ERR_FILE_OPEN);
        CLEAR_MEMORY(exit_mark)
    }

    while (1) {
        char line[max_asm_line_sz] = {};
        char *cur_line_ptr = line;

        if (!asm_getline(asm_file_ptr, line, max_asm_line_sz)) {
            break;
        }

        int delta_ptr = 0;
        while (1) {
            sscanf(cur_line_ptr, "%s%n", asm_code.code[asm_temp_idx], &delta_ptr);
            if (!delta_ptr) {
                break;
            }
            if (delta_ptr >= (int) max_asm_command_size) {
                asm_add_err(return_err, ASM_ERR_SYNTAX);
                DEBUG_ERROR(ASM_ERR_SYNTAX)
                debug("parsed command size is larger than max_asm_command_size. Command : {%s}, Max_asm_com_sz: {%lu}", cur_line_ptr, max_asm_command_size);
                CLEAR_MEMORY(exit_mark)
            }

            asm_temp_idx++;
            cur_line_ptr += delta_ptr;
            delta_ptr = 0;
        }
    }

    fclose(asm_file_ptr);
    asm_code.code_sz = asm_temp_idx;

    return asm_code;

    exit_mark:
    if (asm_file_ptr != NULL) {
        fclose(asm_file_ptr);
    }
    return asm_code;
}



void asm_commands_translate(bin_code_t *bin_code, asm_code_t *asm_code, asm_err *return_err)
{
    assert(bin_code != NULL);
    assert(asm_code != NULL);
    assert(return_err != NULL);

    fill_label_list(DEFAULT_label_VALUE);

    // fprint_asm_commands_list(stdout, asm_code);


    while (asm_code->asm_idx < asm_code->code_sz) {
        // printf("before com[%lu]: '%s'\n", bin_code->bin_idx, asm_code->code[asm_code->asm_idx]);
        // label_list_dump(stdout);
        // printf("\n");
        // char v = '@';
        // printf("com: '%s', bin_idx: {%d}, asm_idx: {%d}\n", asm_code->code[asm_code->asm_idx], bin_code->bin_idx, asm_code->asm_idx);
        int asm_com_idx = get_bin_idx_from_list(asm_code->code[asm_code->asm_idx]);
        asm_coms_nums com_num = asm_com_list[asm_com_idx].com_num;
        if (asm_com_idx == -1) {
            asm_add_err(return_err, ASM_ERR_SYNTAX);
            debug("Unknown command: '%s'", asm_code->code[asm_code->asm_idx]);
            DEBUG_ERROR(ASM_ERR_SYNTAX);
            return;
        }
        asm_com_launch(asm_com_idx, bin_code, asm_code, com_num, return_err);
        if (*return_err != ASM_ERR_OK) {
            debug("Prosesing error: '%s'", asm_code->code[asm_code->asm_idx]);
            DEBUG_ERROR(ASM_ERR_SYNTAX);
            return;
        }
        if (com_num == HLT_COM) {
            break;
        }
    }

    // IMPORTANT
    fix_up_table_pull_up(bin_code, return_err);
    if (*return_err != ASM_ERR_OK) {
        DEBUG_ERROR(*return_err);
        return;
    }
}

void bin_code_write(const char path[], bin_code_t bin_code, asm_err *return_err) {
    // bin_code_dump(stdout, bin_code);
    FILE *bin_code_file_ptr = fopen(path, "wb");
    if (bin_code_file_ptr == NULL) {
        asm_add_err(return_err, ASM_ERR_SYNTAX);
        DEBUG_ERROR(ASM_ERR_FILE_OPEN);
        return;
    }

    fwrite(bin_code.code, sizeof(bin_code.code[0]), bin_code.bin_idx, bin_code_file_ptr); // FIXME: делать обработку ошибок write
}