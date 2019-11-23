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
#include "../KVCache/KVCache.h"
#include "../KVStore/KVStore.h"

#define MAX_BUFFER_SIZE 1026*256*8

// buffer to read in requests
char buffer[MAX_BUFFER_SIZE];

// Thread pool
struct th_pool_t* th_pool;
struct cache* cache_in_use;

// printing error messages
void error(char *msg)
{
	perror(msg);
	exit(1);
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

    	if (find_successor(query->key) == my_id)
    	{
    		// process normally with recieve_query
    	}
    	else {
    		// send request to id returned in find successor
    	}


		struct arg_t* args = setup_args(fd, strdup(buffer));

		// printf("%d ----- %s -- %d\n", args->fd, args->request_string, n);
	
		// pass the string and fd to the request pool
	
		add_job_to_queue(th_pool, create_job(worker, (void*)args));
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

void transfer_keys(int start_hash, int end_hash) {
	int i = start_hash;
	for (; i < end_hash; ++i) {
		struct key_cache* keyc = get_keys_by_hash(i);
		while(keyc != NULL) {
			char* key = keyc->key;
			struct block* current_block = get_current_block(cache_in_use, key);
			struct QUERY* qu = get_entry_from_cache(current_block, key, cache_in_use->set_size);
			transfer_key_to_remote(qu->key, qu->value);
			qu = del_entry_from_cache(current_block, key, cache_in_use->set_size);
		}
	}
}

void receive_query(char* query) {
	// get fd by connecting to client listening port
	struct arg_t* args = setup_args(fd, strdup(query));
	add_job_to_queue(th_pool, create_job(worker, (void*)args));
}

void sig_handler(int signum){
    // printf("Received signal %d\n", signum);
    KVStore_complete();
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
	unsigned const char *us;
	unsigned long h;
	h = 0; 
	for(us = (unsigned const char *) s; *us; us++) {
		h = h * MULTIPLIER + *us;
	}
	return h%sets;
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

struct query_params* q_params;


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
	 
		int fd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		int* fd_arg = malloc(sizeof(*fd_arg));
		*fd_arg = fd;
		pthread_create(&process_thread, NULL, process, fd_arg);
		// process(fd);

	}
	return 0; 
}




