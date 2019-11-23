/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<pthread.h>
#include<stdlib.h>
#include"../ThPool/thread_pool.h"
#include<unistd.h>
#include<string.h>
#include<strings.h>
#include<signal.h>
#include <arpa/inet.h>
#include "../KVCache/KVCache.h"
#include "../KVStore/KVStore.h"
#include "../../chordDHT/chord.h"

#define MAX_BUFFER_SIZE 1026*256*8

#define MULTIPLIER (97)

// buffer to read in requests
char buffer[MAX_BUFFER_SIZE];

// Thread pool
struct th_pool_t* th_pool;
struct cache* cache_in_use;

struct query_params* q_params;

// printing error messages
void error(char *msg)
{
	perror(msg);
	exit(1);
}

struct query_params {
	int port;
	int num_block;
	int set_size;
	int th_pool_size;
	char* known_ip;
	char* my_ip;
	int join;
	int my_id;
};

static int
hash_function(const char *s, int sets) {
	unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (int)hash%sets;
}


int get_fd_for_client() {
	int socket_fd;
	struct sockaddr_in serv_addr;
	// int nbyte_w; 	// nbytes write(), read() done so far for a connection
	// int nbyte_temp, len; 			// For temporary work
	int PORT;

	PORT = 9001;
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("Network Error: Could not create socket\n");
        // fprintf(out_file, "Network Error: Could not create socket\n");
        exit(-1); 
    }

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){ 
        // printf("Client: Invalid address given as command line argument. \n"); 
        exit(-1); 
    }

    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) { 
        printf("Network Error: Could not connect\n"); 
        // fprintf(out_file, "Network Error: Could not connect\n");
        exit(-1); 
    }
    return socket_fd;
}

void send_response_to_client(char* query) {
	// get fd by connecting to client listening port
	int fd = get_fd_for_client();
	struct arg_t* args = setup_args(fd, strdup(query));
	add_job_to_queue(th_pool, create_job(worker, (void*)args));
}

int send_query_to_server(char* server_ip, char* query) {
	int socket_fd;
	struct sockaddr_in serv_addr;
	int nbyte_w; 	// nbytes write(), read() done so far for a connection
	int nbyte_temp, len; 			// For temporary work
	int PORT;

	PORT = q_params->port;

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("Network Error: Could not create socket\n");
        // fprintf(out_file, "Network Error: Could not create socket\n");
        exit(-1); 
    }

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0){ 
        // printf("Client: Invalid address given as command line argument. \n"); 
        exit(-1); 
    }

    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) { 
        printf("Network Error: Could not connect\n"); 
        // fprintf(out_file, "Network Error: Could not connect\n");
        exit(-1); 
    }

    len = nbyte_w = strlen(query) + 1;	// +1 for the '\0' at last of string

		// Writing length of the string on socket
	if ((nbyte_temp = write(socket_fd, &len, sizeof(int))) < 0){
			// printf("Client: socket broken\n");
		exit(-1);
	}

	// Send the whole xml buffer. Big xml may not be sent in 1 shot. So while()
	// printf("Sending:             %s\n", xml);
	while (nbyte_w > 0){
		// printf("in while nbyte_w = %d ", nbyte_w);

		if ((nbyte_temp = write(socket_fd, &query[len - nbyte_w], nbyte_w)) < 0){
			printf("Network Error: Could not send data. exiting\n");
			// fprintf(out_file, "Network Error: Could not send data\n");
			exit(-1);
		}
		// printf("One write done nbyte_temp = %d\n", nbyte_temp);
		nbyte_w -= nbyte_temp;
		if (nbyte_w < 0){
			// printf("Client: Some calculation error\n");
			exit(-1);
		}
	}

}

// int ijh = 0;

void* process(void* filed) {
	int query_size;
	int fd = *((int*)filed);
	free(filed);
	int n;
	// printf("Process called\n");
	while(1) {
		// ijh++;

		// One file done at client
		if(!read(fd, &query_size, sizeof(int))){
			break;
		}

		n = 0;
		while(query_size > n){
			// printf("In read\n");
			n += read(fd, &buffer[n], query_size - n);
		}


		struct QUERY* query = (struct QUERY*)malloc(sizeof(struct QUERY));

    	XMLlib_qinit(query);
    	XMLlib_parse(buffer, query);

    	chord_ip ch = find_successor(hash_function(query->key, 65535));

    	if (strcmp(ch.ip, q_params->my_ip) == 0)
    	{
    		// process normally with recieve_query
    		send_response_to_client(buffer);
    	}
    	else {
    		// send request to ip returned in find successor

    		send_query_to_server(ch.ip, buffer);

    	}


			// struct arg_t* args = setup_args(fd, strdup(buffer));

		// printf("%d ----- %s -- %d\n", args->fd, args->request_string, n);
	
		// pass the string and fd to the request pool
	
			// add_job_to_queue(th_pool, create_job(worker, (void*)args));
		// printf("%d -> %s\n", fd, buffer);
		// n = write(fd,"I got your message",18);
		// if (n < 0)
			// error("ERROR writing to socket");
		// if (query_size >= n) {
		// 	// error("ERROR reading from socket");
		// 	break;
		// }
	}
	return NULL;
}

// void transfer_keys(int start_hash, int end_hash) {
// 	int i = start_hash;
// 	for (; i < end_hash; ++i) {
// 		struct key_cache* keyc = get_keys_by_hash(i);
// 		while(keyc != NULL) {
// 			char* key = keyc->key;
// 			struct block* current_block = get_current_block(cache_in_use, key);
// 			struct QUERY* qu = get_entry_from_cache(current_block, key, cache_in_use->set_size);
// 			transfer_key_to_remote(qu->key, qu->value);
// 			qu = del_entry_from_cache(current_block, key, cache_in_use->set_size);
// 		}
// 	}
// }






void sig_handler(int signum){
    // printf("Received signal %d\n", signum);
    KVStore_complete();
    exit(1);
}






void parse_query(char* argv, struct query_params* q_params) {
	// char str[] = "Geeks-for-Geeks"; 
  
    // Returns first token 
    char* token = strtok(argv, "="); 
  
    // Keep printing tokens while one of the 
    // delimiters present in str[]. 
    while (1) { 
        // printf("%s\n", token);
        if (strcmp(token, "-port") == 0) {
         	/* code */
         	token = strtok(NULL, "=");
         	q_params->port = atoi(token); 
         	// break;
        }
        else if (strcmp(token, "-threadPoolSize") == 0) {
        	token = strtok(NULL, "=");
         	q_params->th_pool_size = atoi(token); 

        } 
        else if (strcmp(token, "-numSetsInCache") == 0) {
        	token = strtok(NULL, "=");
         	q_params->num_block = atoi(token); 
        }
        else if (strcmp(token, "-sizeOfSet") == 0) {
        	token = strtok(NULL, "=");
         	q_params->set_size = atoi(token); 
        }
        else if (strcmp(token, "-knownIp") == 0) {
        	/* code */
        	token = strtok(NULL, "=");
        	q_params->known_ip = token;
        }
        else if (strcmp(token, "-join") == 0) {
        	token = strtok(NULL, "=");
        	q_params->join = atoi(token);
        }
        else if (strcmp(token, "-myIp") == 0) {
        	token = strtok(NULL, "=");
        	q_params->my_ip = token;
        }
        break; 
    } 
  
    // return 0; 
}

pthread_t process_thread;




int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, clilen;
	int socket_option = 1; // Abhik: To fix socket binding issue upon restart

	q_params = (struct query_params*)malloc(sizeof(struct query_params));

	int i;
	for (i = 1; i < argc; i++)
	{
		/* code */
		parse_query(argv[i], q_params);
	}

	q_params->my_id = hash_function(q_params->my_ip, 65535);

	int x = chordInit(q_params->my_id, q_params->my_ip, q_params->known_ip, q_params->join);

	// printf("%d\n", q_params->port);
	// printf("%d\n", q_params->num_block);
	// printf("%d\n", q_params->set_size);
	// printf("%d\n", q_params->th_pool_size);
	

	// creating a thread pool
	th_pool = thread_pool_create(q_params->th_pool_size);

	// initialize cache
	cache_in_use = initialize_cache(q_params->num_block, q_params->set_size);

	// printf("cache pointer -- %p\n", cache_in_use);
	// printf("%d\n", cache_in_use->set_size);
	// printf("%d\n", cache_in_use->sets);
	// print_cache(cache_in_use);

	//INITIALIZING XMLLIB
	// XMLlib_deugoff();

	KVStore_init();

	signal(SIGINT, sig_handler);


	
	struct sockaddr_in serv_addr, cli_addr;
	// int n;

	// int fds[5], maxfd = 0, minfd = 0;
	// fd_set read_set;
	

	// create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option));

	if (sockfd < 0) 
		error("ERROR opening socket");

	// setting up server address and port
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = q_params->port;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
			  error("ERROR on binding");
	listen(sockfd,5);
	
	clilen = sizeof(cli_addr);

	// continuously accept requests and serve them
	while (1) {
	 
		int fd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
		int* fd_arg = (int*)malloc(sizeof(*fd_arg));
		*fd_arg = fd;
		pthread_create(&process_thread, NULL, process, fd_arg);
		// process(fd);

	}
	return 0; 
}





