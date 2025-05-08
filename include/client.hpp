#pragma once

#include <base.hpp>

class ChatClient : public ServerBase {
public:
  ChatClient();

  void run() override;

protected:
  void connectSocket() override;

private:
  void sendToServer(const std::string &s) const;
  std::string getFromServer();
  std::atomic<bool> _running{true};
};