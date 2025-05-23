int rec_factorial(int n) {
    int return_val = 0;
    if (n <= 1) {
        return_val = 1;
    } else {
        return_val = n * rec_factorial(n - 1);
    };
    return return_val;
};

int iter_factorial(int n) {
    int return_val = 1;
    while (n >= 1) {
        return_val = return_val * n;
        n = n - 1;
    };
    return return_val;
};

int main() {
    int n = input();
    print(rec_factorial(n));
    print(iter_factorial(n));
    return 0;
};
