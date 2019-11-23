#pragma once
char error_codes [10][128] = {
	"Network Error: Could not send data",
	"Network Error: Could not receive data",
	"Network Error: Could not connect",
	"Network Error: Could not create socket",
	"XML Error: Received unparseable message",
	"Oversized key",
	"Oversized value",
	"IO Error",
	"Does not exist",
	"Unknown Error: Unknown error occured"
};

char success_message[128] = "Success";