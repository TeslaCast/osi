#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t* currentPid;
int I;
void killer(int arg)
{
    int killed = -1;
printf("%d**",currentPid[I]);
killed = kill(currentPid[I], SIGKILL);
    if (killed == 0)
    {
        printf("Child process was killed\n");
    }
    else
    {
        printf("ERR:: Child process was NOT killed\n");
    }
}

int main(int argc, char** argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    int timeout = -1;
    bool with_files = false;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = { {"seed", required_argument, 0, 0},
                                          {"array_size", required_argument, 0, 0},
                                          {"pnum", required_argument, 0, 0},
					  {"timeout", required_argument, 0, 0},
                                          {"by_files", no_argument, 0, 'f'},
                                          {0, 0, 0, 0} };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
        case 0:
            switch (option_index) {
            case 0:
                seed = atoi(optarg);
                if (seed <= 0)
                {
                    printf("seed is a positive number\n\n");
                    return 1;
                }
                break;
            case 1:
                array_size = atoi(optarg);
                if (array_size <= 0)
                {
                    printf("array_size is a positive number\n");
                    return 1;
                }
                break;
            case 2:
                pnum = atoi(optarg);
                if (array_size <= 0)
                {
                    printf("pnum is a positive number\n");
                    return 1;
                }
                break;
            case 3:
                timeout = atoi(optarg);
                if (timeout <= 0)
                {
                    printf("timeout is a positive number\n");
                    return 1;
                }

            case 4:
                with_files = true;
                break;

            defalut:
                printf("Index %d is out of options\n", option_index);
            }
            break;
        case 'f':
            with_files = true;
            break;

        case '?':
            break;

        default:
            printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (optind < argc) {
        printf("Has at least one no option argument\n");
        return 1;
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
            argv[0]);
        return 1;
    }

    int* array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    /*int i;
    for(i = 0; i< array_size; i++)
    printf("%d  ",array[i]);
    printf("\n");*/

    int active_child_processes = 0;

    struct timeval start_time;
    gettimeofday(&start_time, NULL);



    int pipefd[2];
    pipe(pipefd);
    int numSegment = array_size / pnum;

currentPid = malloc(sizeof(int) * pnum);
    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
currentPid[i] = child_pid;
        if (child_pid >= 0) {
            // successful fork
            active_child_processes += 1;
            if (child_pid == 0) {
                // child process
                // parallel somehow
                struct MinMax myMinMax;
                if (i != pnum - 1)
                {
                    myMinMax = GetMinMax(array, i * numSegment, (i + 1) * numSegment);
                }
                else  myMinMax = GetMinMax(array, i * numSegment, array_size);
                if (with_files) {
                    // use files here
                    FILE* file = fopen("threads.txt", "a");
                    fwrite(&myMinMax, sizeof(struct MinMax), 1, file);
                    fclose(file);
                }
                else {
                    // use pipe here
                    write(pipefd[1], &myMinMax, sizeof(struct MinMax));
                }
                return 0;
            }

        }
        else {
            printf("Fork failed!\n");
            return 1;
        }
    }


for(int i =0; i< pnum;i++)
{
I=i;
        if (timeout != -1)
        {alarm(timeout);
	    signal(SIGALRM, killer);
sleep(10);
        }
}

    while (active_child_processes > 0) {
        // your code here

        close(pipefd[1]);
        wait(NULL);
        active_child_processes -= 1;
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        struct MinMax myMinMax;

        if (with_files) {
            // read from files
            FILE* file = fopen("threads.txt", "r");
            fseek(file, i * sizeof(struct MinMax), SEEK_SET);
            fread(&myMinMax, sizeof(struct MinMax), 1, file);
            //printf("pnum:%i:\tmin:%7i     max:%i\n", i, minMaxBuff.min, minMaxBuff.max);
            fclose(file);
        }
        else {
            // read from pipes
            read(pipefd[0], &myMinMax, sizeof(struct MinMax));
        }
        min = myMinMax.min; max = myMinMax.max;

        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}
