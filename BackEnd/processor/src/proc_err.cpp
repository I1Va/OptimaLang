#include "proc_err.h"

void proc_add_err(proc_err *src, proc_err add) {
    *src = (proc_err) ((unsigned long long) (*src) | (unsigned long long) (add));
}
