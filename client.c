/***************************************************
IMPORTS
***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <errno.h>
#include "constants.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "User32.lib")



/***************************************************
SOCKET FUNCTIONS
***************************************************/


void error(const char *msg) {
    MessageBox(NULL, msg, "Error", MB_OK | MB_ICONERROR);
    exit(1);
}

void recv_msg(SOCKET sockfd, char *msg) {
    memset(msg, 0, 4);
    int n = recv(sockfd, msg, 3, 0);
    if (n == SOCKET_ERROR || n != 3)
        error("Error reading from socket");
}

int recv_int(SOCKET sockfd) {
    int msg = 0;
    int n = recv(sockfd, (char*)&msg, sizeof(int), 0);
    if (n == SOCKET_ERROR || n != sizeof(int))
        error("Error reading from socket");
    return msg;
}

void write_server_msg(SOCKET sockfd, char *msg) {
    int n = send(sockfd, msg, strlen(msg), 0);
    if (n == SOCKET_ERROR)
        error("Error writing to socket");
}


void write_server_int(SOCKET sockfd, int msg) {
    int n = send(sockfd, &msg, sizeof(int), 0);
    if (n == SOCKET_ERROR)
        error("Error writing to socket");
}

SOCKET connect_to_server(char *hostname, int portno) {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        error("Errore nell'inizializzazione di WinSock");
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
        error("Errore apertura socket");

    server = gethostbyname(hostname);
    if (server == NULL) {
        error("Nessun host trovato");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Errore di connessione");

    return sockfd;
}

void write_server_square(SOCKET sockfd, int buffer[3]) {
    int n = send(sockfd, buffer, 3 * sizeof(int), 0);
    if (n == SOCKET_ERROR)
        error("Error writing to socket");
}

void write_server_data(SOCKET sockfd, int tab[GRID_SIZE][GRID_SIZE]) {
    int n = send(sockfd, tab, 100 * sizeof(int), 0);
    if (n == SOCKET_ERROR)
        error("Error writing to socket");
}

void recv_grid(int src[GRID_SIZE][GRID_SIZE], SOCKET sockfd) {
    int msg[GRID_SIZE][GRID_SIZE];
    int n = recv(sockfd, msg, 100 * sizeof(int), 0);
    if (n == SOCKET_ERROR)
        error("Error reading from socket");
    memcpy(src, msg, 100 * sizeof(int));
}



/***************************************************
USER INTERFACE FUNCTIONS
***************************************************/


void draw_legends() {
  printf("LEGENDA :\n");
  printf("__________\n");
  printf(" - \033[43mPortaerei\033[0m\n");
  printf(" - \033[46mSottomarino\033[0m\n");
  printf(" - \033[41mTorpediniere\033[0m\n");
  printf("__________ \n");
}

void print_grid_row(int row, char data[GRID_SIZE][GRID_SIZE][30]) {
    printf("%d | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | \n",
           row, data[row][0], data[row][1], data[row][2], data[row][3], 
           data[row][4], data[row][5], data[row][6], data[row][7], 
           data[row][8], data[row][9]);
}

void initialize_data_array(char data[GRID_SIZE][GRID_SIZE][30], int grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            strcpy(data[i][j], " "); // Default to empty space
            if (grid[i][j] == 1) {
                strcpy(data[i][j], "\033[41mX\033[0m"); // Red X for hit
            } else if (grid[i][j] == 2 || grid[i][j] == 3) {
                strcpy(data[i][j], "\033[46mX\033[0m"); // Cyan X for a specific state
            } else if (grid[i][j] == 4) {
                strcpy(data[i][j], "\033[43mX\033[0m"); // Yellow X for another specific state
            }
        }
    }
}

void draw_grid(int grid[GRID_SIZE][GRID_SIZE], int is_game_grid) {
    char data[GRID_SIZE][GRID_SIZE][30];
    initialize_data_array(data, grid);

    const char *header = is_game_grid ? "    A | B | C | D | E | F | G | H | I | J | \n" : 
                                         "    A | B | C | D | E | F | G | H | I | J | \n";
    printf("%s", header);
    printf("   -------------------------------------------\n");

    for (int i = 0; i < GRID_SIZE; i++) {
        print_grid_row(i, data);
        printf("   -------------------------------------------\n");
    }
}

void draw_grid_placement(int grid[GRID_SIZE][GRID_SIZE]) {
    draw_grid(grid, 0); // Call with flag indicating placement grid
}

void draw_grid_game(int grid[GRID_SIZE][GRID_SIZE]) {
    draw_grid(grid, 1); // Call with flag indicating game grid
}

void recv_and_print_player_game_grid(SOCKET sockfd){
  int grid[GRID_SIZE][GRID_SIZE];
  int n = recv(sockfd, grid, 100 * sizeof(int), 0);
  if (n == SOCKET_ERROR)
      error("Error reading from socket");
  draw_grid_game(grid);
}



/***************************************************
HELPERS FUNCTIONS
***************************************************/


int check_square(char data[3]) {
  int nbr = data[0] - '0';
  if (nbr >= 0 && nbr <= 9 && data[1] >= 'A' && data[1] <= 'J')
    return 1;
  return 0;
}

void initialize_grid(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = 0;
        }
    }
}

int contains(char c, char tab[10]) {
  for (int i = 0; i < 10; i++) {
    if (c == tab[i])
      return 1;
  }
  return 0;
}

int getNumberInTab(char c, char tab[10]) {
  if (c == '0')
    return 0;
  for (int i = 0; i < 10; i++) {
    if (c == tab[i])
      return i;
  }
  return -1;
}

void get_alive_players(SOCKET sockfd, int *num_players, int **players){
  int n = recv(sockfd, num_players, sizeof(int), 0);
  if (n == SOCKET_ERROR)
    error("Error reading from socket 1");
  *players = (int*)malloc(*num_players * sizeof(int));
  if (*players == NULL) 
    error("Memory allocation failed");
  n = recv(sockfd, *players, (*num_players)*sizeof(int), 0);
  if (n == SOCKET_ERROR)
    error("Error reading from socket 2");
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



/***************************************************
FUNCTIONS FOR PLAYING THE TURN
***************************************************/


void display_alive_players(SOCKET sockfd) {
    int *players, num_players;
    get_alive_players(sockfd, &num_players, &players);
    printf("I giocatori vivi sono:\n");
    for (int i = 0; i < num_players; i++) {
        printf("Player%d\n", players[i] + 1);
    }
    free(players);
    printf("\n");
}

int parse_player_id(const char *player_token) {
    int player_id = parseInt(player_token);
    if (player_id < 0) {
        printf("Id del giocatore non valido. Riprova.\n");
    }
    return player_id;
}

int parse_coordinates(const char *coordinate_token, int *square, const char letters[]) {
    if (strlen(coordinate_token) == 2 && check_square(coordinate_token)) {
        square[0] = coordinate_token[0] - '0';
        square[1] = getNumberInTab(coordinate_token[1], letters);
        return 1; // valid coordinates
    } else {
        printf("Coordinate non valide!\n");
        return 0; // invalid coordinates
    }
}

void handle_user_choice(SOCKET sockfd, int player_id, char *coordinate_token, int *square, const char letters[]) {
    if (coordinate_token == NULL) {
        square[0] = -1;
        square[1] = -1;
        square[2] = player_id - 1;
        write_server_square(sockfd, square);
    } else if (parse_coordinates(coordinate_token, square, letters)) {
        square[2] = player_id - 1;
        write_server_square(sockfd, square);
    } else {
        printf("Qualcosa è andato storto.\n");
    }
}

void take_turn(SOCKET sockfd) {
    const char letters[10] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
    char buffer[100];
    int square[3];

    display_alive_players(sockfd);
    printf("Scrivi il numero del giocatore per vedere la sua tabella di gioco. (Esempio: per vedere il Player1 scrivere 1)\n");
    printf("Per attaccare scrivere il numero del giocatore e la coordinata. (Esempio: 1 3A)\n");

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            error("Errore durante input");
        }
        buffer[strcspn(buffer, "\n")] = 0;

        char *player_token = strtok(buffer, " ");
        int player_id = parse_player_id(player_token);
        if (player_id < 0) {
            continue;
        }

        char *coordinate_token = strtok(NULL, " ");
        handle_user_choice(sockfd, player_id, coordinate_token, square, letters);
        recv_and_print_player_game_grid(sockfd);
        break;
    }
}



/***************************************************
FUNCTIONS TO PLACE THE SHIPS
***************************************************/


int is_valid_placement(const char *input, int length) {
    return strlen(input) == length;
}

int validate_horizontal_placement(const char *placement, int grid[GRID_SIZE][GRID_SIZE], int row, int col1, int col2) {
    if (grid[row][col1] == 0 && grid[row][col2] == 0) {
        grid[row][col1] = grid[row][col2] = 1;
        return 1;
    }
    return 0;
}

int validate_vertical_placement(const char *placement, int grid[GRID_SIZE][GRID_SIZE], int row1, int row2, int col) {
    if (grid[row1][col] == 0 && grid[row2][col] == 0) {
        grid[row1][col] = grid[row2][col] = 1;
        return 1;
    }
    return 0;
}

int validate_and_place(int grid[GRID_SIZE][GRID_SIZE], int **coordinates, int ship_size, int value) {
    for (int i = 0; i < ship_size; i++) {
        int row = coordinates[i][0];
        int col = coordinates[i][1];
        if (grid[row][col] != 0) {
            return 0;
        }
    }
    for (int i = 0; i < ship_size; i++) {
        int row = coordinates[i][0];
        int col = coordinates[i][1];
        grid[row][col] = value;
    }
    return 1;
}

void place_ship(int grid[GRID_SIZE][GRID_SIZE], const char *ship_name, int ship_size, int value) {
    const char letters[10] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
    char placement[100];

    int is_correct = 0;

    int **coordinates = malloc(ship_size * sizeof(int *));
    for (int i = 0; i < ship_size; i++) {
        coordinates[i] = malloc(2 * sizeof(int));
    }

    while (!is_correct) {
        printf("Inserisci le coordinate per %s: ", ship_name);
        if (fgets(placement, sizeof(placement), stdin) == NULL) {
            error("Errore durante input");
        }
        placement[strcspn(placement, "\n")] = 0;
        if (is_valid_placement(placement, ship_size * 3 - 1)) {  // Account for '-' separators
            int valid_input = 1;

            for (int i = 0; i < ship_size; i++) {
                int row = placement[i * 3] - '0';
                int col = getNumberInTab(placement[i * 3 + 1], letters);

                if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
                    valid_input = 0;
                    break;
                }
                coordinates[i][0] = row;
                coordinates[i][1] = col;
            }

            if (valid_input) {
                is_correct = validate_and_place(grid, coordinates, ship_size, value);
            }
        }

        if (!is_correct) {
            printf("Posizionamento errato di %s. Per favore riprova.\n", ship_name);
        }
    }

    // Free dynamically allocated memory
    for (int i = 0; i < ship_size; i++) {
        free(coordinates[i]);
    }
    free(coordinates);
}


void fill_data(SOCKET sockfd) {
    int player_grid[GRID_SIZE][GRID_SIZE];
    initialize_grid(player_grid);

    const char *torpedo = "Torpediniera";
    const char *submarine = "Sottomarino";
    const char *aircraft_carrier = "Portaerei";

    draw_grid_placement(player_grid);
    printf("\nPer iniziare, posiziona il tuo %s (%d caselle).\n", torpedo, 2);
    printf("Scegli le caselle che vuoi riempire (es. 0D-0E o 4C-5C).\n");
    place_ship(player_grid, torpedo, 2, 1);
    for (int i = 0; i < 2; i++) {
        draw_grid_placement(player_grid);
        printf("\nBene, ora posiziona il %s %d. (es. 2D-2E-2F o 4F-5F-6F)\n", submarine, i + 1);
        place_ship(player_grid, submarine, 3, i + 2);
    }
    draw_grid_placement(player_grid);
    printf("\nInfine, posiziona il tuo %s (es. 8A-8B-8C-8D o 3G-4G-5G-6G).\n", aircraft_carrier);
    place_ship(player_grid, aircraft_carrier, 4, 4);

    printf("Perfetto! La tua griglia è stata riempita correttamente.\n");
    write_server_data(sockfd, player_grid);
    draw_legends();
    draw_grid_placement(player_grid);
}



/***************************************************
CLIENT MAIN
***************************************************/


int handle_message(SOCKET sockfd, char *msg) {
  if (!strcmp(msg, MSG_FILL_GRID)) {
    printf("Riempi la griglia...\n");
    fill_data(sockfd);
  } else if (!strcmp(msg, MSG_WAIT_FILL_GRID)) {
    printf("I tuoi avversari stanno riempiendo la griglia...\n");
  } else if (!strcmp(msg, MSG_START_GAME)) {
    printf("\nIL GIOCO COMINCIA!\n");
  } else if (!strcmp(msg, MSG_OPPONENT_TURN)) {
    printf("\nE' il turno dei tuoi avversari\n");
  } else if (!strcmp(msg, MSG_HIT_BY_OPPONENT)) {
    printf(COLOR_RED "Uno dei tuoi avversari ha colpito la tua flotta! Rimani concentrato!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_TARGET_HIT)) {
    printf(COLOR_GREEN "Hai colpito l'obiettivo! Continua l'offensiva!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_ENEMY_AIRCRAFT_CARRIER_SUNK)) {
    printf("\n" COLOR_YELLOW "Hai affondato la portaerei del tuo avversario." COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_OWN_AIRCRAFT_CARRIER_SUNK)) {
    printf("\n" COLOR_YELLOW "La vostra portaerei è stata affondata..." COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_ENEMY_SUBMARINE_SUNK)) {
    printf("\n" COLOR_CYAN "Hai affondato uno dei sottomarini del tuo avversario!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_OWN_SUBMARINE_SUNK)) {
    printf("\n" COLOR_CYAN "Il tuo sottomarino è stato affondato..." COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_ENEMY_DESTROYER_SUNK)) {
    printf("\n" COLOR_RED "Hai affondato la torpediniera del tuo avversario!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_OWN_DESTROYER_SUNK)) {
    printf("\n" COLOR_RED "La vostra torpediniera è stata affondata..." COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_TARGET_SUNK)) {
    printf(COLOR_GREEN "Hai affondato il tuo obiettivo! Continua così e vincerai!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_OPPONENT_TARGET_SUNK)) {
    printf(COLOR_RED "Uno dei tuoi avversari ha affondato il suo obiettivo! Controlla le tabelle prima di attaccare!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_OPPONENT_MISS)) {
    printf(COLOR_GREEN "Uno dei tuoi avversari ha mancato il suo obiettivo! Controlla le tabelle prima di attaccare!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_MISS)) {
    printf(COLOR_RED "Hai mancato il tuo obiettivo! Rimani concentrato!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_YOUR_TURN)) {
    printf("\n" COLOR_BLUE "E' il tuo turno!" COLOR_RESET "\n");
    take_turn(sockfd);
  } else if (!strcmp(msg, MSG_WIN)) {
    printf("\n" COLOR_GREEN "BRAVO SOLDATO! HAI VINTO!" COLOR_RESET "\n");
    return 0;
  } else if (!strcmp(msg, MSG_LOSS)) {
    printf("\n" COLOR_RED "Hai perso... Ma un soldato non ammette mai la sconfitta!" COLOR_RESET "\n");
    return 0;
  } else if (!strcmp(msg, MSG_PLAYER_ELIMINATED)) {
    printf("\n" COLOR_RED "Il giocatore colpito è stato eliminato!" COLOR_RESET "\n");
  } else if (!strcmp(msg, MSG_SELF_ELIMINATED)) {
    printf("\n" COLOR_RED "Sei stato eliminato!" COLOR_RESET "\n");
  }
  return 1;
}


int main(int argc, char * argv[]) {
  if (argc < 3) {
    printf("Scrivi server IP e porta: %s\n", argv[0]);
    exit(0);
  }
  SOCKET sockfd = connect_to_server(argv[1], atoi(argv[2]));
  int id = recv_int(sockfd);
  char msg[4];
  printf("La battaglia navale di Mirko Cetorelli e Christian Cecili\n");

  do {
    recv_msg(sockfd, msg);
    if (!strcmp(msg, MSG_WAIT_FOR_PLAYERS))
      printf("In attesa di altri giocatori...\n");
  } while (strcmp(msg, MSG_GAME_START));

  do {
    recv_msg(sockfd, msg);
  }while(handle_message(sockfd, msg));

  printf("Partita finita!\n");
  closesocket(sockfd);
  WSACleanup();
  return 0;
}
