#include <stdio.h>

int main()
{
    int A[100];
    int sum = 0;
    for (int i = 1; i <= 5; i++)
    {
        scanf("%d", &A[i]);
        sum += A[i];
    }
    printf("%d\n", sum / 5);
}