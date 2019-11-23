CC = gcc
CFLAGS = -lpthread
DEPS = XMLlib/XMLlib.h KVCache/KVCache.h ThPool/thread_pool.h KVStore/KVStore.h
OBJ = KVServer/kv_server.o KVCache/KVCache.o KVStore/KVStore.o  ThPool/thread_pool.o  XMLlib/XMLlib.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(DEPS) $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $@

run: server
	./server -threadPoolSize=4 -numSetsInCache=5 -port=8080 -sizeOfSet=5

clean:
	rm -rf server
	rm ./*/*.o

clean-db:
	rm -rf db.txt