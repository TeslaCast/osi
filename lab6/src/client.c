#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "MultModulo.h"

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

struct Server {
  char ip[255];
  uint64_t port;
};

struct ServersSegment {
  struct sockaddr_in server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

void *defSck(void *args) {
  struct ServersSegment *serverSegment = (struct ServersSegment *)args;
  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    pthread_exit(NULL);
  }

  if (connect(sck, (struct sockaddr *)&serverSegment->server, sizeof(serverSegment->server)) < 0) {
    fprintf(stderr, "Connection failed\n");
    close(sck);
    pthread_exit(NULL);
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &serverSegment->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &serverSegment->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &serverSegment->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    close(sck);
    pthread_exit(NULL);
  }

  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Receive failed\n");
    close(sck);
    pthread_exit(NULL);
  }

  uint64_t answer = 0;
  memcpy(&answer, response, sizeof(uint64_t));

  close(sck);
  uint64_t *result = malloc(sizeof(uint64_t));
  *result = answer;
  return result;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'};

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                       {"mod", required_argument, 0, 0},
                                       {"servers", required_argument, 0, 0},
                                       {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        if (!ConvertStringToUI64(optarg, &k)) {
          printf("k is a positive number\n");
          return 1;
        }
        break;
      case 1:
        if (!ConvertStringToUI64(optarg, &mod)) {
          printf("mod is a positive number\n");
          return 1;
        }
        break;
      case 2:
        strcpy(servers, optarg);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Usage: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
    return 1;
  }

  unsigned int servers_num = 0;
  struct Server *to = NULL;
  FILE *file = fopen(servers, "r");
  if (!file) {
    fprintf(stderr, "Failed to open servers file\n");
    return 1;
  }

  char *serv = malloc(255 * sizeof(char));
  while (fgets(serv, 255, file) != NULL) {
    servers_num++;
    to = realloc(to, servers_num * sizeof(struct Server));
    char *ip = strtok(serv, ":");
    char *port_str = strtok(NULL, "\n");
    if (ip && port_str) {
      strcpy(to[servers_num - 1].ip, ip);
      ConvertStringToUI64(port_str, &to[servers_num - 1].port);
    }
  }
  fclose(file);
  free(serv);

  uint64_t segments = k / servers_num;
  uint64_t answer = 1;

  struct ServersSegment serversSegment[servers_num];
  pthread_t threads[servers_num];
  for (int i = 0; i < servers_num; i++) {
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    serversSegment[i].server.sin_family = AF_INET;
    serversSegment[i].server.sin_port = htons((uint16_t)to[i].port);
    serversSegment[i].server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    serversSegment[i].begin = 1 + i * segments;
    serversSegment[i].end = ((i != servers_num - 1) ? (i + 1) * segments : k);
    serversSegment[i].mod = mod;

    pthread_create(&threads[i], NULL, defSck, (void *)&serversSegment[i]);
  }

  for (uint64_t i = 0; i < servers_num; i++) {
    uint64_t *between_answer;
    pthread_join(threads[i], (void **)&between_answer);
    answer = multModulo(answer, *between_answer, mod);
    free(between_answer);
  }

  printf("answer: %lu\n", answer);

  free(to);

  return 0;
}
