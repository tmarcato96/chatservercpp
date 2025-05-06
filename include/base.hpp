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
  virtual void connectSocket() = 0;
  void throwError() const;

  addrinfo _hints, *_servinfo, *_socketinfo;
  FileDescriptor _sockfd;
};