#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    // Создание дочернего процесса
    pid_t childPid = fork();
    if (childPid > 0) {
        // Ставим родительский процесс на минутную паузу
        sleep(60);
    }
    else {
        //Завершаем работу дочернего процесса
        exit(0);
    }
    return 0;
}
