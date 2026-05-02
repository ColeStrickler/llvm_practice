#include <stdio.h>

int foo(int x) {
    if (x > 0)
        return x + 1;
    else
        return x - 1;
}

int main() {
    return foo(0);
}