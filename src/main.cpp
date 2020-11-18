// Includes
#include "ssl.hpp"
#include "etc.hpp"
#include "main.hpp"

// Ctrl+C handler function
void sigint_handler(int sig){ // can be called asynchronously
  sigint_flag = 1; // set flag
}

// Main function
int main(int argc, char* argv[]) 
{
  // Ctrl+C handler
  signal(2, sigint_handler); 

  // When run without arguments
  if (argc == 1)
  {
    showHelp();
    std::exit(EXIT_SUCCESS);
  }

  // Variables init
  bool show_help {false};
  bool verbose   {false};
  std::string token; // Discord bot token 

  // Parses arguments, fills variables with values
  parseArgs(argc, argv, show_help, verbose, token);
  
  // When --help flag is on, show help
  if (show_help == true)
    { showHelp(); }

  // When both -h and -v flags are on, print newline
  if (show_help == true and
      verbose == true)
    { std::cout << NEWLINE; }

  // Connect socket
  SSL* ssl = nullptr;
  auto con_err = initConnection(&ssl);
  if (con_err != 0 or !ssl)
  {
    std::cerr << "Cannot connect." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // GET requests
  int64_t bot_id;
  auto get_bot_user_id = "GET /api/users/@me HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot " + token + "\r\n\r\n";
  auto get_guilds = "GET /api/users/@me/guilds HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot " + token + "\r\n\r\n";
  
  sendPacket(ssl, &get_bot_user_id[0]);
  auto response_bot_user_id = recieveResponse(ssl);

  // Send GET request to get bot user id
  std::regex r_bot_id("\\{\"id\": \"(\\d+)");
  std::smatch bot_id_match;
  std::regex_search(response_bot_user_id, bot_id_match, r_bot_id);
  if (bot_id_match.empty() == true) 
  {
    shutdownSSL(ssl);
    exit(EXIT_FAILURE);
  } 
  bot_id = std::stoll(bot_id_match.str(1));
  
  // Send GET request to get guilds ids
  sendPacket(ssl, &get_guilds[0]);
  auto response_guilds = recieveResponse(ssl);

  // Load guild info
  std::vector<Guild> guilds;
  std::regex r_guild_id("\"id\": \"(\\d+)");
  for (auto i = std::sregex_iterator(response_guilds.begin(), response_guilds.end(), r_guild_id);
      i != std::sregex_iterator(); ++i)
  {
    auto g = Guild();
    g.id_str = (*i).str(1);
    g.id = std::stoll(g.id_str);
    guilds.push_back(g);
  }

  if (guilds.empty())
  {
    shutdownSSL(ssl);
    std::cerr << "No guilds found." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Load channel info
  std::string get_channels, response_channels;
  for (auto& g: guilds)
  {
    // Send GET request to get channel ids
    get_channels = "GET /api/guilds/" + g.id_str + "/channels HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot " + token + "\r\n\r\n";
    sendPacket(ssl, &get_channels[0]);
    response_channels = recieveResponse(ssl);

    std::regex r_channel_id("\"id\": \"(\\d+)\", \"last_message_id\": \"\\d+\", \"type\": \\d+, \"name\": \"isa-bot\"");
    for (auto r = std::sregex_iterator(response_channels.begin(), response_channels.end(), r_channel_id);
        r != std::sregex_iterator(); ++r)
    {
      // Load channel
      auto ch = Channel();
      ch.guild_id = g.id;
      ch.guild_id_str = g.id_str;
      ch.id_str = (*r).str(1);
      ch.id = std::stoll(ch.id_str);

      g.channels.push_back(ch);
    }
  }

  for (auto& g: guilds)
  {
    if (g.channels.empty())
    {
      shutdownSSL(ssl);
      std::cerr << "No channels found." << std::endl;
      std::exit(EXIT_SUCCESS);
    }
  }

  // Start loading messagess and answering them
  std::string get_channel_messenges, response_channel_messenges, post_request, response_post, response;
  std::stringstream time_stream;
  time_t t;
  struct tm tm;
  int64_t wait {3}, current_time, id;
  std::regex r_message ("(?:\\[\\{\"id\": |, \\{\"id\": )\"(\\d+)\", \"type\": \\d+?, \"content\": \"(.+?)\", \"channel_id\": \"(\\d+)\", \"author\": \\{\"id\": \"(\\d+)\", \"username\": \"(.+?)\".+?\\}, .+?, \"timestamp\": \"(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}):(\\d{2}):(\\d{2})\\.(\\d{6})\\+(\\d{2}):(\\d{2})\".+?(?:\\}\\]|\\})");
  std::unordered_set<int64_t> ignore {}; // Set of ignored IDs
  
  while (sigint_flag == 0)
  {
    // Load messages
    for (auto& g : guilds)
    {
      for (auto& ch : g.channels)
      {
        // Reset messages
        ch.messages.clear();
        
        // Get timestamp
        t = std::time(nullptr);
        tm = *std::gmtime(&t);
        time_stream.str("");
        time_stream.clear();
        time_stream << std::put_time(&tm,"%Y%m%d%H%M%S");
        time_stream >> current_time;

        // Send GET request
        get_channel_messenges = "GET /api/channels/" + ch.id_str + "/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot " + token + "\r\n\r\n";
        sendPacket(ssl, &get_channel_messenges[0]);
        response_channel_messenges = recieveResponse(ssl);
        
        // Regex - Get all info
        for (auto match = std::sregex_iterator(response_channel_messenges.begin(), response_channel_messenges.end(), r_message);
          match != std::sregex_iterator(); ++match)
        {
          id = std::stoll((*match).str(1));
          
          // If message doesnt exist
          if (ch.messages.find(id) == ch.messages.end())
          {
            // Erase oldest unnecesarry message
            if (ch.messages.size() == 50)
              { ch.messages.erase(ch.messages.begin()->first); }

            // Load new message
            auto mes = Message();
            mes.timestamp_local = current_time;

            mes.id = id;
            mes.content = (*match).str(2);
            mes.channel_id = std::stoll((*match).str(3));
            mes.user_id = std::stoll((*match).str(4));
            mes.username = (*match).str(5);

            std::stringstream time_stamp ((*match).str(6) + (*match).str(7) + (*match).str(8) + (*match).str(9) + (*match).str(10) + (*match).str(11));
            time_stamp >> mes.timestamp;

            ch.messages[id] = mes;
          }
        }
      }
    }

    // Respond to new messages
    for (const auto& g : guilds)
    {
      for (const auto& ch : g.channels)
      { 
        for (const auto& [key, m] : ch.messages)
        {
          (void)key;
          // Ignore mesages from bots
          if (m.username.find("bot") != std::string::npos)
            { ignore.insert(m.id); }
          
          // If the message is new
          if (m.timestamp_local - wait < m.timestamp)
          {
            // If is not ignored message
            if (ignore.find(m.id) == ignore.end())
            {
              // Dont answer to my own messages
              if(bot_id != m.user_id)
              {
                if (verbose == true)
                  { std::cout << m.channel_id << " - " << m.username << ": " << m.content << std::endl; }
                
                // Send POST request
                response = "echo: " + m.username + " - " + m.content;
                post_request = "POST /api/channels/" + std::to_string(m.channel_id) + "/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot " + token + "\r\nContent-Type: application/json\r\nContent-Length:" + std::to_string(15 + response.length()) + "\r\n\r\n{\"content\": \"" + response + "\"}\r\n\r\n";
                
                // Wait to not pass HTTP request limit before actually sending request
                std::this_thread::sleep_for(std::chrono::milliseconds(1250));
                sendPacket(ssl, &post_request[0]);
                response_post = recieveResponse(ssl);

                if (wait < 30)
                  { wait += 3; }

                // Insert id to ignored ids vector to prevent responding to already answered messages
                ignore.insert(m.id);

              } else
                { ignore.insert(m.id); }
            } else
              { ignore.insert(m.id); }
          }
          else
          {
            // Erase old unnecessary ignored ids
            if (ignore.find(m.id) != ignore.end())
              { ignore.erase(m.id); }
          }
        }
      }
    }

    // Wait to not pass HTTP request limit
    std::this_thread::sleep_for(std::chrono::milliseconds(1250));
  }
  
  // Safely shut down
  shutdownSSL(ssl);
  std::exit(EXIT_SUCCESS);
}
