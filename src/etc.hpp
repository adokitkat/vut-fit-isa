#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__) // Windows
  #define NEWLINE "\r\n"
#else
  #define NEWLINE '\n' // Avoid std::endl because of proper buffering and flushing
#endif

// Basic C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
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

// Function for help text
void showHelp();

// Argument parsing
void parseArgs(int &argc, char* argv[], bool &show_help, bool &verbose, std::string &token);
