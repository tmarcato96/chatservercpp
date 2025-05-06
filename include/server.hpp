#pragma once

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <base.hpp>
#include <fd.hpp>

class ChatServer : public ServerBase {
public:
  ChatServer();

  void run() override;

protected:
  void connectSocket() override;

private:
  sockaddr_storage _client_addr;
};