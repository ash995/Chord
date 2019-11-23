#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include "KVCache.h"
#include "../XMLlib/XMLlib.h"
#include "../XMLlib/error_codes.h"
#include "../KVStore/KVStore.h"

pthread_mutex_t store_lock;
struct key_cache* keytable[65535];


extern struct cache* cache_in_use;

struct cache* initialize_cache(int sets, int set_size) {
	struct cache* cache_in_use = (struct cache*)malloc(sizeof(struct cache));
	struct block** blocks = (struct block**)malloc(sizeof(struct block*)*sets);
	// struct entry** cache_entry = (struct)
	int i, j;
	for (i = 0; i < sets; i++) {
		/* code */
		struct block* ind_block = (struct block*)malloc(sizeof(struct block));
		ind_block->key_hash = i;
		pthread_mutex_init(&(ind_block->block_lock), NULL);
		pthread_cond_init(&(ind_block->block_cond), NULL);

		struct entry** block_entry = (struct entry**)malloc(sizeof(struct entry*)*set_size);
		for (j = 0; j < set_size; j++) {
			/* code */
			struct entry* cache_entry = (struct entry*)malloc(sizeof(struct entry));
			// cache_entry->key = "Ashwin";
			// cache_entry->value = "Ash";
			block_entry[j] = cache_entry;

		}
		ind_block->cache_entry = block_entry;
		blocks[i] = ind_block;
	}
	cache_in_use->cache_block = blocks;
	cache_in_use->sets = sets;
	cache_in_use->set_size = set_size;

	pthread_mutex_init(&store_lock, NULL);
	return cache_in_use;
}

#define MULTIPLIER (97)

static int
hash_function(const char *s, int sets) {
	unsigned long hash = 5381;
    int c;

    while (c = *s++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (int)hash%sets;
}

int del_key_from_keytable(char* key, int hash_value) {
	if (keytable[hash_value] == NULL) {
		return 0;
	}
	else {
		struct key_cache* p = keytable[hash_value];
		struct key_cache* q = keytable[hash_value];
		while(p != NULL) {
			if (strcmp(p->key, key) == 0) {
				/* code */
				q->next = p->next;
				free(p);
				return 1;
			}
			q=p;
			p = p->next;

		}
		return 0;	
	}
}

struct key_cache* get_keys_by_hash(int hash_value) {
	return keytable[hash_value];
}

void put_key_in_keytable(char* key, int hash_value) {
	struct key_cache* entry = (struct key_cache*)malloc(sizeof(struct key_cache));
	entry->key = strdup(key);
	entry->next = NULL;
	if (keytable[hash_value] == NULL) {
		keytable[hash_value] = entry;
	} 
	else {
		struct key_cache* p = keytable[hash_value];
		while(p->next != NULL) {
			p = p->next;
		}
		p->next = entry;
	}
}

static int get_evict_index(struct block* current_block, int set_size) {
	int looper_index = current_block->clock_index;
	while ((current_block->cache_entry)[looper_index]->use_bit != 0) {
		(current_block->cache_entry)[looper_index]->use_bit = 0;
		looper_index = (looper_index + 1)% set_size;
	}
	return looper_index;
}

void print_cache(struct cache* cache_in_use) {
	int i, j;
	// printf("Printing Cache\n");
	// printf("%p\n", cache_in_use);
	// printf("%d\n", cache_in_use->set_size);
	// printf("%d\n", cache_in_use->sets);
	for (i = 0; i < cache_in_use->sets; i++) {
		/* code */
		struct block* block_to_use = (cache_in_use->cache_block)[i];
		// printf("%p\n", );
		printf("Printing block %d--%p\n", i, block_to_use);
		for (j = 0; j < cache_in_use->set_size; j++) {
			/* code */
			struct entry* ent_to_use = (block_to_use->cache_entry)[j];

			printf("%s --> %s (%p)\n", ent_to_use->key, ent_to_use->value, ent_to_use);
		}
	}
}

struct entry* search_for_key(struct block* current_block, char* key, int set_size) {
	int i;
	for (i = 0; i < set_size; i++) {
		/* code */
		if ((current_block->cache_entry)[i]->key != NULL && strcmp(key, (current_block->cache_entry)[i]->key) == 0)
		{
			/* code */
			return (current_block->cache_entry)[i];
		}
	}
	return NULL;
}

struct block* get_current_block(struct cache* cache_in_use, char* key) {
	int hash = hash_function(key, cache_in_use->sets);
	return (cache_in_use->cache_block)[hash];
}

struct QUERY* get_entry_from_cache(struct block* current_block, char* key, int set_size) {

	// printf("%p index\n", current_block);
	// now search for this key in that block
	struct entry* current_entry = search_for_key(current_block, key, set_size);


	// if found , return
	if (current_entry != NULL) {
		// make use bit 1 to indicate recently used
		current_entry->use_bit = 1;

		struct QUERY* qu = (struct QUERY*)malloc(sizeof(struct QUERY));
		XMLlib_qinit(qu);
		qu->type = GETR;
		strcpy(qu->key, key);
		strcpy(qu->value, current_entry->value);
		strcpy(qu->message, success_message);

		// return struct qu 
		return qu;

		// return current_entry->value;
	}
	// else goto store, get key and replace an entry if necessary, and return
	else {
		int evict_index = get_evict_index(current_block, set_size);

		// printf("%d\n", evict_index);

		struct QUERY* qu = (struct QUERY*)malloc(sizeof(struct QUERY));
		XMLlib_qinit(qu);
		qu->type = GET;
		strcpy(qu->key, key);

		// get cache entry value from kvstore
		// KVStore_restore_from_file(qu);
		// if ((current_block->cache_entry)[evict_index]->key != NULL) {
		// 	/* code */
		// 	struct QUERY* qu_temp = (struct QUERY*)malloc(sizeof(struct QUERY));
		// 	XMLlib_qinit(qu_temp);
		// 	qu_temp->type = PUT;
		// 	strcpy(qu_temp->key, (current_block->cache_entry)[evict_index]->key);
		// 	strcpy(qu_temp->value, (current_block->cache_entry)[evict_index]->value);

		// 	print_query(qu_temp);

		// 	pthread_mutex_lock(&store_lock);
		// 	KVStore_dumpToFile(qu_temp);
		// 	pthread_mutex_unlock(&store_lock);

		// }

		// print_query(qu);

		// get value from kvstore
		pthread_mutex_lock(&store_lock);
		KVStore_restoreFromFile(qu);
		pthread_mutex_unlock(&store_lock);

		// print_query(qu);

		if (qu->type != 5) {
			/* code */
		
			// (current_block->cache_entry)[evict_index]->key = key;
			strcpy((current_block->cache_entry)[evict_index]->key, qu->key);
			// (current_block->cache_entry)[evict_index]->value = value;
			strcpy((current_block->cache_entry)[evict_index]->value, qu->value);
			(current_block->cache_entry)[evict_index]->use_bit = 1;

			// set clock hand to evict index
			current_block->clock_index = evict_index;
		}

		// return struct qu
		// print_cache(cache_in_use);
		
		return qu;
		// return value;

	}
	
}

struct QUERY* put_entry_in_cache(struct block* current_block, char* key, char* value, int set_size) {

	// printf("%p index\n", current_block);

	

	struct QUERY* qu = (struct QUERY*)malloc(sizeof(struct QUERY));
	XMLlib_qinit(qu);
	qu->type = PUT;
	strcpy(qu->key, key);
	strcpy(qu->value, value);


	struct entry* current_entry = search_for_key(current_block, key, set_size);

	// print_cache(cache_in_use);

	if (current_entry != NULL) {
		// UPDATE IN CACHE
		current_entry->value = value;
		current_entry->use_bit = 1;

	}
	else {
	
		//get evict index
		int evict_index = get_evict_index(current_block, set_size);
		// printf("Evict index %d\n", evict_index);
		// if ((current_block->cache_entry)[evict_index]->key != NULL) {
		// 	// push key, value to KVStore
		// 	struct QUERY* qu_temp = (struct QUERY*)malloc(sizeof(struct QUERY));
		// 	XMLlib_qinit(qu_temp);
		// 	qu_temp->type = PUT;
		// 	strcpy(qu_temp->key, key);
		// 	strcpy(qu_temp->value, value);

		// 	// pass struct to KVStore
		// 	pthread_mutex_lock(&store_lock);
		// 	KVStore_dumpToFile(qu_temp);
		// 	pthread_mutex_unlock(&store_lock);

		// } 
	
		// put key, value at evict index
		(current_block->cache_entry)[evict_index]->key = key;
		(current_block->cache_entry)[evict_index]->value = value;
		(current_block->cache_entry)[evict_index]->use_bit = 1;

		

		// set clock hand to evict index
		current_block->clock_index = evict_index;
	}

	// push new key, value to kvstore
	// printf("%s\n", );
	// print_query(qu);
	pthread_mutex_lock(&store_lock);
	KVStore_dumpToFile(qu);
	pthread_mutex_unlock(&store_lock);
	// print_query(qu);
	// return struct qu
	// print_cache(cache_in_use);
	return qu;


}

struct QUERY* del_entry_from_cache(struct block* current_block, char* key, int set_size) {

	struct QUERY* qu = (struct QUERY*)malloc(sizeof(struct QUERY));
	XMLlib_qinit(qu);
	qu->type = DEL;
	strcpy(qu->key, key);

	// delete key from cache 
	struct entry* entry_to_delete = search_for_key(current_block, key, set_size);
	if (entry_to_delete != NULL) {
		entry_to_delete->key = NULL;
		entry_to_delete->value = NULL;
		entry_to_delete->use_bit = 0;
	}
	
	// delete entry from KVstore
	pthread_mutex_lock(&store_lock);
	KVStore_dumpToFile(qu);
	pthread_mutex_unlock(&store_lock);

	// return struct qu

	// print_cache(cache_in_use);
	return qu;
}

// int main(int argc, char const *argv[])
// {
// 	/* code */
// 	struct cache* cche_to_use = initialize_cache(3,3);
	
// 	// printf("%d\n", hash_function("Ashwin", 4));
// 	put_entry_in_cache(cche_to_use, "Ashwin", "Ashwin");
// 	put_entry_in_cache(cche_to_use, "Mansi", "Ashwin");
// 	put_entry_in_cache(cche_to_use, "Munmun", "Ashwin");
// 	put_entry_in_cache(cche_to_use, "Uditi", "Ashwin");
// 	put_entry_in_cache(cche_to_use, "Aishwarya", "Ashwin");
// 	put_entry_in_cache(cche_to_use, "Kiera", "Ashwin");
// 	print_cache(cche_to_use);
// 	return 0;

// }

