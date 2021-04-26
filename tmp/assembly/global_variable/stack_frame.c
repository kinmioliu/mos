#include <stdio.h>
int c = 0;
int d = 0;

int add(int a, int b)
{
    c = a;
    d = b;
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

