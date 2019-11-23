#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include "../XMLlib/XMLlib.h"

#define KEY_SIZE_M		256
#define VALUE_SIZE_M	262144
#define S_PORT 			9001		// Clint starts a server on this port
#define CLIENT_IP		"127.0.0.1"	// Clint starts a server on this IP
// #define PORT 			8000

int query_make (struct QUERY * query, char *buff)
{
	char ch;
	int i, j;

	if (!query || !buff) {
		// printf("query_make: called with empty query or empty buff\n");
		return -1;
	}

	if (!strncmp(buff, "GET", 3)){
		query->type = GET;
	}
	else if (!strncmp(buff, "PUT", 3)){
		query->type = PUT;
	}
	else if (!strncmp(buff, "DEL", 3)){
		query->type = DEL;
	}
	else{
		// printf("query_make: Unable to get query type\n");
		return -1;
	}

	for (i = 4, j = 0; (buff[i] != ',') && (i < (strlen(buff) - 1)); i++, j++){
		// printf("i= %d,buff_len= %ld\n", i, strlen(buff));
		fflush(stdout);
		query->key[j] = buff[i];
	}
	if (buff[i] == '\n' || buff[i] == ',')
		query->key[j] = '\0';
	else{
		query->key[j] = buff[i];
		query->key[j + 1] = '\0';
	}

	if (query->type == PUT){
		i++; 	// Consume the ','
		for (j = 0; i < strlen(buff) - 1; i++, j++)		// fgets put a \n at end
		{
			query->value[j] = buff[i];
		}
		query -> value[j] = '\0';
	}
	return 1;
}

int write_reply_to_file(struct QUERY *query, FILE * out_file)
{
	if (!query){
		printf("write_reply_to_file: null query given\n");
		return -1;
	}
	if (!out_file){
		printf("write_reply_to_file: null out_file\n");
		return -1;
	}

	switch(query->type){
		case(UNSUCCESS): fprintf(out_file, "error,%s\n", query->message); break;
		case(SUCCESS): fprintf(out_file, "success\n"); break;
		case(GETR): fprintf(out_file, "%s,%s\n", query->key, query->value ); break;
	}
	fflush(out_file);
	return 1;
}

void *serverThread(void *args)
{
	int client_fd, new_fd, nbyte_r, len;
	struct sockaddr_in client_addr, server_addr;
    int addrlen = sizeof(client_addr);
    char *xml = (char *)malloc((KEY_SIZE_M + VALUE_SIZE_M + 128)*sizeof(char));
    char clientip[20];
    struct QUERY query;
    FILE *out_file;

    out_file = (FILE *)args;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(S_PORT);
    inet_pton(AF_INET, CLIENT_IP, &client_addr.sin_addr);
    bind(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));
    listen(client_fd, 50);

	while(1){
	    new_fd = accept(client_fd, (struct sockaddr *)&server_addr,(socklen_t*)&addrlen);

    	strcpy(clientip, inet_ntoa(server_addr.sin_addr));
    	printf("Received reply from %s\n", clientip);

		// Now reading the reply length from server in variable len
		if (read(new_fd, &len, sizeof(int)) <= 0){
			printf("Network Error: Could not receive data\n");
			fprintf(out_file, "Network Error: Could not receive data\n");
			exit(-1);
		}
		// printf("length received %d\n", len);
		nbyte_r=0;
		while(len > nbyte_r){
			nbyte_r += read(new_fd, &xml[nbyte_r], len - nbyte_r);
			if (nbyte_r < 0){
				exit (-1);
			}
		}

		if (XMLlib_parse(xml, &query) < 0){
			exit(-1);
		}
		// printf("Parsing done\n");

		if (write_reply_to_file(&query, out_file) < 0){
			// printf("Client: Unable to write to o/p file\n");
			exit(-1);
		}
	}
}

int main (int argc, char **argv)
{
	FILE *in_file, *out_file;
	struct QUERY query;
	char *input; 		//[KEY_SIZE_M + VALUE_SIZE_M + 128];
	char *xml;			//[KEY_SIZE_M + VALUE_SIZE_M + 128];
	int socket_fd;
	struct sockaddr_in serv_addr;
	int nbyte_w, nbyte_r; 	// nbytes write(), read() done so far for a connection
	int nbyte_temp, len; 			// For temporary work
	int PORT;
	pthread_t tid = 1;

	if (argc != 5){
		printf("./client in_file out_file server_ip server_port\n");
		exit (-1);
	}

	in_file = fopen(argv[1], "r");
	out_file = fopen(argv[2], "w");

	if (!in_file || !out_file){
		printf("Error opening files\n");
		exit(-1);
	}

	PORT = atoi(argv[4]);

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("Network Error: Could not create socket\n");
        fprintf(out_file, "Network Error: Could not create socket\n");
        exit(-1); 
    }


    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, argv[3], &serv_addr.sin_addr) <= 0){ 
        // printf("Client: Invalid address given as command line argument. \n"); 
        exit(-1); 
    }

    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) { 
        printf("Network Error: Could not connect\n"); 
        fprintf(out_file, "Network Error: Could not connect\n");
        exit(-1); 
    }

	XMLlib_qinit(&query);
	input = (char *)malloc((KEY_SIZE_M + VALUE_SIZE_M + 128)*sizeof(char));
	xml = (char *)malloc((KEY_SIZE_M + VALUE_SIZE_M + 128)*sizeof(char));
	XMLlib_debugon();	// For dubug purpouse

	// Starting Client-server 
	pthread_create(&tid, NULL, serverThread, (void *)out_file);

	while (fgets(input, KEY_SIZE_M + VALUE_SIZE_M + 128, in_file)){
		if (query_make(&query, input) < 0){
			// printf("Unable to generate struct query from input\n");
			exit(-1);
		}

		xml[0] = '\0';
		if (XMLlib_build(xml, &query) < 0){
			// printf("client: XMLlib_build returned -1\n");
			return -1;
		}

		nbyte_r = 0;
		len = nbyte_w = strlen(xml) + 1;	// +1 for the '\0' at last of string

		// Writing length of the string on socket
		if ((nbyte_temp = write(socket_fd, &len, sizeof(int))) < 0){
			// printf("Client: socket broken\n");
			exit(-1);
		}

		// Send the whole xml buffer. Big xml may not be sent in 1 shot. So while()
		// printf("Sending:             %s\n", xml);

		while (nbyte_w > 0){
			// printf("in while nbyte_w = %d ", nbyte_w);

			if ((nbyte_temp = write(socket_fd, &xml[len - nbyte_w], nbyte_w)) < 0){
				printf("Network Error: Could not send data. exiting\n");
				fprintf(out_file, "Network Error: Could not send data\n");
				exit(-1);
			}
			// printf("One write done nbyte_temp = %d\n", nbyte_temp);
			nbyte_w -= nbyte_temp;
			if (nbyte_w < 0){
				// printf("Client: Some calculation error\n");
				exit(-1);
			}
		}


		// Receiving
		// Note: We've no way to know the size of reply in advance
		// So, we'll assume reply will come in one shot. Possible 
		// when both client and server is on localhost only.
		// ASK SIR

		// printf("Starting read\n");

		// // Now reading the reply length from server in variable len
		// if (read(socket_fd, &len, sizeof(int)) <= 0){
		// 	printf("Network Error: Could not receive data\n");
		// 	fprintf(out_file, "Network Error: Could not receive data\n");
		// 	exit(-1);
		// }
		sleep(2);
		// // printf("length received %d\n", len);
		// nbyte_r=0;
		// while(len > nbyte_r){
		// 	// printf("nbyte_r before read %d\n", nbyte_r);
		// 	nbyte_r += read(socket_fd, &xml[nbyte_r], len - nbyte_r);
		// 	// printf("received some xml with nbyte_r = %d\n", nbyte_r);
		// 	for (int k = 0; k < nbyte_r; k++){
		// 		if (xml[k] == '\0'){
		// 			// printf("%d th 0 received\n", k);
		// 		}
		// 		// printf("%c", xml[k]);
		// 	}
		// 	// printf("Some xml print done\n");
		// 	if (nbyte_r < 0){
		// 		// TO DO: This has to be handled carefully
		// 		// printf("Unable to read from socket\n");
		// 		exit (-1);
		// 	}
		// }

		// // printf("Reply before parse \t\t==%s==\n", xml);
		// // printf("Parsing start\n");
		// if (XMLlib_parse(xml, &query) < 0){
		// 	// printf("Client: Unable to parse received string. XMLlib_parse returned -1\n");
		// 	exit(-1);
		// }
		// // printf("Parsing done\n");

		// if (write_reply_to_file(&query, out_file) < 0){
		// 	// printf("Client: Unable to write to o/p file\n");
		// 	exit(-1);
		// }
		// printf("Write to file done\n");
	}
	close(socket_fd);
}