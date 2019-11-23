#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define KEY_SIZE 257
#define VALUE_SIZE 262144
#define MESSAGE_SIZE 128

bool debug_mode = false;

#include "XMLlib.h"

void XMLlib_debugon()
{
	debug_mode = true;
}

void XMLlib_debugoff()
{
	debug_mode = false;
}

void debug_print (char *buff)
{
	if (!debug_mode)
		return;
	if (!buff)
		printf("debug_print: called with empty string\n");
	else
		printf("%s\n", buff);
}

// Initialize q structure. For internal use only.
// A valid allocated pointer must be given.
int XMLlib_qinit (struct QUERY *q)
{
	if (!q)
		return -1;
	// Put type to error because it's not decided
	q -> type = ERROR;

	q -> key = (char *)malloc (sizeof(char) * KEY_SIZE);
	if (!q -> key){
		debug_print("q_int: malloc error in key");
		return -1;
	}

	q->value = (char *)malloc (sizeof(char) * VALUE_SIZE);
	if (!q -> value){
		debug_print("q_int: malloc error in value");
		return -1;
	}

	q -> message = (char *) malloc(sizeof(char) * MESSAGE_SIZE);
	if (!q -> message){
		debug_print("q_int: malloc error in message");
		return -1;
	}

	return 1;
}

int XMLlib_qfree (struct QUERY * q)
{
	if (!q){
		debug_print("XMLlib_q_free: called with NULL pointer");
		return -1;
	}
	if (q -> key)
		free(q -> key);
	if (q -> value)
		free(q->value);
	if (q -> message);
		free(q -> message);
	return 1;
}

// Convert xml to structured data. For server to parse q
int XMLlib_parse(char *xml, struct QUERY *q)
{
	// printf("XML received: \t\t ==%s==\n", xml);
	int i = 0, j;
	char ch, req_type[16];
	char tag [8]; 	// Diff GETR & SUCCESS

	// Safety check
	if (!q || !xml){
		debug_print("XMLlib_parse: called qith null xml or null q");
		return -1;
	}
	if (!strlen(xml)){
		debug_print("XMLlib_parse: called with 0 length xml");
		return -1;
	}

	if (!q->key || !q->value || !q->message){
		debug_print("XMLlib_parse: q_init not done");
		return -1;
	}
	q-> key [0] = '\0';
	q -> value [0] = '\0';
	q -> message[0] = '\0';

	while ((xml[i++] != '>') && (i < strlen(xml)));	// Consume self declaration
	while ((xml[i++] != '"') && (i < strlen(xml)));	// Go to Attribute

	// Copy KVMessage type in req_type
	req_type[0] = '\0';
	for (j = 0; xml[i] != '"' && xml[i] != '\0'; j++, i++)
		req_type[j] = xml[i];
	req_type[j] = '\0';
	// i it at "

	if (!strlen(req_type)){
		q->type = ERROR;
		debug_print("XMLlib_parse 0 size req_type");
		return -1;
	}

	// Processing all REQUESTS first, not replys
	if (strcmp(req_type, "resp")){
		// i it at " No need to move. While will take care
		// Need to consume [> <Key>] i.e two >
		while ((xml[i++] != '>') && (i < strlen(xml)));
		while ((xml[i++] != '>') && (i < strlen(xml)));

		// Now i is at start of Key
		for (j = 0; xml[i] != '<' && xml[i] != '\0'; j++, i++)
			q->key[j] = xml[i];
		q->key[j] = '\0';

		// Go to parse value only for PUT request

		if (!strcmp(req_type, "getreq")){
			q -> type = GET;
			q -> value[0] = '\0';
			return 1;
		}
		else if (!strcmp(req_type, "delreq")){
			q -> type = DEL;
			q -> value[0] = '\0';
			return 1;
		}
		else if (!strcmp(req_type, "putreq")){
			q -> type = PUT;
			// Need to consume [> <Value>] i.e two >
			while ((xml[i++] != '>') && (i < strlen(xml)));
			while ((xml[i++] != '>') && (i < strlen(xml)));
			for (j = 0; xml[i] != '<' && xml[i] != '\0'; j++, i++)
				q->value[j] = xml[i];
			q->value[j] = '\0';
			return 1;
		}
		else{
			// req_type is not "resp", "putreq", "qetreq", "delreq"
			debug_print("XMLlib_parse: req_type is not getreq, putreq, delreq or resp");
			return -1;
		}
	}
	// Done processing requests. Now RESPONSE messages
	else {
		// i it at "
		// GO to < and check whether "Key" tag is there or not
		while (xml[i++] != '<');
		strncpy (tag, &xml[i], 3);
		tag[3] = '\0';			// Note: strncpy doesn't put NULL

		// i is at K or M after < i.e. <Key or <Message

		if (!strcmp (tag, "Key")){
			// process GET Reply. Note: <Key> only present in GETR "resp"
			q -> type = GETR;
			// Consume till >
			while ((xml[i++] != '>') && (i < strlen(xml)));

			// Now i at start of Key-content.
			// Copy till <
			for (j = 0; xml[i] != '<' && xml[i] != '\0'; j++, i++)
				q->key[j] = xml[i];
			q->key[j] = '\0';

			// Now i at < of </Key>. Go till > of <Value>
			while ((xml[i++] != '>') && (i < strlen(xml)));
			while ((xml[i++] != '>') && (i < strlen(xml)));
			for (j = 0; xml[i] != '<' && xml[i] != '\0'; j++, i++)
				q->value[j] = xml[i];
			q->value[j] = '\0';
		}
		else if (!strcmp(tag, "Mes")) {
			// process SUCCESS or UNSUCCESS rpy
			// Consume till >
			while ((xml[i++] != '>') && (i < strlen(xml)));
			for (j = 0; xml[i] != '<' && xml[i] != '\0'; j++, i++)
				q->message[j] = xml[i];
			q->message[j] = '\0';

			// Check success? decide reply type
			if (!strcmp(q->message, "Success"))
			{
				q->type = SUCCESS;
			}
			else
			{
				q->type = UNSUCCESS;
			}
		}
		else {
			// <--- is neigther <Key> nor <Messsage>
			debug_print("XMLlib_parse: req_type is resp but no <Key> or <Messsage> found");
			return -1;
		}

		// req_type was resp for sure
		return 1;
	}
	debug_print("XMLlib_parse: reached at end for unknown reason");
	return -1;	// Never reach. Eles may compiler warn
}

int XMLlib_build(char *xml, struct QUERY * q)
{
	if (!xml || !q) {
		debug_print("XMLlib_build: null xml or null q");
		return -1;
	}
	// Copying the fixed part till first " before attribute
	strcpy(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <KVMessage type=\"");

	// Copying request type for all q
	if (q->type == GET)
		strcat(xml, "getreq\"> <Key>");
	else if (q->type == PUT)
		strcat(xml, "putreq\"> <Key>");
	else if (q->type == DEL)
		strcat(xml, "delreq\"> <Key>");
	else if (q->type == GETR || q->type == SUCCESS || q->type == UNSUCCESS)
		strcat(xml, "resp\"> ");
	else{
		debug_print("XMLlib_build: (1) q has unknown type");
		return -1;
	}

	// KEY for GET, PUT, DEL and VALUE for PUT, GETR
	if (q->type == GET || q->type == PUT || q->type == DEL || q->type == GETR){
		if (q->type == GETR)
			strcat(xml, "<Key>");
		strcat(xml, q->key);
		strcat(xml, "</Key> ");
		if (q->type == PUT || q->type == GETR){
			strcat(xml, "<Value>");
			strcat(xml, q->value);
			strcat(xml, "</Value> ");
		}

	}
	// Successful PUTR or DELR
	else if (q-> type == SUCCESS || q->type == UNSUCCESS) {
		strcat(xml, "<Message>");
		strcat(xml, q->message);
		strcat(xml, "</Message> ");
	}
	else {
		debug_print("XMLlib_build: (2): q has unknown type");
		return -1;
	}

	// Copying the last root close fixed part
	strcat(xml, "</KVMessage>");

	// Return 1 on success
	// printf("XMLlib_build builded ==%s== size = %ld\n", xml, strlen(xml));
	return 1;
}


void print_query(struct QUERY* q) {
	printf("%d\n", q->type);
	printf("%s\n", q->key);
	printf("%s\n", q->value);
	// printf("%d\n", q->type);
}