#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "XMLlib.h"

char *query_t[16] = {"GET", "PUT", "DEL", "GETR", "SUCCESS", "UNSUCCESS", "ERROR"};

int main ()
{
	XMLlib_debugon();
	struct QUERY query;
	XMLlib_qinit(&query);
	
	printf("Testing XMLlib_parse function with ALL 7 requests\n");

	char buff[262144] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <KVMessage type=\"getreq\"> <Key>My Get key</Key> </KVMessage>";
	XMLlib_parse(buff, &query);
	printf("Type: %s, Key: %s, Value: %s\n", query_t[query.type], query.key, query.value);
	
	strcpy(buff, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><KVMessage type=\"putreq\"><Key>My Put key</Key><Value>imp value</Value></KVMessage>");
	XMLlib_parse(buff, &query);
	printf("Type: %s, Key: %s, Value: %s\n", query_t[query.type], query.key, query.value);
	
	strcpy(buff, "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <KVMessage type=\"delreq\"> <Key>My Del key</Key> </KVMessage>");
	XMLlib_parse(buff, &query);
	printf("Type: %s, Key: %s, Value: %s\n", query_t[query.type], query.key, query.value);

	strcpy(buff, "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <KVMessage type=\"resp\"> <Key>My Del key</Key> </KVMessage>");
	XMLlib_parse(buff, &query);
	printf("Type: %s, Key: %s, Value: %s\n", query_t[query.type], query.key, query.value);

	strcpy(buff, "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <KVMessage type=\"resp\"> <Message>Success</Message> </KVMessage>");
	XMLlib_parse(buff, &query);
	printf("Type: %s, Message: %s\n", query_t[query.type], query.message);

	strcpy(buff, "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <KVMessage type=\"resp\"> <Message>This is UnSuccess Message</Message> </KVMessage>");
	XMLlib_parse(buff, &query);
	printf("Type: %s, Message: %s\n", query_t[query.type], query.message);

	printf("\nTesting XMLlib_build function.\n");

	query.type = GET;
	strcpy(query.key, "get_key");
	XMLlib_build(buff, &query);
	printf("%s\n", buff);

	query.type = PUT;
	strcpy(query.key, "put_key");
	strcpy(query.value, "put_valu: put Testing");
	XMLlib_build(buff, &query);
	printf("%s\n", buff);

	query.type = DEL;
	strcpy(query.key, "del_key");
	XMLlib_build(buff, &query);
	printf("%s\n", buff);

	query.type = GETR;
	strcpy(query.key, "getr_key");
	strcpy(query.value, "getr_value: test getr value");
	XMLlib_build(buff, &query);
	printf("%s\n", buff);

	query.type = SUCCESS;
	strcpy(query.message, "Success");
	XMLlib_build(buff, &query);
	printf("%s\n", buff);

	query.type = UNSUCCESS;
	strcpy(query.message, "This is a test UnSuccess message");
	XMLlib_build(buff, &query);
	printf("%s\n", buff);

	XMLlib_qfree(&query);
	return 0;
}