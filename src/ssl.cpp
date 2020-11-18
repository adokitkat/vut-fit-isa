#include "ssl.hpp"

// Initialize SSL connection
void initializeSSL()
{
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

// Safely shut-down SSL connection
void shutdownSSL(SSL* ssl)
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

// Helper function to connect to Discord server
int initConnection(SSL** ssl_return)
{
  auto socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0)
  {
      std::cout << "Error creating socket." << std::endl;
      return -1;
  }

  auto server_ip   = inet_addr("162.159.136.232"); // Discord IPv4 address
  auto server_port = htons(443);

  struct sockaddr_in socket_address;
  bzero(&socket_address, sizeof(socket_address));
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = server_ip;
  socket_address.sin_port = server_port;

  auto connect_err = connect(socket_fd, (struct sockaddr *) &socket_address, sizeof(socket_address));
  if (connect_err != 0)
  {
    std::cout << "Error connecting to server." << std::endl;
    return -1;
  }

  initializeSSL();
  
  auto ssl_method = SSLv23_client_method();
  auto ssl_ctx = SSL_CTX_new(ssl_method);
  auto ssl = SSL_new(ssl_ctx);

  if (!ssl) {
    std::cout << "Error creating SSL." << std::endl;
    return -1;
  }

  SSL_set_fd(ssl, socket_fd);

  auto ssl_err = SSL_connect(ssl); 
  if (ssl_err <= 0)
  {
    printf("Error creating SSL connection.  err=%x\n", ssl_err);
    shutdownSSL(ssl);
    return -1;
  }

  *ssl_return = ssl;
  return 0;
}

// Sends packets via SSL connection
int sendPacket(SSL* ssl, const char *buf)
{
  int ret {0};
  auto len = SSL_write(ssl, buf, strlen(buf));
  if (len < 0) {
    auto ssl_err = SSL_get_error(ssl, len);
    switch (ssl_err)
    {
      case SSL_ERROR_WANT_WRITE:
      case SSL_ERROR_WANT_READ:
        ret = 0;
        break;
      case SSL_ERROR_ZERO_RETURN:
      case SSL_ERROR_SYSCALL:
      case SSL_ERROR_SSL:
      default:
        ret = -1;
        break;
    }
  }
  return ret;
}

// Recieves packets via SSL connection and returns a string
std::string recieveResponse(SSL* ssl)
{
  int read_len {0};
  char buf[256];
  std::stringstream ss;
  
  do {
    read_len = SSL_read(ssl, buf, 255);
    buf[read_len] = '\0';
    ss << buf;
  } while (read_len > 5);

  return ss.str();
}
