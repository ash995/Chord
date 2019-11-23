// each job in the request queue
#pragma once
struct job_t {
	void (*todo)(void* args);
	void* args;
	struct job_t* next_job;
};

// argument passed to each job
struct arg_t {
	int fd;
	char* request_string;
};

// thread pool struct
struct th_pool_t {
	// request queue linked list
	struct job_t* first;
	struct job_t* last;
	// job queue mutex
	pthread_mutex_t job_lock;
	// queue empty condition
	pthread_cond_t queue_empty;

};

struct job_t* create_job(void (*todo)(void* args), void* args);
void add_job_to_queue(struct th_pool_t* th_pool, struct job_t* job);
struct job_t* get_job_from_queue(struct th_pool_t* th_pool);
struct th_pool_t* thread_pool_create(int num_threads);
struct arg_t* setup_args(int fd, char* request_message);
void worker(void* args);
// struct arg_t* setup_args(int fd, char* str);
int hash_function(char *s, int sets);