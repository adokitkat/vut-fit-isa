#include "etc.hpp"

// Function for help text
void showHelp()
{
  std::cout
    << "Usage:" << NEWLINE
    << "  isabot [-h|--help] [-v|--verbose] -t|--token <bot_access_token>" << NEWLINE
    << NEWLINE
    << "Flags:" << NEWLINE
    << "  -h, --help\t\t\t\t"               << "Show help" << NEWLINE
    << "  -v, --verbose\t\t\t\t"            << "Verbose output switch" << NEWLINE
    << "  -t, --token <bot_access_token>\t" << "Discord bot auth token" << std::endl
  ;
}

// Argument parsing
void parseArgs(int& argc, char* argv[], bool& show_help, bool& verbose, std::string& token)
{  
  const struct option long_options[] =
  {
    {"help",    no_argument, nullptr, 'h'},
    {"verbose", no_argument, nullptr, 'v'},
    {"token",   no_argument, nullptr, 't'},
    {nullptr,   no_argument, nullptr, 0}
  };

  int arg {0};
  // Load arguments
  while (arg != -1)
  {
    arg = getopt_long(argc, argv, "t::vh", long_options, nullptr);
    switch (arg)
    {
      case 't':
        if (!optarg and
            argv[optind] != nullptr and
            argv[optind][0] != '-')
        { 
          token = std::string(argv[optind++]); 
        }
        else // token.empty()
        {
          std::cerr << "Token cannot be empty." << std::endl;
          std::exit(EXIT_FAILURE);
        }
        break;

      case 'v':
        verbose = true;
        break;

      case 'h':
      case '?':
        show_help = true;
        break;

      default:
        break;
    }
  }
}