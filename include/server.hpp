#pragma once

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fd.hpp>

#define PORT "3490"
#define BACKLOG 10

void *get_in_addr(sockaddr *sa);
void sigchild_handler(int);

class ChatServer {
public:
  ChatServer();
  ~ChatServer();

  void run();

private:
  void bindSocket();
  void throwError() const;

  addrinfo _hints, *_servinfo;
  FileDescriptor _sockfd;
  sockaddr_storage _client_addr;
};