Group Members
================================================================================
1. 184050002 - Abhik Bose
2. 194050001 - K Ashwin Kumar
3. 19V972005 - Bhushan Manikrao Patil

Compiling and running the KVStore using Makefile
================================================================================
1. To compile and run the server move into the Server folder
		make run 		=> To compile and run the server
		make 			=> To compile the server only
		make clean		=> To clean the build
		make clean-db	=> To clean the KVStore database

2. To compile and run the client move into the Client folder
		make run 		=> Compile and Run
		make 			=> Only compile
		make clean 		=> Clean the build

3. File Locations
	i. KVStore database i.e. where the server store all key and values are in 
	db.txt in Server folder. The KVStore creates one if not exists, else existing
	Keys are read into KVStore.

	ii. Client takes the input and output file from command line argument. If
	you use the Makefiles 

		a. Client inputs are taken from Queries/batchRun.txt and outputs are 
		written in Output/out.txt. You may copy or paste your own input in 
		batchRun.txt and run using the Makefile. You've given a sample one as 
		per the question format.

		b. If you like to run multiple client with different input files you
		can't use the Makefile. You've to run them manually. See as follows.

4. Default values
	
	i. IP = 127.0.0.1, Port = 8080.
	ii. -threadPoolSize=4 -numSetsInCache=5 -sizeOfSet=5

5. To STOP the server use CTRL + C. Please stop the server before inspecting 
	db.txt file. We've signal handler that compact the db.txt during exit. Else
	you may see hole in the file. Still the file will be in correct format but
	it'll look odd with unnecesseary whitespace.

6. NOTE: Please don't run them using very oversized key and value. We've fixed
	size buffer to store them. If oversize key and values are under limit like 
	260 to 270 bytes.


Manually Compiling and Running the KVStore
================================================================================
1. Compile the server in Server folder as follows
	gcc KVServer/kv_server.o KVCache/KVCache.o KVStore/KVStore.o  \
		ThPool/thread_pool.o  XMLlib/XMLlib.o -lpthread -o server

   Compile the client as follows
    gcc KVClient/KVClient.c XMLlib/XMLlib.c -o client

2. Run the server as follows
	./server -threadPoolSize=4 -numSetsInCache=5 -port=8080 -sizeOfSet=5
	(You may change any values)
   Run the client as follows
    ./clinet Queries/batchRun.txt Output/out.txt 127.0.0.1 8080
    (You may change values)

3. The server binds to all IP interfaces in a machine. It's not taken as a
	command line argument as it's not asked in question. 