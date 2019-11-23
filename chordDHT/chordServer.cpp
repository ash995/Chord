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
#include <cmath>

#include "gen-cpp/chordDHT.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

#define CHORD_PORT 	9000
#define CHORD_M		16
#define NTHREAD		4

struct chord_ip{
	char ip[16];
};

class chordDHT : public chordDHTIf {
private:
	NODE_INFO node_info;
	int next_f;

	int open_range(int start, int end, int val)
	{
		if (val == -1 || start == -1 || end == -1)
			return 0;
		else if (end == start){
			if(val == start)
				return 0;
			else return 1;
		}
		else if (end > start) {
			if (start < val && end > val) {
				return 1;
			}
			else
				return 0;
		}
		else {
			if (val <= start && val >= end) {
				return 0;
			}
			else
				return 1;
		}
	}

	int close_range(int start, int end, int val) {
		if (val == -1 || start == -1 || end == -1)
			return 0;
		if (end == start)
			return 1;
		else if (end > start) {
			if (start < val && end >= val) {
				return 1;
			}else
				return 0;
		}else {
			if (val <= start && val > end) {
				return 0;
			}else
				return 1;
		}
	}

public:
	// char known_ip[17];
	chordDHT(int my_id, const char *my_ip, int join, const char *known_ip)
	{
		int i;
		if(!my_ip || !strlen(my_ip) || strlen(my_ip) > 15){
			printf("chordDHT: Error in IP. Returning.\n");
		}
		// strcpy(this->known_ip,known_ip);
		node_info.me.id = my_id;
		node_info.me.ip = my_ip;
		// strcpy(node_info.me.ip, my_ip);

		node_info.pred.id = -1;
		node_info.pred.ip[0] = '\0';

		if(!join){
			node_info.succ.id = my_id;
			// strcpy(node_info.succ.ip, my_ip);
			node_info.succ.ip = my_ip;
		}
		else{
			std::shared_ptr<TTransport> socket(new TSocket(known_ip, CHORD_PORT));
			std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
			std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
			chordDHTClient client(protocol);
			// node_info.succ = find_successor(my_id);
			try {
				transport->open();
				client.find_successor(node_info.succ, my_id);
			}
			catch ( ... ){
				cout << "chordDHT: Error cought during find_successor" << endl;
			}
			transport->close();
		}

		// Starting finger table
		next_f = 1;

		NODE nullNode;
		nullNode.id = -1;
		for (i = 0; i < 16; i++){
			if(!join){
				// cout << "node_info.me " << node_info.me.id << " " << node_info.me.ip << endl; 
				node_info.f_table.push_back(node_info.me);
			}
			else{
				node_info.f_table.push_back(nullNode);
			}
		}
	}

	// node = for which node we want pred, return = pred's NODE
	void get_pred(NODE& _return, const NODE& node) override
	{
		if (node.id == node_info.me.id){
			_return = node_info.pred;
		}else{
			std::shared_ptr<TTransport> socket(new TSocket(node.ip, CHORD_PORT));
			std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
			std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
			chordDHTClient client(protocol);
			try {
				transport->open();
				client.get_pred(_return, node);
			}
			catch( ... ){
				cout << "get_succ_pred: Error catched" << endl;
			}
			transport->close();
		}
	}

	// stabilize should put node = succ, pred = me
	void notify(const NODE& node, const NODE& pred) override
	{
		if(node.id == node_info.me.id){
			cerr << "Notify if" << endl;
			if(node_info.pred.id == -1 || open_range(node_info.pred.id, node_info.me.id, pred.id)){
				node_info.pred = pred;
			}
		} else {
			cerr << "Notify else" << endl;
			std::shared_ptr<TTransport> socket(new TSocket(node.ip, CHORD_PORT));
			std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
			std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
			chordDHTClient client(protocol);
			try {
				transport->open();
				client.notify(node, pred);
			}
			catch ( ... ){
				cout << "notify: Error catch" << endl;
			}
			transport->close();
		}
	}

	void stabilize()
	{
		NODE x;
		while(1){
			get_pred(x, node_info.succ);
			if(x.id != -1 && open_range(node_info.me.id, node_info.succ.id, x.id)){
				cerr << "stabilize if" << endl;
				node_info.succ = x;
			}
			cerr << "Stabilize else" << endl;
			notify(node_info.succ, node_info.me);
			sleep(3);
		}
	}

	void find_successor (NODE& _return, const int64_t id) override 
	{
		cerr << "Find successor called id = " << id << endl;
		if (close_range(node_info.me.id, node_info.succ.id, id)){
			cerr << "Find_successor In if" << endl;
			_return = node_info.succ;
		}
		else{
			cerr << "Find_successor in else id = " << id << endl;
			NODE x = closest_preceding_node(id);
			cerr << "going to ip " << x.ip << endl;
			if (x.id == node_info.me.id){
				_return = node_info.succ;
				return;
			}
			std::shared_ptr<TTransport> socket(new TSocket(x.ip, CHORD_PORT));
			std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
			std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
			chordDHTClient client(protocol);
			try {
				cerr << "Find_successor opening transport" << endl;
				transport->open();
				client.find_successor(_return, id);
				cerr << "Find_successor transport done" << endl;
			}
			catch( ... ){
				cout << "find_successor: Error catched" << endl;
			}
			transport->close();
		}
	}

	NODE closest_preceding_node(int id)
	{
		int i;
		for (i = 15; i >= 0; i--){
			if (node_info.f_table[i].id == -1)
				continue;
			if (open_range(node_info.me.id, id, node_info.f_table[i].id))
				return node_info.f_table[i];
		}
		return node_info.me;
	}

	void fix_fingers()
	{
		sleep(2);
		next_f = 1;
		double power;
		while(1){
			next_f ++;
			if (next_f > CHORD_M)
				next_f = 1;
			power = pow(2,next_f - 1);
			find_successor(node_info.f_table[next_f - 1], node_info.me.id + (int)power);
			// sleep(1);
		}
	}

	string get_my_ip()
	{
		return node_info.me.ip;
	}

	void *print_status(){
		int i;
		while(1){
			cout << "my_id: " << node_info.me.id << " my_ip: " << node_info.me.ip << endl;
			cout << "succ_id: " << node_info.succ.id << " succ_ip: " << node_info.succ.ip << endl;
			cout << "pred_id: " << node_info.pred.id << " pred_ip: " << node_info.pred.ip << endl;
			cout << "Finger table ===================" << endl;
			for (i = 0; i < CHORD_M; i++){
				cout << "node-" << i;
				cout << " my_id: " << node_info.f_table[i].id ;
				cout << " my_ip: " << node_info.f_table[i].ip << endl;
			}
			sleep(10);
		}
		cout << "print_status exiting" << endl;
		return NULL;
	}
};

// Global cord instance shared pointer
std::shared_ptr<chordDHT> chord;

// struct startServerArgs{
// 	char ip[17];
// 	void *chord;
// };

void *startServer(void *args)
{
	string my_ip = (*(std::shared_ptr<chordDHT> *)args)->get_my_ip();
	TThreadedServer server(
	std::make_shared<chordDHTProcessor>(*(std::shared_ptr<chordDHT> *)args),
	std::make_shared<TServerSocket>(my_ip, CHORD_PORT),
	std::make_shared<TBufferedTransportFactory>(),
	std::make_shared<TBinaryProtocolFactory>());
	cout << "Starting the server..." << endl;
	server.serve();	
}

void *start_fix_fingures(void *args)
{
	(*(std::shared_ptr<chordDHT> *)args)->fix_fingers();
	return NULL;
}

void *start_stabilize(void *args)
{
	(*(std::shared_ptr<chordDHT> *)args)->stabilize();
	return NULL;
}

void *start_print_status(void *args)
{
	(*(std::shared_ptr<chordDHT> *)args)->print_status();
	return NULL;	
}

int chordInit(int my_id, char *my_ip, char *known_ip, int join) 
{	

	chord = std::make_shared<chordDHT>(my_id, (const char *)my_ip, join, (const char *)known_ip);

	pthread_t tid[NTHREAD];
	for(int i = 0; i < NTHREAD; i++)
		tid[i] = i;

	pthread_create(&tid[0], NULL, startServer, (void *)&chord);
	pthread_create(&tid[1], NULL, start_stabilize, (void *)&chord);
	pthread_create(&tid[2], NULL, start_fix_fingures, (void *)&chord);
	pthread_create(&tid[3], NULL, start_print_status, (void *)&chord);

	sleep(2);

	cout << "Chord initialized Done." << endl;
	pthread_join(tid[0], NULL);		// Remove
	return 1;
}

chord_ip find_successor(int id){
	NODE _return;
	chord_ip cip;
	chord->find_successor(_return, id);
	strcpy(cip.ip, _return.ip.c_str());
	return cip;
}

// int main(int argc, char **argv) 
// {	
// 	int my_id, join;

// 	if (argc != 5){
// 		cout << "./exe my_id my_ip known_ip join " << endl;
// 		exit(100);
// 	}
// 	my_id = atoi(argv[1]);
// 	join = atoi(argv[4]);

// 	chordInit(my_id, argv[2], argv[3], join);

// 	return 0;
// }

// int main(int argc, char **argv) 
// {	
// 	int my_id, join;

// 	if (argc != 5){
// 		cout << "./exe my_id my_ip known_ip join " << endl;
// 		exit(100);
// 	}
// 	my_id = atoi(argv[1]);
// 	// my_ip = argv[2];
// 	// known_ip = argv[3];
// 	join = atoi(argv[4]);

// 	// startServerArgs ssa;
// 	// strcpy(ssa.ip, argv[2])
// 	std::shared_ptr<chordDHT> chord = std::make_shared<chordDHT>(my_id, (const char *)argv[2], join, (const char *)argv[3]);

// 	pthread_t tid[NTHREAD];
// 	for(int i = 0; i < NTHREAD; i++)
// 		tid[i] = i;

// 	pthread_create(&tid[0], NULL, startServer, (void *)&chord);
// 	pthread_create(&tid[1], NULL, start_stabilize, (void *)&chord);
// 	pthread_create(&tid[2], NULL, start_fix_fingures, (void *)&chord);
// 	pthread_create(&tid[3], NULL, start_print_status, (void *)&chord);

// 	// Starting actual code
// 	sleep(1);
// 	int run = 1;
// 	// cout << "Enter this server ID: ";
// 	// cin >> run;
// 	while(run){
// 		NODE succ;
// 		cout << "Enter Key ID " ;
// 		cin >> run;
// 		chord->find_successor(succ, run);
// 		cout << "Returned from ID " << succ.id << endl;
// 	}

// 	cout << "Done." << endl;
// 	return 0;
// }