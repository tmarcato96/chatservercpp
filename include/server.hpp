#pragma once

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <base.hpp>
#include <fd.hpp>

class ChatServer : public ServerBase {
public:
  using ServerBase::ServerBase;

  void run() override;

private:
  sockaddr_storage _client_addr;
};