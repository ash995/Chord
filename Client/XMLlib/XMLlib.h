/*----------Query Types----------------
GET => Get request
PUT => Put request
DEL => Delete request
*R => Get/Put/Delete Reply
UNSUCCESS => Unsuccessful reply for GET/PUT/DEL
ERROR => Some other error. For Debug
----------------------------------------*/
#pragma once
#ifndef XMLLIB_H
#define XMLLIB_H
enum QUERY_t {GET, PUT, DEL, GETR, SUCCESS, UNSUCCESS, ERROR};

// Error Codes in the given sequence of Question
enum ERROR_t {NESend, NERecv, NEConn, NESock, XMLEr, OKey, OVal, IOErr,NTExist, UNErr};

// Pass valid pointer of this struct to the following functions
struct QUERY
{
	enum QUERY_t type;
	char *key;
	char *value;
	char *message;
};

// Parse the buffer and fill in QUERY *query. Caller must give valid ptr
int XMLlib_parse(char *xml, struct QUERY *query);
// Build the buffer from Query *query and Error code into buff
int XMLlib_build(char *xml, struct QUERY * q);
// Start Printing dubug messages
void XMLlib_debugon();
// Stop Printing debug messages, default
void XMLlib_debugoff();
// Free the query structure
int XMLlib_qfree (struct QUERY * query);
int XMLlib_qinit (struct QUERY * query);
void print_query(struct QUERY* q);
#endif