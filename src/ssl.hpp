#pragma once

// Basic includes
#include <iostream>
#include <string>
#include <thread>
#include <regex>
#include <sstream>

// Networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// OpenSSL
#include <openssl/ssl.h>
#include <openssl/err.h>

// Initialize SSL connection
void initializeSSL();

// Safely shut-down SSL connection
void shutdownSSL(SSL* ssl);

// Helper function to connect to Discord server
int initConnection(SSL** ssl_return);

// Sends packets via SSL connection
int sendPacket(SSL* ssl, const char *buf);

// Recieves packets via SSL connection and returns a string
std::string recieveResponse(SSL* ssl);