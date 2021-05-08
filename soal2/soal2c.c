#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void main(int argc, char *argv[]) {
    int fd1[2];

    if (pipe(fd1)==-1) 
	{ 
		fprintf(stderr, "Pipe Failed" ); 
		return; 
	}
	
    pid_t pid = fork();

    if(pid == 0) {
        dup2(fd1[1], STDOUT_FILENO);

        close(fd1[0]);
        close(fd1[1]);

        char *argv[] = {"ps", "aux", NULL};
        execv("/bin/ps", argv);
    }

    int fd2[2];

    if (pipe(fd2)==-1) 
	{ 
		fprintf(stderr, "Pipe Failed" ); 
		return; 
	}

    pid = fork();

    if(pid == 0) {
        dup2(fd1[0], STDIN_FILENO);

        close(fd1[0]);
        close(fd1[1]);

        dup2(fd2[1], STDOUT_FILENO);

        close(fd2[0]);
        close(fd2[1]);

        char *argv[] = {"sort", "-nrk", "3,3", NULL};
        execv("/bin/sort", argv);
    } else if (pid > 0) {
        
        close(fd1[0]);
        close(fd1[1]);

        dup2(fd2[0], STDIN_FILENO);

        close(fd2[0]);
        close(fd2[1]);

        char *argv[] = {"head", "-5", NULL};
        execv("/bin/head", argv);
    }
}