#include <stdio.h>

int factorial(int x) {
    if (x <= 0)
        return 1;
    return x * factorial(x-1);
}

int main() {
    int* data = new int[4096];

    int res = 0;
    for (int i = 0; i < 4096; i += 8)
    {
        res += data[i];
       // printf("addr data 0x%llx\n", &data[i]);
    }

    return res;
}