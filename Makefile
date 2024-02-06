CC=gcc
FLAGS= -Wall -Werror -g

CLI=src/client
SERV=src/server
UTILS=src/utils
OBJ=ogj
BIN=bin

all : server chat




$(BIN):
	mkdir -p $(BIN)

$(OBJ):
	mkdir -p $(OBJ)

chat : $(CLI)/chat.c $(UTILS)/socket_utils.c | $(BIN)
	$(CC) -o $(BIN)/$@ $^ $(FLAGS)


server : $(SERV)/server.c $(UTILS)/socket_utils.c | $(BIN)
	$(CC) -o $(BIN)/$@ $^ $(FLAGS)


clean:
	rm -rf $(OBJ)/*
	rm -rf $(BIN)/*
