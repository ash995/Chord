CC=gcc
OBJ=client

client: KVClient/KVClient.c XMLlib/XMLlib.c XMLlib/XMLlib.h
	$(CC) KVClient/KVClient.c XMLlib/XMLlib.c -o $(OBJ)

run: client
	./$(OBJ) Queries/batchRun.txt Output/out.txt 127.0.0.1 8080


clean:
	rm -rf client