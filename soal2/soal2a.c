#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

void main()
{
    int mat1[4][3];
    int mat2[3][6];
    int (*mat3)[6];
    key_t key = 1234;
    int i;
    int j;
    int k;

    int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget() Failed");
        return;
    }

    mat3 = shmat(shmid, NULL, 0);
    if(mat3 == (void *)-1) {
        fprintf(stderr, "shmat() Failed" ); 
        return;
    }

    printf("Input matrix 4x3\n");
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 3; j++)
        {
            scanf("%d", &mat1[i][j]);
        }
    }

    printf("Input matrix 3x6\n");
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 6; j++)
        {
            scanf("%d", &mat2[i][j]);
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            mat3[i][j] = 0;

            for (k = 0; k < 3; k++)
            {
                mat3[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }

    printf("Result\n");
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            printf("%d ", mat3[i][j]);
        }
        printf("\n");
    }

    char ch;
    do
    {
        printf("Type c to close\n");
        scanf(" %c", &ch);
    } while (ch != 'c');

    shmdt(mat3);
    shmctl(shmid, IPC_RMID, NULL);
}
