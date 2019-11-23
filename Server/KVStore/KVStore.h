#ifndef KVSTORE_H
#define KVSTORE_H

#include "../XMLlib/XMLlib.h"

int KVStore_init();
extern int KVStore_dumpToFile(struct QUERY *q);
extern int KVStore_restoreFromFile(struct QUERY *q);
int KVStore_complete();

#define MAX_KEY_SIZE 	300
#define MAX_VALUE_SIZE 	262200
#endif