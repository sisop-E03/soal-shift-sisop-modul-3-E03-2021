#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

typedef struct
{
    long long *cell;
    int a;
    int b;
} CalcArgs;

CalcArgs *makeCalcArgs(long long *cell, int a, int b)
{
    CalcArgs *args = malloc(sizeof(CalcArgs));

    args->cell = cell;
    args->a = a;
    args->b = b;

    return args;
}

void *calculate(void *arguments)
{
    CalcArgs *args = (CalcArgs *)arguments;

    if (args->a == 0 || args->b == 0)
    {
        *(args->cell) = 0;
    }
    else
    {
        int i;
        int x = args->a - args->b > 0 ? args->a - args->b : 1;

        *(args->cell) = args->a;
        for (int i = args->a - 1; i > x; i--)
        {
            *(args->cell) *= i;
        }
    }

    return NULL;
}

void main()
{
    int (*matA)[6];
    int matB[4][6];
    long long matC[4][6];
    int i;
    int j;
    key_t key = 1234;

    int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget() Failed");
        return;
    }

    matA = shmat(shmid, NULL, 0);
    if (matA == (void *)-1)
    {
        fprintf(stderr, "shmat() Failed");
        return;
    }

    printf("Matrix A\n");
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            printf("%d ", matA[i][j]);
        }
        printf("\n");
    }

    printf("Input matrix B 4x6\n");
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            scanf("%d", &matB[i][j]);
        }
    }

    pthread_t tid[4][6];

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            pthread_create(&(tid[i][j]), NULL, calculate,
                           (void *)makeCalcArgs(&(matC[i][j]), matA[i][j], matB[i][j]));
        }
    }
 
    printf("Result\n");
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            pthread_join(tid[i][j], NULL);
            printf("%lld ", matC[i][j]);
        }
        printf("\n");
    }

    shmdt(matA);
    shmctl(shmid, IPC_RMID, NULL);
}
