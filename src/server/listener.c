#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../common/constants.h"

#ifndef GAME_EXEC_NAME
    #error "non e' stato possibile completare la compilazione in quanto il path dell'eseguibile del gioco non e' stato definito correttamente"
#endif

void *init_game(void *num_player){
  if(fork() == 0){
    char argv[2];
    sprintf(argv, "%d", *(int *)num_player);
    execl("./game", "./game", &argv, NULL);
    exit(EXIT_FAILURE);
  }
  pthread_exit(NULL);
}

int setup_listening_socket(int no_port, int queue_len){
    int sockfd = -1;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("inizializzazione listening socket fallita\n");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(no_port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("inizializzazione listening socket fallita\n");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, queue_len);
    return sockfd;
}

void wait_game_players(int num_player, int *players_fd, int sockfd){
    socklen_t clilen;
    int num_conn = 0;
    while (num_conn < num_player) {
      players_fd[num_conn] = accept(sockfd, NULL, NULL);
      if (players_fd[num_conn] < 0){
          printf("errore durante la connessione di un giocatore alla partita\n");
          exit(EXIT_FAILURE);
      }

      write(players_fd[num_conn], &num_conn, sizeof(int));
      num_conn++;
      if (num_conn < (num_player-1)) {
        for(int i = 0; i < num_conn; i++){
          int n = write(players_fd[i], MSG_WAIT_FOR_PLAYERS, strlen(MSG_WAIT_FOR_PLAYERS));
          if (n < 0){
              printf("errore durante la connessione di un giocatore alla partita\n");
              exit(EXIT_FAILURE);
          }
        }
      }
    }

}

void store_as_env(int num_player, int *players_fd){
    for(int i = 0; i < num_player; i++){
      char name[20];
      char fd[20] = {0};
      sprintf(name, "%d", i);
      sprintf(fd, "%d", players_fd[i]);
      setenv(name, fd, 1);
    }

}

void init_game_creation_thread(int thread_arg){
  pthread_t thread;
  int *arg = malloc(sizeof(int));
  *arg = thread_arg;
  int result = pthread_create(&thread, NULL, init_game, (void *)arg);
  if(result) {
    printf("errore durante l'inizializzazione del thread di creazione partita\n");
    exit(EXIT_FAILURE);
  }
  pthread_join(thread, NULL);
  free(arg);
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
      printf("utilizzo: listener <DIMENSIONE PARTITE: 2 o 4>\n");
      exit(EXIT_FAILURE);
  }
  int num_player = *argv[1] - '0';
  int no_port;
  switch(num_player){
      case 2:
          no_port = 2222;
          break;
      case 4:
          no_port = 4444;
          break;
      default:
          printf("dimensione partite invalida, dimensioni valide: 2 o 4\n");
          exit(EXIT_FAILURE);
  }

  int sockfd = setup_listening_socket(no_port, num_player);

  while(1){
    int players_fd[num_player];
    wait_game_players(num_player, players_fd, sockfd);
    store_as_env(num_player, players_fd);
    init_game_creation_thread(num_player);
  }
}
