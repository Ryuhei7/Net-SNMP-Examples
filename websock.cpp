#include <stdio.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "civetweb.h"
#define HAVE_STRUCT_TIMESPEC
#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep((x)*1000)
#else
#include <unistd.h>
#endif
using namespace std;

/***************************************************
   callback function
****************************************************/
static int
websocket_client_data_handler(
	struct mg_connection *conn,
	int flags,
	char *data,
	size_t data_len,
	void *user_data)
{
  int is_text = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_TEXT);
	int is_bin = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_BINARY);
	int is_ping = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_PING);
	int is_pong = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_PONG);
	int is_close = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE);	

	cout << (long unsigned)data_len << "bytes of data from server." << endl;
	
	if (is_ping){
		mg_websocket_client_write(conn,MG_WEBSOCKET_OPCODE_PONG,data,data_len);
		return 1;
	}
	if (is_pong)return 1;
	if (is_text){};
	if (is_bin){};
	if (is_close){
		cout << "Goodbye" << endl;
		return 0;
	}
	return 1;
}

/*****************************************************
   close function
*****************************************************/
static void
websocket_client_close_handler(
	const struct mg_connection *conn,
	void *user_data)
{
	cout << "Client: Close handler" << endl;
}

/****************************************************
   connect function
*****************************************************/
void
run_websocket_client(
	const char *host,
	int port,
	int secure,
	const char *path,
	const char *greetings)
{
	char err_buf[100] = {0};
	struct mg_connection *client_conn = NULL;
	//int i;

	cout << "Connectin to " << host << ":" << port << endl;
	client_conn = mg_connect_websocket_client(
		host,
		port,
		secure,
		err_buf,
		sizeof(err_buf),
		path,
		NULL,
		websocket_client_data_handler,
		websocket_client_close_handler,
		NULL);

	if(client_conn == NULL) {
		cout << "mg_connect_websocket_client error " << err_buf << endl;
		return;
	}
	cout << "Connected" << endl;
	if(greetings) {
		cout << "Sending greetings" << endl;
		mg_websocket_client_write(client_conn,MG_WEBSOCKET_OPCODE_TEXT,greetings,strlen(greetings));
	} sleep(2);
/*
	for (i = 0; i < 5; i++){
		cout << "Sending PING" << endl;
		mg_websocket_client_write(client_conn,MG_WEBSOCKET_OPCODE_PING,(const char *)&i,sizeof(int));
	} sleep(2);
*/
	cout << "Sending close message" << endl;
	mg_websocket_client_write(client_conn,MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE,NULL,0); 
	sleep(5);

	mg_close_connection(client_conn);
	cout << "End of this connection" << endl;
}

/***************************************************
   main function
****************************************************/
int main(int argc, char *argv[]){
	const char *greetings = "Hello";
	const char *host = "echo.websocket.org";
	const char *path = "/";

/*
#if defined(NO_SSL)
	const int port = 80;
	const int secure = 1;
	mg_init_library(0);
#else
*/
	const int port = 443;
	const int secure = 1;
	mg_init_library(MG_FEATURES_SSL);
//#endif
	run_websocket_client(host, port, secure, path, greetings);
}
