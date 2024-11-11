TARGET_DIR = target

SERVER_DIR = src/server
CLIENT_DIR = src/client
COMMON_DIR = src/common

LIST_EXEC_NAME = Listener
GAME_EXEC_NAME = Game
CLIENT_EXEC_NAME = Client

LIST_PARAMS = --pthread -DGAME_EXEC_NAME=$(GAME_EXEC_NAME)

hosting:
	ifeq($(origin OS), undefined)
		gcc $(SERVER_DIR)/listener.c $(LIST_PARAMS) -o $(LIST_EXEC_NAME)
		gcc $(SERVER_DIR)/server.c -o $(GAME_EXEC_NAME)
		!echo "servizio di hosting compilato con successo, per iniziare ad hostare le partite si esegua \"./Listener <DIMENSIONE PARTITE>\""
	else
		!echo "la compilazione ed esecuzione del servizio di hosting deve essere effettuata su un sistema unix-like"

client:
	ifeq($(OS), Windows_NT)
		cl $(wildcard $(CLIENT_DIR)/*.c) -o $(CLIENT_EXEC_NAME)
		!echo "servizio client compilato con successo, per connettersi ad una partita si esegua \"Client <INDIRIZZO IP SERVIZIO DI HOSTING> <NUMERO PORTA SERVIZIO DI HOSTING>\""
	else
		!echo "the client service must be compiled and executed on a Windows system"
