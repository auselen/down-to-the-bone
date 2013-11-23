typedef struct { int quot; int rem; } idiv_return;

idiv_return __aeabi_idivmod(int num, int denom) {
    idiv_return r;
    int q = 0;

    while (num >= denom) {
        num -= denom;
        q++;
    }

    r.quot = q;
    r.rem = num;

    return r;
}

int __aeabi_idiv(int num, int denom) {
    int q = 0;

    while (num >= denom) {
        num -= denom;
        q++;
    }

   return q;
}

