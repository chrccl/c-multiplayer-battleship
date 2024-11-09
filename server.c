#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX_PLAYERS 4
#define GRID_LEN 10

int PLAYERS_NUM = 0;

int num_players_alive = 4;
int deads[MAX_PLAYERS] = {1, 1, 1, 1};
pthread_mutex_t mutexcount;

int players_placement_grid[MAX_PLAYERS][10][10];
int players_game_grid[MAX_PLAYERS][10][10];

int turn = 0;
int prev_turn = 3;

int aircraftCarrierSunk[MAX_PLAYERS] = {0};
int submarine1Sunk[MAX_PLAYERS] = {0};
int submarine2Sunk[MAX_PLAYERS] = {0};
int torpedoBoatSunk[MAX_PLAYERS] = {0};

int aircraftCarrierStock[MAX_PLAYERS][4][2] = {{{0}}};
int submarine1Stock[MAX_PLAYERS][3][2] = {{{0}}};
int submarine2Stock[MAX_PLAYERS][3][2] = {{{0}}};
int torpedoBoatStock[MAX_PLAYERS][2][2] = {{{0}}};

int is_ship_sunk(int player, int stock[][4][2], int ship_size) {
  for (int i = 0; i < ship_size; i++) {
    int x = stock[player][i][0];
    int y = stock[player][i][1];
    if (players_game_grid[player][x][y] != 1) {
        return 0;
    }
  }
  return 1;
}

int check_and_update_sunk_status(int player, int stock[][4][2], int *sunk_status, int ship_size) {
  if (*sunk_status == 0 && is_ship_sunk(player, stock, ship_size)) {
    *sunk_status = 1;
    return 1;
  }
  return 0;
}

int check_boat(int *cli_sockfd, int player) {
  if (check_and_update_sunk_status(player, aircraftCarrierStock, &aircraftCarrierSunk[player - 1], 4)) return 1;
  if (check_and_update_sunk_status(player, submarine1Stock, &submarine1Sunk[player - 1], 3)) return 1;
  if (check_and_update_sunk_status(player, submarine2Stock, &submarine2Sunk[player - 1], 3)) return 1;
  if (check_and_update_sunk_status(player, torpedoBoatStock, &torpedoBoatSunk[player - 1], 2)) return 1;

  return 0;
}

void addBoatPosition(int player, int x, int y, int boatType, int counter, int stock[][10][2]) {
    stock[player][counter][0] = x;
    stock[player][counter][1] = y;
}

void fillBoatArray(int player) {
    int cptPA = 0, cptT = 0, cptSM1 = 0, cptSM2 = 0;
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int cellValue = players_placement_grid[player][i][j];
            
            switch (cellValue) {
                case 4:
                    addBoatPosition(player, i, j, cellValue, cptPA++, aircraftCarrierStock);
                    break;
                case 1:
                    addBoatPosition(player, i, j, cellValue, cptT++, torpedoBoatStock);
                    break;
                case 2:
                    addBoatPosition(player, i, j, cellValue, cptSM1++, submarine1Stock);
                    break;
                case 3:
                    addBoatPosition(player, i, j, cellValue, cptSM2++, submarine2Stock);
                    break;
                default:
                    break;
            }
        }
    }
}


void error(const char * msg) {
  perror(msg);
  pthread_exit(NULL);
}

int is_game_over(int * cli_sockfd, int player) {
  for(int i = 0; i < 10; i++){
    for(int j = 0; j < 10; j++){
      if(players_placement_grid[player][i][j] != 0){
        if(players_game_grid[player][i][j] == 0){
          return 0;
        }
      }
    }
  }
  deads[player] = 0;
  write_client_msg(cli_sockfd[player], "LOS");
  close(cli_sockfd[player]);
  num_players_alive--;
  return 1;
}

void write_client_data(int sockfd, int tab[10][10]) {
  int n = write(sockfd, tab, 100 * sizeof(int));
  if (n < 0)
    error("Error");
}

void get_grid_symbol(int value, char *symbol) {
    // Determine the symbol based on the grid value
    switch (value) {
        case 0: strcpy(symbol, " "); break;
        case 1: strcpy(symbol, "\033[42mO\033[0m"); break;  // For grid_game - green 'O'
        case 2: strcpy(symbol, "\033[41mX\033[0m"); break;  // For grid_game - red 'X'
        case 3: strcpy(symbol, "\033[46mX\033[0m"); break;  // For grid_placement - cyan 'X'
        case 4: strcpy(symbol, "\033[43mX\033[0m"); break;  // For grid_placement - yellow 'X'
        default: strcpy(symbol, " "); break;
    }
}

void print_grid_row(char data[10][10][30], int row) {
    // Print a single row with formatted data
    printf("%d |", row);
    for (int j = 0; j < 10; j++) {
        printf(" %s |", data[row][j]);
    }
    printf("\n   -------------------------------------------\n");
}

void fill_data(int grid[10][10], char data[10][10][30], int is_game_grid) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int value = grid[i][j];
            get_grid_symbol(value, data[i][j]);
            // Adjust symbols for specific grid types
            if (!is_game_grid && value == 1) strcpy(data[i][j], "\033[41mX\033[0m");  // Player grid placement red 'X'
        }
    }
}

void draw_grid(int grid[10][10], int is_game_grid, int player_id) {
    char data[10][10][30];
    fill_data(grid, data, is_game_grid);

    if (!is_game_grid) {
        printf("------------------GRIGLIA DI GIOCO %d------------------------\n", player_id + 1);
    }

    printf("    A | B | C | D | E | F | G | H | I | J | \n");
    printf("   -------------------------------------------\n");
    for (int i = 0; i < 10; i++) {
        print_grid_row(data, i);
    }
}

void recv_msg(int sockfd, char * msg) {
  memset(msg, 0, 4);
  int n = read(sockfd, msg, 3);

  if (n < 0 || n != 3)
    error("Error ");
}

void recv_grid(int src[10][10], int cli_sockfd) {
  int msg[10][10];
  int n = read(cli_sockfd, msg, 100 * sizeof(int));
  memcpy(src, msg, 100 * sizeof(int));
}

void recv_grid_game(int src[10][10], int cli_sockfd, int player) {
  int msg[10][10];
  int n = read(cli_sockfd, msg, 100 * sizeof(int));
  memcpy(src, msg, 100 * sizeof(int));
}

void recv_square(int sockfd, int data[3]) {
  int tmp[3];
  int n = read(sockfd, tmp, 3 * sizeof(int));
  if (n == -1) {
    perror("Square Read error");
    exit(EXIT_FAILURE);
  }
  memcpy(data, tmp, 3 * sizeof(int));
}

void write_client_msg(int cli_sockfd, char * msg) {
  int n = write(cli_sockfd, msg, strlen(msg));
  if (n < 0)
    error("Error");
}

void write_client_int(int cli_sockfd, int msg) {
  int n = write(cli_sockfd, & msg, sizeof(int));
  if (n < 0)
    error("Error");
}

void write_clients_msg(int * cli_sockfd, char * msg) {
  for(int i = 0; i < PLAYERS_NUM; i++){
    if(deads[i] == 1)
      write_client_msg(cli_sockfd[i], msg);
  }
}

void write_clients_int(int * cli_sockfd, int msg) {
  for(int i = 0; i < PLAYERS_NUM; i++){
    if(deads[i] == 1)
      write_client_int(cli_sockfd[i], msg);
  }
}

int is_hit(int move[3]) {
    return players_placement_grid[move[2]][move[0]][move[1]] != 0;
}

void update_game_grid_with_hit(int move[3]) {
    players_game_grid[move[2]][move[0]][move[1]] = 1;
}

void update_game_grid_with_miss(int move[3]) {
    players_game_grid[move[2]][move[0]][move[1]] = 2;
}

void handle_hit(int *cli_sockfd, int sockfd, int from, int target_player) {
    printf("Player %d hit Player %d\n", from, target_player);
    send_player_grid(sockfd, target_player);

    if (check_boat(cli_sockfd, target_player)) {
        notify_active_player(cli_sockfd, from, "NAF", "FAN");
    } else {
        notify_active_player(cli_sockfd, from, "TBH", "HBT");
    }

    if (is_game_over(cli_sockfd, target_player)) {
        notify_active_player(cli_sockfd, target_player, "ILE", "ELI");
    }
}

void handle_miss(int *cli_sockfd, int sockfd, int from, int target_player) {
    printf("Player %d did not hit Player %d\n", from, target_player);
    send_player_grid(sockfd, target_player);
    notify_active_player(cli_sockfd, from, "TNH", "HNT");
}

int process_turn_update(int *cli_sockfd, int sockfd, int from, int move[3]) {
    int hit_result = 0;

    if (is_hit(move)) {
        hit_result = 1;
        update_game_grid_with_hit(move);
        handle_hit(cli_sockfd, sockfd, from, move[2]);
    } else {
        update_game_grid_with_miss(move);
        handle_miss(cli_sockfd, sockfd, from, move[2]);
    }

    return hit_result;
}

void notify_active_player(int * cli_sockfd, int player, const char *message, const char *default_msg){
  for(int i = 0; i < PLAYERS_NUM; i++){
    if(player == i){
      if(deads[i] == 1)
        write_client_msg(cli_sockfd[player], message);
    }else{
      if(deads[i] == 1)
        write_client_msg(cli_sockfd[i], default_msg);
    }
  }
}

void send_alive_players(int * cli_sockfd, int player){
  int num_alive = 0;
  for(int i = 0; i < PLAYERS_NUM; i++){
    if(deads[i] == 1) num_alive++;
  }
  int num_opponents = num_alive-1;
  int n = write(cli_sockfd[player], &num_opponents, sizeof(int));
  if (n < 0)
    error("Error");
  int alive_opponents[num_opponents];
  int j = 0;
  for(int i = 0; i < PLAYERS_NUM; i++){
    if(deads[i] == 1 && i != player) alive_opponents[j++] = i;
  }
  n = write(cli_sockfd[player], alive_opponents, num_opponents*sizeof(int));
  if (n < 0)
    error("Error");
}

int parseInt(char *str){
  char *end;
  errno = 0;
  int num = (int) strtol(str, &end, 10);
  if(errno == ERANGE || (*end && *end != '\n')){
    return -1;
  }
  return num;
}

void recv_turn_action(int cli_sockfd, int *tmp, int *grid_to_send){
  int buffer[3];
  int n = read(cli_sockfd, buffer, 3*sizeof(int));
  if (n == -1) {
    perror("Square Read error");
    exit(EXIT_FAILURE);
  }
  printf("Mossa: %d %d %d\n", buffer[0], buffer[1], buffer[2]);
  if(buffer[0] == -1){
    *grid_to_send = buffer[2];
  }else{
    *grid_to_send = -1;
  }
  memcpy(tmp, buffer, sizeof(buffer));
}

void send_player_grid(int cli_sockfd, int player){
  int n = write(cli_sockfd, players_game_grid[player], 100 * sizeof(int));
  if (n < 0)
      error("Error writing to socket");
}

int play(int *cli_sockfd, int player, int data[3]) {
  int action_status  = 1;
  int turn_data[3];
  int grid_to_send;

  while (action_status  == 1) {
    notify_active_player(cli_sockfd, player, "TRN", "NTR");
    send_alive_players(cli_sockfd, player);
    recv_turn_action(cli_sockfd[player], turn_data, &grid_to_send);
    if(grid_to_send != -1){
      send_player_grid(cli_sockfd[player], grid_to_send);
    }else{
      action_status  = process_turn_update(cli_sockfd, cli_sockfd[player], player, turn_data);
    }
  }

  memcpy(data, turn_data, 3 * sizeof(int));
  return action_status;
}

int check_square(char * data) {
  int nbr = data[0] - '0';
  if (nbr >= 0 && nbr <= 9 && data[1] >= 'A' && data[1] <= 'J')
    return 1;
  return 0;
}

int next_turn(int *curr){
    do{
      *curr = (*curr + 1) % PLAYERS_NUM;
    }while(deads[turn] == 0);
}

int has_won(int player_id, int *cli_sockfd){
    if (num_players_alive == 1) {
      printf("Player%i won!", (turn + 1));
      write_client_msg(cli_sockfd[player_id], "WIN");
      return 1;
    }
    return 0;
}

void *run_game(int * cli_sockfd) {
  char msg[4];
  printf("i giocatori stanno riempiendo le griglie...\n");
  write_clients_msg(cli_sockfd, "SRT");
  for(int cur = 0; cur < PLAYERS_NUM; cur++){
    //for(int i = 0; i < PLAYERS_NUM; i++){
    //  if(cur == i){
    //    write_client_msg(cli_sockfd[cur], "FIG");
    //  }else{
    //    write_client_msg(cli_sockfd[i], "WFG");
    //  }
    //}
    notify_active_player(cli_sockfd, cur, "FIG", "WFG");
    recv_grid(players_placement_grid[cur], cli_sockfd[cur]);
    draw_grid(players_placement_grid[cur], 0, cur);
    fillBoatArray(cur);
    printf("la griglia del giocatore %d e' stata riempita\n", cur);
  }

  printf("tutte le griglie sono pronte, inizio della partita\n");
  write_clients_msg(cli_sockfd, "STP");
  int game_over = 0;
  while (!game_over) {
    int square[3];
    printf("e' il turno del giocatore %d\n", (turn + 1));
    int value = play(cli_sockfd, turn, square);

    next_turn(&turn);
    game_over = has_won(turn, cli_sockfd);
  }
  printf("\n partita terminata, disconnessione dei giocatori...\n");

  for(int i = 0; i < PLAYERS_NUM; i++){
    if(deads[i] == 1)
      close(cli_sockfd[i]);
  }
}

int retrieve_clients_connections(int num_player, int *cli_sockfd){
    int ret = 1;
    for(int i = 0; i < PLAYERS_NUM; i++){
      char name[20];
      sprintf(name, "%d", i);
      char *fd_env = getenv(name);
      if(fd_env){
          cli_sockfd[i] = parseInt(fd_env);
      }else{
          ret = 0;
          break;
      }
    }
    return ret;
}

int main(int argc, char * argv[]) {
  if (argc < 2) {
    printf("utilizzo: game <NUMERO GIOCATORI: 2 o 4)\n");
    exit(1);
  }
  PLAYERS_NUM = parseInt(argv[1]);
  if(PLAYERS_NUM == -1 || (PLAYERS_NUM != 2 && PLAYERS_NUM != 4)){
      printf("numero giocatori fornito invalido, inserire 2 o 4.\n");
      exit(EXIT_FAILURE);
  }
  pthread_mutex_init( &mutexcount, NULL);

  //for(int k = 0; k < PLAYERS_NUM; k++){
  //  for (int i = 0; i < 10; i++) {
  //    for (int j = 0; j < 10; j++) {
  //      players_game_grid[k][i][j] = 0;
  //    }
  //  }
  //}
  memset(players_game_grid, 0, GRID_LEN * GRID_LEN* PLAYERS_NUM);
  int cli_sockfd[PLAYERS_NUM];
  if(!retrieve_clients_connections(PLAYERS_NUM, cli_sockfd)){
      printf("errore nel recupero delle connessioni dei giocatori");
      exit(EXIT_FAILURE);
  }
  run_game(cli_sockfd);
}
