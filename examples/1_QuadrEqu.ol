int quadr_equ_solver() {
    int a = input();
    int b = input();
    int c = input();
    if (a == 0) {
        if (b == 0) {
            if (c == 0) {
                print_string("infinity_roots");
            } else {
                print_string("zero_roots");
            };
        } else {
            print(1);
        };
    } else {
        int D = b * b - 4 * a * c;
        if (D >= 0) {
            if (D == 0) {
                print_string("one_root:");
                print(-1 * b / (2 * a));
            } else {
                float D_sqrt = sqrt(D);
                print_string("two_roots:");
                print((-1 * b + D_sqrt) / (2 * a));
                print((-1 * b - D_sqrt) / (2 * a));
            };
        } else {
            print_string("zero_roots");
        };
    };

    return 0;
};

int main() {
    quadr_equ_solver();
    return 0;
};
