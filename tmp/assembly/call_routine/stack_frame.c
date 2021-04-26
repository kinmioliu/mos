#include <stdio.h>

int add(int a, int b)
{
    int c = a;
    int d = b;
    int sum = c + d + 5;
    return sum;
}

int main(int argc, char *argv[])
{
    int a = 4;
    int b = 7;
    int sum = 0;
    sum = add(a, b);
    printf("sum :%d\n", sum);
    return 0;
}

