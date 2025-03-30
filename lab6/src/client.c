#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"

struct Server {
  char ip[255];
  int port;
};

struct ServersSegment {
  struct sockaddr_in server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}

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

uint64_t defSck(struct ServersSegment *args) {
  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    exit(1);
  }

  if (connect(sck, (struct sockaddr *)&args->server, sizeof(args->server)) < 0) {
    fprintf(stderr, "Connection failed\n");
    exit(1);
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &args->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &args->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &args->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    exit(1);
  }

  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Receive failed\n");
    exit(1);
  }

  uint64_t answer = 0;
  memcpy(&answer, response, sizeof(uint64_t));

  close(sck);
  return answer;
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
  char *serv = malloc(255 * sizeof(char));
  while (fgets(serv, 255, file) != NULL) {
    servers_num++;
    to = realloc(to, servers_num * sizeof(struct Server));
    strcpy(to[servers_num - 1].ip, strtok(serv, ":"));
    ConvertStringToUI64(strtok(NULL, "\n"), &to[servers_num - 1].port);
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

    pthread_create(&threads[i], NULL, (void *)defSck, (void *)&serversSegment[i]);
  }

  for (uint64_t i = 0; i < servers_num; i++) {
    uint64_t between_answer = 0;
    pthread_join(threads[i], (void **)&between_answer);
    answer = MultModulo(answer, between_answer, mod);
  }

  printf("answer: %lu\n", answer);

  free(to);

  return 0;
}
