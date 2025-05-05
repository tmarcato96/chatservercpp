#pragma once

#include <netdb.h>
#include <sys/socket.h>

#include <fd.hpp>

#define PORT "3490"
#define BACKLOG 10

void *get_in_addr(sockaddr *sa);

class ServerBase {
public:
  ServerBase();
  ~ServerBase();

  virtual void run() = 0;

protected:
  void bindSocket();
  void throwError() const;

  addrinfo _hints, *_servinfo;
  FileDescriptor _sockfd;
};