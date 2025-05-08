#pragma once

#include <map>
#include <memory>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <base.hpp>
#include <fd.hpp>

struct ClientSession {
  FileDescriptor fd;
  std::mutex mutex;
  std::condition_variable cv;
  std::queue<std::string> messageQueue;
  bool active = true;
};

class ChatServer : public ServerBase {
public:
  ChatServer();

  void run() override;

protected:
  void connectSocket() override;

private:
  void initializeClientSession(const std::shared_ptr<ClientSession> &session,
                               const std::string &username);

  void sendToClient(const std::shared_ptr<ClientSession> &session,
                    const std::string &s);
  std::string getFromClient(int fd);

  sockaddr_storage _client_addr;
  std::map<std::string, std::shared_ptr<ClientSession>> _users;
  std::mutex _userMapMutex, _connectMutex;
  std::condition_variable _clientReadyCV;
};
