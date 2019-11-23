#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "KVStore.h"
#include "../XMLlib/XMLlib.h"

#define MAX_KEY_SIZE 	300
#define MAX_VALUE_SIZE 	262200
#define DB_FILE_NAME 	"db.txt"
#define TYPE_KEY		0
#define TYPE_VALUE		1

FILE *db_file;
int max_index = 0;	//0 starting index of last key

struct META {
	int valid;
	int index;
	char key[MAX_KEY_SIZE];
	struct META *next;
};

struct META *meta = NULL;


// Both xml and data must be valid pointer and non empty string
int xml_to_data(char *xml, char *data)
{
	// printf("xml_to_data\n");
	// printf("xml is %s\n", xml);
	int i, begin_pos = 0;
	char ch;

	if (!xml || !strlen(xml)){
		// printf("KVStore:xml_to_data Called with empty xml\n");
		return -1;
	}
	if (!data){
		// printf("KVStore:xml_to_data Called with NULL data\n");
		return -1;
	}

	for (i = 0; xml[i] != '>' && i < strlen(xml); i++);
	i++;	// Point i to next to >

	begin_pos = i;
	// printf("i begin %d\n", i);
	for (; xml[i] != '<' && i < strlen(xml); i++){
		// printf("%d ", i);
		data[i - begin_pos] = xml[i];
	}
	// printf("i end %d\n", i);
	data[i - begin_pos] = '\0';
	// printf("xml_to_data: data is %s\n", data);
	return 1;
}

int add_key_in_mdata (char *key, int index)
{
	// printf("add_key_in_mdata\n");
	struct META *temp_m;
	if (!key || !strlen(key)){
		// printf("KVStore:add_key_in_mdata called withe NULL or empty key\n");
		return -1;
	}
	if (!meta) {
		meta = (struct META *)malloc(sizeof(struct META));
		meta->index = index;
		meta->valid = 1;
		strcpy(meta->key, key);
		meta->next = NULL;
	}
	else {
		temp_m = (struct META *)malloc(sizeof(struct META));
		temp_m->index = index;
		temp_m->valid = 1;
		strcpy(temp_m->key, key);
		temp_m->next = meta;
		meta = temp_m;
	}
	return 1;
}

// This function must be called from KVStore_init during initialization
int construct_mdata(FILE *fp)
{
	rewind(db_file);
	// printf("construct_mdata\n");
	if (!fp){
		// printf("KVStore: construct_mdata called with Null FP\n");
		return -1;
	}

	char temp[MAX_VALUE_SIZE];
	char key[MAX_KEY_SIZE];
	int index = 3;
	fgets(temp, MAX_VALUE_SIZE, fp); // Consume self declaration
	fgets(temp, MAX_VALUE_SIZE, fp); // Consume <KVStore>
	fgets(temp, MAX_VALUE_SIZE, fp); // Consume <KVPair> if present
	// Get key till there is a key
	while(fgets(temp, MAX_VALUE_SIZE, fp)){
		xml_to_data(temp, key);
		// printf("%s, %s\n", temp, key); // DEBUG, to print the retrived key
		max_index = index;
		add_key_in_mdata(key, index);
		index += 4;
		fgets(temp, MAX_VALUE_SIZE, fp); // <Value>
		fgets(temp, MAX_VALUE_SIZE, fp); // </KVPair>
		fgets(temp, MAX_VALUE_SIZE, fp); // <KVPair>
	}
	// printf("max_index = %d\n", max_index);
	return 1;
}

// This function must be called to remove spaces from db xml file before exit.
int compact_db(FILE *fp)
{
	// printf("compact_db\n");
	rewind(fp);
	FILE *bp = fopen("backup.txt", "w+");
	char temp[MAX_VALUE_SIZE];

	if (!fp || !bp){
		// printf("KVStore:compact_db: Unable to open db file or backup file\n");
		return -1;
	}

	while(fgets(temp, MAX_VALUE_SIZE, fp)){
		// printf("%s\n", temp);
		if (strncmp(&temp[2], "       ", 7)){
			fputs(temp, bp);
		}
	}
	fclose(bp);
	fclose(fp);
	system("cp ./backup.txt ./db.txt");  // Must change
	return 1;
}

int KVStore_complete()
{	
	// printf("KVStore_complete\n");
	char temp[12];
	fseek(db_file, -10, SEEK_END);
	fgets(temp, strlen("</KVStore>") + 1, db_file);
	// printf("%s\n", temp);	// DEBUG, to check </KVStore> is detected or not.
	if (strcmp(temp, "</KVStore>")){
		//Something is written and </KVStore> is replaced. Write it again.
		fseek(db_file, 0, SEEK_END);
		fputs("</KVStore>", db_file);
	}
	// fclose(db_file);
	return(compact_db(db_file));
}



void fill_space(char *buff, int size)
{
	// printf("fill_space\n");
	int i;
	if (!buff || !size){
		// printf("KVStore: fill_space: buff NULL or size 0\n");
		return;
	}
	for (i = 0; i < size; i++){
		buff[i] = ' ';
	}
	buff[i-1] = '\n';
	buff[i] = '\0';
	return;
}

int delete_key_from_mdata(char *key)
{
	// printf("delete_key_from_mdata\n");
	struct META *temp_m = meta;
	char temp[MAX_VALUE_SIZE];
	char blank_space[MAX_VALUE_SIZE];
	int index, len;
	if (!key || !strlen(key)){
		// printf("KVStore:add_key_in_mdata called withe NULL or empty key\n");
		return -1;
	}

	while(temp_m){
		// Key found in metadata
		if (!strcmp(temp_m->key, key)){
			temp_m->valid = 0;
			index = temp_m->index - 1; // Point index at <KVPair>
			while(index--){
				fgets(temp, MAX_VALUE_SIZE, db_file);
			}
			// now file pointer is at begining of <KVPair>
			fgets(temp, MAX_VALUE_SIZE, db_file);
			fseek(db_file, -strlen(temp), SEEK_CUR);
			fill_space(blank_space, strlen(temp));
			fputs(blank_space, db_file);  // <KVPair> Gone

			fgets(temp, MAX_VALUE_SIZE, db_file);
			fseek(db_file, -strlen(temp), SEEK_CUR);
			fill_space(blank_space, strlen(temp));
			fputs(blank_space, db_file);  // <Key> .. </Key> Gone

			fgets(temp, MAX_VALUE_SIZE, db_file);
			fseek(db_file, -strlen(temp), SEEK_CUR);
			fill_space(blank_space, strlen(temp));
			fputs(blank_space, db_file);  // <Value> .. </Value> Gone

			fgets(temp, MAX_VALUE_SIZE, db_file);
			fseek(db_file, -strlen(temp), SEEK_CUR);
			fill_space(blank_space, strlen(temp));
			fputs(blank_space, db_file);  // </KVPair> Gone

			fflush(db_file);

			return 1;
		}
		temp_m = temp_m->next;
	}
	// printf("KVStore:delete_key_from_mdata called with non existing key\n");
	return -1;
}

// Find a key in metadata file and return it's position. If not found return -1
int find_key_in_mdata(char *key)
{
	// printf("find_key_in_mdata\n");
	char temp[MAX_KEY_SIZE];
	struct META *temp_m = meta;

	if (!key || !strlen(key)){
		// printf("KVStore:find_key_in_mdata called withe NULL or empty key\n");
		return -1;
	}

	while(temp_m){
		if(!strcmp(temp_m->key, key) && temp_m->valid){
			return temp_m->index;
		}
		temp_m = temp_m -> next;
	}

	return -1;	// Key not present
}


int data_to_xml(char *xml, char *data, int type)
{
	// printf("data_to_xml\n");
	if (!xml){
		// printf("KVStore:data_to_xml called with NULL xml\n");
		return -1;
	}

	if (!data || strlen(data)){
		// printf("KVStore:data_to_xml called with NULL or empty data\n");
		return -1;
	}

	xml[0] = '\0';
	switch(type)
	{
		case(TYPE_KEY):
		{
			strcpy(xml, "<key>");
			strcat(xml, data);
			strcat(xml, "</key>");
			break;	
		}
		case(TYPE_VALUE):
		{
			strcpy(xml, "<data>");
			strcat(xml, data);
			strcat(xml, "</data>");
			break;
		}
		default:
			// printf("KVStore:data_to_xml some unknown type is given\n");
			return -1;
	}
	return 1;
}

int KVStore_init ()
{
	char temp[MAX_VALUE_SIZE];
	int i;

	db_file = fopen(DB_FILE_NAME, "r+");
	if (!db_file){
		// Trying to create if the file is not existing
		db_file = fopen(DB_FILE_NAME, "w");
		fclose(db_file);
		db_file = fopen(DB_FILE_NAME, "r+");
		if(!db_file)
			return -1;
	}

	if (construct_mdata(db_file) < 0){
		// printf("KVStore:KVStore_init:construct_mdata returned -1. Exiting.\n");
		exit(-1);
	}

	// Create 1st two line
	rewind(db_file);
	if (!fgets(temp, MAX_KEY_SIZE, db_file)){
		fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<KVStore>\n</KVStore>", 
			db_file);
	}

	return 1;
}

// Copy key or value from a line number, 0 starting
int restore_data_from_lineno(char *data, int lineno)
{
	char temp[MAX_VALUE_SIZE];
	int i = 0;

	if (lineno < 3){
		// printf("KVStore:restore_data_from_lineno: Invalid lineno given\n");
		return -1;
	}
	if (!data){
		// printf("KVStore:restore_data_from_lineno: NULL data\n");
		return -1;
	}

	rewind(db_file);
	while(i < lineno){
		fgets(temp, MAX_VALUE_SIZE, db_file);
		i++;
	}
	fgets(data, MAX_VALUE_SIZE, db_file);

	return 1;
}

int KVStore_dumpToFile(struct QUERY *q)
{
	int i = 0, key_in_mdata = -1;
	rewind(db_file);

	// This checkig that key or value has some ENTER in it which it should not
	for (; i < strlen(q->key); i++){
		if(q->key[i] == '\n'){
			// printf("KEY has ENTER\n");
		}
	}
	for(i = 0; i < strlen(q->value); i++){
		if(q->value[i] == '\n'){
			// printf("Value has ENTER\n");
		}
	}

	// printf("DUMP: Type: %d, KEY: %s, VALUE: %s\n", q->type, q->key, q->value);

	if (!q || !q->key || !strlen(q->key)){
		// printf("KVStore_dumpToFile: Called with invalid struct query or key\n");
		q->type = ERROR;
		return -1;
	}

	if (q->type == PUT && (!q->value || !strlen(q->value))){
		// printf("KVStore_dumpToFile: PUT called without message");
		q->type = ERROR;
		return -1;
	}
	
	if (q->type != PUT && q->type != DEL){
		// printf("KVStore_dumpToFile: Called with neigther PUT not DEL\n");
		return -1;
	}
	key_in_mdata = find_key_in_mdata(q->key);

	// DEL called with non existing key
	if (q->type == DEL && key_in_mdata == -1){
		// printf("KVSotre_dempToFile: Delete called with non existing key\n");
		q->type = UNSUCCESS;
		strcpy(q->message, "Does not exist");
		return 1; //NOTE: Key not present is not an error, so returning 1
	}

	// PUT calles with existing key
	if (q->type == PUT && key_in_mdata != -1){
		// printf("KVStore_dumpToFile: Key already present for PUR req\n");
		q->type = SUCCESS;
		strcpy(q->message,"Unknown Error: Key present for PUT req");
		return -1; // This is an error. So, -1
	}

	// printf("KEY: %s, VALUE: %s\n", q->key, q->value);

	// Now doing actual PUT or DEL
	if (q->type == DEL){
		if(delete_key_from_mdata(q->key) < 0){
			// printf("KVSotre_dempToFile: delete_key_from_mdata returned -1");
			q->type = ERROR;
			strcpy(q->message, "Error: Unknown Error");
			return -1;
		}
		q->type = SUCCESS;
		strcpy(q->message, "Success");
		return 1;
	}
	else if (q->type == PUT){
		// Writting the value in 
		fseek(db_file, -10, SEEK_END); 	// 10 is size of </KVStore>
		fprintf(db_file, "\t<KVPair>\n");
		fprintf(db_file, "\t\t<Key>%s</Key>\n", q->key);
		fprintf(db_file, "\t\t<Value>%s</Value>\n", q->value);
		fprintf(db_file, "\t</KVPair>\n");
		fprintf(db_file, "</KVStore>");
		fflush(db_file);

		// Writting the Key_value and index in db_file
		// Note it's 0 starting index. So, first index will be 3.
		if (!max_index)
			max_index += 3;
		else
			max_index += 4;

		// printf("KVSotre_dempToFile: assigned index = %d\n", max_index);
		add_key_in_mdata(q->key, max_index);
		// fseek(md_file, 0, SEEK_END);
		// fprintf(md_file, "%s\n", q->key);
		// fprintf(md_file, "%d\n", max_index);

		// Generating reply
		q->type = SUCCESS;
		strcpy(q->message, "Success");
	}
	return 1;
}

int KVStore_restoreFromFile(struct QUERY *q)
{
	// printf("RESTORE: Type: %d, KEY: %s, VALUE: %s\n", q->type, q->key, q->value);
	rewind(db_file);
	int key_in_mdata = -1;
	char key_xml[MAX_KEY_SIZE];
	char value_xml[MAX_VALUE_SIZE];

	if (!q || !q->key || !strlen(q->key)){
		// printf("KVStore_restoreFromFile called with invalid query\n");
		return -1;
	}

	if (q->type != GET){
		// printf("KVStore_restoreFromFile called for something other than GET\n");
		return -1;
	}

	key_in_mdata = find_key_in_mdata(q->key);

	if (key_in_mdata == -1){
		q->type = UNSUCCESS;
		// printf("Does not exist\n");
		strcpy(q->message, "Does not exist");
		return 1;	// NOTE: Key not present is not an error, so returning 1
	}
	else {
		if(restore_data_from_lineno(key_xml, key_in_mdata) < 0){
			// printf("KVStore_restoreFromFile: Unable to restore key\n");
			q->type = ERROR;
			strcpy(q->message, "Unknown Error: Unable to restore key");
			return -1; 	// This is an error. So returning -1
		}
		else if (restore_data_from_lineno(value_xml, key_in_mdata + 1) < 0){
			// printf("KVStore_restoreFromFile: Unable to restore data\n");
			q->type = ERROR;
			strcpy(q->message, "Unknown Error: Unable to restre data");
			return -1;
		}
	}

	// At this point no error occured. Key and data are present in xml form
	if (xml_to_data(key_xml, q->key) < 0){
		// printf("KVStore_restoreFromFile: Unable to convert xml key back\n");
		q->type = ERROR;
		strcpy(q->message, "Unknown Error: Unable to convert xml key back");
		return -1;  // This is an error. So reply with -1
	}
	else if (xml_to_data(value_xml, q->value) < 0){
		// printf("KVStore_restoreFromFile: Unable to convert xml value back\n");
		q->type = ERROR;
		strcpy(q->message, "Unknown Error: Unable to convert xml value back");
		return -1; // This is an error
	}

	// printf("REPLYING: Key: %s, Value: %s\n", q->key, q->value);
	// At this point QUERY is filled with correct key and value
	q->type = GETR;
	strcpy(q->message, "Success");
	return 1;
}