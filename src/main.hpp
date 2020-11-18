#pragma once

// Basic C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>
#include <thread>
#include <chrono>
#include <regex>
#include <iomanip>
#include <sstream>
#include <algorithm>

// C-style includes
#include <cstdlib>
#include <ctime>

// Argument loading
#include <getopt.h>

// Networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

// OpenSSL
#include <openssl/ssl.h>
#include <openssl/err.h>

// POSIX signal
#include <signal.h>

// Ctrl+C handler flag
volatile sig_atomic_t sigint_flag = 0;

struct Message
{
  int64_t id;
  int64_t user_id;
  int64_t channel_id;
  int64_t timestamp;
  int64_t timestamp_local;
  std::string username;
  std::string content;
};

struct Channel
{
  int64_t id;
  std::string id_str;
  int64_t guild_id;
  std::string guild_id_str;
  std::string name;
  std::map<int64_t, Message> messages;
};

struct Guild
{
  int64_t id;
  std::string id_str;
  std::string name;
  std::vector<Channel> channels;
};