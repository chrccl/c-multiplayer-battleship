TARGET_DIR = target

CLIENT_EXEC_NAME = Client

# Default target
all: client

# Client target
client:
	@if not exist $(TARGET_DIR) mkdir $(TARGET_DIR)
	@if "$(OS)" == "Windows_NT" ( \
		cl *.c /Fe$(TARGET_DIR)\$(CLIENT_EXEC_NAME) & \
		del /Q *.obj & \
		echo Client service compiled successfully. To connect to a game, execute "$(CLIENT_EXEC_NAME) <HOST IP ADDRESS> <HOST SERVICE PORT>" \
	) else ( \
		echo The client service must be compiled through a DCP of Visual Studio. \
	)

# Clean target
clean:
	@if exist $(TARGET_DIR)\$(CLIENT_EXEC_NAME).exe del /Q $(TARGET_DIR)\$(CLIENT_EXEC_NAME).exe
	@if exist *.obj del /Q *.obj
