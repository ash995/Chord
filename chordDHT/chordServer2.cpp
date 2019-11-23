#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "gen-cpp/chordDHT.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

int MY_ID = 30;

class chordDHT : public chordDHTIf {
public:
	chordDHT() = default;
	int16_t find_successor (int16_t id) override 
	{
		if (id > MY_ID){
			cout << "I'm answering, " << "MY_ID is 2, Query_id is " << id << endl;
			return 2;
		}
		else {
			std::shared_ptr<TTransport> socket(new TSocket("127.0.0.1", 9000));
			std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
			std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
			chordDHTClient client(protocol);
			try {
				transport->open();
				return client.find_successor(id);
			}
			catch ( ... ){
				printf("Error occured in node id %d\n", MY_ID);
			}
		}
	}
};

void *startServer(void *args)
{
	TSimpleServer server(
	std::make_shared<chordDHTProcessor>(std::make_shared<chordDHT>()),
	std::make_shared<TServerSocket>("127.0.0.2", 9000),
	std::make_shared<TBufferedTransportFactory>(),
	std::make_shared<TBinaryProtocolFactory>());
	cout << "Starting the server..." << endl;
	server.serve();
}

int main() 
{
	// This server only allows one connection at a time, but spawns no threads
	pthread_t tid = 2;
	pthread_create(&tid, NULL, startServer, (NULL));
	sleep(1);

	// Starting actual code
	int16_t run = 1;
	int16_t fromId;
	chordDHT chord;
	// cout << "Enter this server ID: ";
	// cin >> MY_ID;
	while(run){
		cout << "Enter Key ID ";
		cin >> run;
		fromId = chord.find_successor(run);
		cout << "Returned from ID " << fromId << endl;
	}

	cout << "Done." << endl;
	return 0;
}