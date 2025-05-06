#pragma once

#include <base.hpp>

class ClientServer : public ServerBase {
public:
  ClientServer();

  void run() override;

protected:
  void connectSocket() override;
};