#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include"thread_pool.h"
#include "../XMLlib/XMLlib.h"
#include "../KVCache/KVCache.h"

#define BUFFER_SIZE 1026*256*8

#define MULTIPLIER (97)

// This is where the main work is done
// rest of the functions are just for the management of the thread pool
// and the request queue

char *query_t[16] = {"GET", "PUT", "DEL", "GETR", "SUCCESS", "UNSUCCESS", "ERROR"};
char buffer_tp[BUFFER_SIZE];

extern struct cache* cache_in_use;

pthread_mutex_t keytable_lock;


static int
hash_function(const char *s, int sets) {
	unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (int)hash%sets;
}

void process_get_request(struct QUERY* query, int fd) {
	int byte_w, len;

	struct block* current_block = get_current_block(cache_in_use, query->key);
	// obtain lock of block in cache
	pthread_mutex_lock(&(current_block->block_lock));

	// now, get value
	struct QUERY* q = get_entry_from_cache(current_block, query->key, cache_in_use->set_size);

	// release lock
	pthread_mutex_unlock(&(current_block->block_lock));

	// build response xml
	memset(buffer_tp, 0, BUFFER_SIZE);
	// query->type = GETR;
	// strcpy(query->value, value);
	// strcpy(query)
	XMLlib_build(buffer_tp, q);
	// printf("%s\n", buffer_tp);

	// Writing length of message + 1 on socket
	len = strlen(buffer_tp) + 1;
	write(fd, &len, sizeof(int));
	// printf("%d\n", len);
	// write response to fd
	byte_w = 0;
	while(byte_w < len){
		// printf("IN Get write\n");
		byte_w += write(fd, &buffer_tp[byte_w], len - byte_w);
		// printf("%d\n", byte_w);
	}

	close(fd);

}

void process_put_request(struct QUERY* query, int fd) {
	int len, byte_w;
	struct block* current_block = get_current_block(cache_in_use, query->key);

	// printf("current block --- %p\n", current_block);

	// print_cache(cache_in_use);

	pthread_mutex_lock(&(current_block->block_lock));

	struct QUERY* q = put_entry_in_cache(current_block, query->key, query->value, cache_in_use->set_size);

	pthread_mutex_lock(&(keytable_lock));
	int hash_value = hash_function(query->key, 65535);
	put_key_in_keytable(query->key, hash_value);
	pthread_mutex_unlock(&(keytable_lock));

	pthread_mutex_unlock(&(current_block->block_lock));

	// print_cache(cache_in_use);

	// build xml response
	memset(buffer_tp, 0, BUFFER_SIZE);
	// query->type = SUCCESS;
	// strcpy(query->value, value);
	// strcpy(query)
	if (XMLlib_build(buffer_tp, q) < 0) {
		// printf("XMLlib_build returned -1\n");
	}

	// write response to fd
	// printf("reply to client ---- %s\n", buffer_tp);
	len = strlen(buffer_tp) + 1;
	write(fd, &len, sizeof(int));
	byte_w = 0;

	// printf("Buffer: \t==%s== buf_len = %d\n", buffer_tp, len);
	// for (int k = 0; k < len; k++){
	// 	if(buffer_tp[k] == '\0'){
	// 		// printf("NULL\n");
	// 	}
	// }

	while(byte_w < len){
		// printf("IN PUT Write\n");
		byte_w += write(fd, &buffer_tp[byte_w], len - byte_w);
	}

	close(fd);
}

void process_delete_request(struct QUERY* query, int fd) {
	int len, byte_w;
	struct block* current_block = get_current_block(cache_in_use, query->key);

	pthread_mutex_lock(&(current_block->block_lock));

	struct QUERY* q = del_entry_from_cache(current_block, query->key, cache_in_use->set_size);

	pthread_mutex_lock(&(keytable_lock));
	int hash_value = hash_function(query->key, 65535);
	del_key_from_keytable(query->key, hash_value);
	pthread_mutex_unlock(&(keytable_lock));

	pthread_mutex_unlock(&(current_block->block_lock));

	// build xml response
	memset(buffer_tp, 0, BUFFER_SIZE);
	// query->type = SUCCESS;
	// strcpy(query->value, value);
	// strcpy(query)
	XMLlib_build(buffer_tp, q);

	// write response to fd
	len = strlen(buffer_tp) + 1;
	write(fd, &len, sizeof(int));
	byte_w = 0;
	while(byte_w < len){
		// printf("IN delete\n");
		byte_w += write(fd, &buffer_tp[byte_w], len - byte_w);
	}

	close(fd);
}

void worker(void *arg) {
    struct arg_t *arg1 = (struct arg_t*)arg;
    int len, byte_w;
    // int  old = *val;

    // *val += 1000;
    // printf("tid=%lu, fd=%d \n ", pthread_self(), arg1->fd);


    // parse request string from XML into query
    struct QUERY* query = (struct QUERY*)malloc(sizeof(struct QUERY));

    XMLlib_qinit(query);
    XMLlib_parse(arg1->request_string, query);

    // parse query to perform specific operation
    if (strlen(query->key) > 256) {
    	struct QUERY* eq = (struct QUERY*)malloc(sizeof(struct QUERY));
    	XMLlib_qinit(eq);
    	eq->type = UNSUCCESS;
    	eq->message = "Oversized Key";
    	memset(buffer_tp, 0 , BUFFER_SIZE);
    	XMLlib_build(buffer_tp, eq);

    	len = strlen(buffer_tp) + 1;
		write(arg1->fd, &len, sizeof(int));
		byte_w = 0;
		while(byte_w < len){
		// printf("IN delete\n");
			byte_w += write(arg1->fd, &buffer_tp[byte_w], len - byte_w);
		}
    }
    else if (strlen(query->value) > 256*1024) {
    	struct QUERY* eq = (struct QUERY*)malloc(sizeof(struct QUERY));
    	XMLlib_qinit(eq);
    	eq->type = UNSUCCESS;
    	eq->message = "Oversized value";
    	memset(buffer_tp, 0 , BUFFER_SIZE);
    	XMLlib_build(buffer_tp, eq);

    	len = strlen(buffer_tp) + 1;
		write(arg1->fd, &len, sizeof(int));
		byte_w = 0;
		while(byte_w < len){
		// printf("IN delete\n");
			byte_w += write(arg1->fd, &buffer_tp[byte_w], len - byte_w);
		}
    }
    else if (query->type == 0) {
    	// get request
    	process_get_request(query, arg1->fd);
    }
    else if (query->type == 1) {
    	// put request
    	process_put_request(query, arg1->fd);
    }
    else if (query->type == 2){
    	// delete request
    	process_delete_request(query, arg1->fd);
    }


    // if ((arg1->fd)%2)
    //     usleep(100000);
}

struct job_t* create_job(void (*todo)(void* args1), void* args2) {
	struct job_t* new_job = (struct job_t*)malloc(sizeof(struct job_t));

	new_job->todo = todo;
	new_job->args = args2;
	new_job->next_job = NULL;

	return new_job;
}

void print_queue(struct th_pool_t* th_pool) {
	struct job_t* print_job = th_pool->first;
	printf("\n\n");
	while(print_job != NULL) {
		printf("%s -> ", ((struct arg_t*)(print_job->args))->request_string);
		print_job = print_job->next_job;
	}
	printf("\n\n");
}

void add_job_to_queue(struct th_pool_t* th_pool, struct job_t* new_job) {
	// acquire lock on thread pool
	pthread_mutex_lock(&(th_pool->job_lock));

	// printf("Adding to queue %s\n", ((struct arg_t*)(new_job->args))->request_string);
	// printf("%d -- %s\n", ((struct arg_t*)(new_job->args))->fd, ((struct arg_t*)(new_job->args))->request_string);
	if (th_pool->last == NULL && th_pool->first == NULL) {
		th_pool->first = new_job;
		th_pool->last = new_job;
	} else {
		th_pool->last->next_job = new_job;
		th_pool->last = new_job;
	}

	// print_queue(th_pool);

	pthread_cond_broadcast(&(th_pool->queue_empty));
	pthread_mutex_unlock(&(th_pool->job_lock));

}

void remove_job_from_queue(struct job_t* remove_job) {
	if (remove_job != NULL) {
		/* code */
		free(remove_job);
	}
}

struct job_t* get_job_from_queue(struct th_pool_t* th_pool) {
	struct job_t* job_to_be_done;
	struct job_t* job = th_pool->first;
	
	job_to_be_done = th_pool->first;
	if (th_pool->first == NULL)
	{
		/* code */
		return NULL;
	}
	else if (job_to_be_done->next_job == NULL)
	{
		/* code */
		th_pool->first = NULL;
		th_pool->last = NULL;
	}
	else {
		th_pool->first = job_to_be_done->next_job;
	}

	return job_to_be_done;
}


void* worker_function(void* args) {

	struct th_pool_t* th_pool = (struct th_pool_t*)args;
	// printf("Thread %lu has started\n", pthread_self()); 
	while(1) {
		pthread_mutex_lock(&(th_pool->job_lock));
		// printf("%s\n", );

		while (th_pool->first == NULL)
		{
			/* code */
			// printf("Thread %lu waiting for jobs\n", pthread_self());
			pthread_cond_wait(&(th_pool->queue_empty), &(th_pool->job_lock));
		}

		struct job_t* work = get_job_from_queue(th_pool);

		// printf("Thread %lu ready to service job %s\n", pthread_self(), ((struct arg_t*)(work->args))->request_string);

		pthread_mutex_unlock(&(th_pool->job_lock));

		if (work != NULL) {
			/* code */
			work->todo(work->args);
			remove_job_from_queue(work);
		}
	}
	return NULL;
}


struct th_pool_t* thread_pool_create(int num_threads) {
	// buffer_tp = (char*)malloc(256*1024*8);
	pthread_t thread;
	struct th_pool_t* th_pool = (struct th_pool_t*)malloc(sizeof(struct th_pool_t));

	// set up linked list
	th_pool->first = NULL;
	th_pool->last = NULL;

	// initialize locks and conds
	pthread_mutex_init(&(th_pool->job_lock), NULL);
    pthread_cond_init(&(th_pool->queue_empty), NULL);

	int i = 0;
	for (; i < num_threads; i++) {
		// printf("creating threads\n");
		pthread_create(&thread, NULL, worker_function, (void*)th_pool);
		// pthread_detach(thread);
	}
	return th_pool; 
}

struct arg_t* setup_args(int request_fd, char* request_message) {
	struct arg_t* args = (struct arg_t*)malloc(sizeof(struct arg_t));
	args->fd = request_fd;
	args->request_string = request_message;
	return args;
}
