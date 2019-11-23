#pragma once
#include "../XMLlib/XMLlib.h"

struct key_cache {
	char* key;
	struct key_cache* next;
};

struct cache {
	struct block** cache_block;
	int sets;
	int set_size;
};

struct block {
	struct entry** cache_entry;
	int key_hash;
	pthread_mutex_t block_lock;
	pthread_cond_t block_cond;
	int clock_index;
};

struct entry {
	char* key;
	char* value;
	int use_bit;
};

void put_key_in_keytable(char* key, int hash_value);
int del_key_from_keytable(char* key, int hash_value);
struct cache* initialize_cache(int sets, int set_size);
struct QUERY* put_entry_in_cache(struct block* current_block, char* key, char* value, int set_size);
struct QUERY* get_entry_from_cache(struct block* current_block, char* key, int set_size);
struct QUERY* del_entry_from_cache(struct block* current_block, char* key, int set_size);
void print_cache(struct cache* cache_in_use);
struct block* get_current_block(struct cache* cache_in_use, char* key);


