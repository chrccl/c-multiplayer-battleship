TARGET_DIR = target

LIST_EXEC_NAME = Listener
GAME_EXEC_NAME = Game

LIST_PARAMS = -pthread -DGAME_EXEC_NAME=\"./$(GAME_EXEC_NAME)\"

all: hosting

# Check if OS is undefined (i.e., a Unix-like system)
ifeq ($(origin OS), undefined)
hosting:
	@if [ ! -d $(TARGET_DIR) ]; then mkdir $(TARGET_DIR); fi
	gcc listener.c $(LIST_PARAMS) -o $(TARGET_DIR)/$(LIST_EXEC_NAME)
	gcc server.c -o $(TARGET_DIR)/$(GAME_EXEC_NAME)
	@echo "Servizio di hosting compilato con successo. Per iniziare ad hostare le partite, esegui: ./Listener <DIMENSIONE PARTITE>"
else
hosting:
	@echo "La compilazione ed esecuzione del servizio di hosting deve essere effettuata su un sistema Unix-like."
endif

clean:
	@if [ -d $(TARGET_DIR) ]; then rm -f $(TARGET_DIR)/$(LIST_EXEC_NAME) $(TARGET_DIR)/$(GAME_EXEC_NAME); fi
