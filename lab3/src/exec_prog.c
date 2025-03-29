#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 


int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s seed arraysize\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // В дочернем процессе
        execl("./sequential_min_max", "./sequential_min_max", argv[1], argv[2], NULL);
        perror("exec failed");
        return 1;
    } else if (pid > 0) {
        // В родительском процессе
        wait(NULL);
        printf("Child process finished.\n");
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}
