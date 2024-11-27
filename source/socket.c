/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-11-27
 */

#include "socket.h"

/*
 * Create sockaddr from address and port
 *
 * PARAMS
 * - bool debug | Print debug messages
 *
 * RETURN (struct sockaddr_in addr)
 */
static struct sockaddr_in sockaddr_create(int sockfd, const char* address, int port, bool debug)
{
  struct sockaddr_in addr;

  if(strlen(address) == 0)
  {
    socklen_t addrlen = sizeof(addr);

    if(getsockname(sockfd, (struct sockaddr*) &addr, &addrlen) == -1)
    {
      if(debug) error_print("Failed to get sock name: %s", strerror(errno));
    }
  }
  else addr.sin_addr.s_addr = inet_addr(address);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  return addr;
}

/*
 * socket, with debug messages
 *
 * RETURN (int sockfd)
 * - >=0 | Success
 * -  -1 | Failed to create socket
 */
static int socket_create(bool debug)
{
  if(debug) info_print("Creating socket");

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd == -1)
  {
    if(debug) error_print("Failed to create socket: %s", strerror(errno));

    return -1;
  }

  if(debug) info_print("Created socket (%d)", sockfd);

  return sockfd;
}

/*
 * connect, with debug messages
 *
 * RETURN (int status)
 * -  0 | Success
 * - -1 | Failed to connect to server socket
 */
static int socket_connect(int sockfd, const char* address, int port, bool debug)
{
  struct sockaddr_in addr = sockaddr_create(sockfd, address, port, debug);

  if(debug) info_print("Connecting socket (%s:%d)", address, port);

  if(connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
  {
    if(debug) error_print("Failed to connect socket (%s:%d): %s", address, port, strerror(errno));

    return -1;
  }

  if(debug) info_print("Connected socket (%s:%d)", address, port);

  return 0;
}

/*
 * Create a client socket and connect it to the server socket
 *
 * RETURN (int sockfd)
 * - >=0 | Success
 * -  -1 | Failed to create server socket
 */
int client_socket_create(const char* address, int port, bool debug)
{
  int sockfd = socket_create(debug);

  if(sockfd == -1) return -1;

  if(socket_connect(sockfd, address, port, debug) == -1)
  {
    socket_close(&sockfd, debug);

    return -1;
  }

  return sockfd;
}

/*
 * close, but with pointer to file descriptor, and with debug messages
 *
 * Note: If no open socket is supplied, nothing is done
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Failed to close socket
 */
int socket_close(int* sockfd, bool debug)
{
  if(!sockfd || *sockfd == -1) return 0;

  if(debug) info_print("Closing socket (%d)", *sockfd);

  if(close(*sockfd) == -1)
  {
    if(debug) error_print("Failed to close socket: %s", strerror(errno));

    return -1;
  }

  if(debug) info_print("Closed socket");

  *sockfd = -1;

  return 0;
}

/*
 * Recieve a single line to a buffer from a socket connection
 *
 * RETURN (ssize_t size)
 * - >0 | The number of recieved characters
 * -  0 | Nothing to recieve, end of file
 * - -1 | Failed to recieve from socket
 */
ssize_t socket_recv(int sockfd, char* buffer, size_t size)
{
  if(errno != 0) return -1;

  if(!buffer) return 0;

  char symbol = '\0';
  ssize_t index;

  for(index = 0; index < size && symbol != '\n'; index++)
  {
    ssize_t status = recv(sockfd, &symbol, 1, 0);

    if(status == -1 || errno != 0) return -1; // ERROR

    buffer[index] = symbol;

    if(status == 0) return 0; // End Of File
  }

  return index;
}

/*
 * Send a single line from a buffer to a socket connection
 *
 * RETURN (ssize_t size)
 * - >0 | The number of sent characters
 * -  0 | Nothing to send to, end of file
 * - -1 | Failed to send to socket
 */
ssize_t socket_send(int sockfd, const char* buffer, size_t size)
{
  if(errno != 0) return -1;

  if(!buffer) return 0;

  ssize_t index;
  char symbol = '\0';

  for(index = 0; index < size && symbol != '\n'; index++)
  {
    symbol = buffer[index];

    ssize_t status = send(sockfd, &symbol, 1, 0);

    if(status == -1 || errno != 0) return -1; // ERROR
    
    if(status == 0) return 0; // End Of File

    if(symbol == '\0') break;
  }

  return index;
}
