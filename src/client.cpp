#include <cstring>
#include <format>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <client.hpp>

ChatClient::ChatClient() : ServerBase() { connectSocket(); }

void ChatClient::connectSocket() {

  addrinfo *p;
  for (p = _servinfo; p != nullptr; p = p->ai_next) {
    _sockfd =
        FileDescriptor(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
    if (!_sockfd.isValid()) {
      std::cerr << std::format("Error: {}\n", strerror(errno));
      continue;
    }

    int yes = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      throwError();
    }

    if ((connect(_sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
      _sockfd.reset();
      std::cerr << std::format("Error: {}\n", strerror(errno));
      continue;
    }
    break;
  }

  if (p == nullptr) {
    throw std::runtime_error("Failed to bind.\n");
  }
}

void ChatClient::run() {
  std::cout << "Enter your username to register:\n";
  std::string username;
  std::getline(std::cin, username);
  sendToServer(username);
  username.clear();
  // Start receiver thread
  std::thread receiver([this] {
    while (_running) {
      std::string msg = getFromServer();
      if (!msg.empty()) {
        std::cout << '\n' << msg << "\n " << std::flush;
      }
    }
  });

  // std::cout << "Select user:\n";
  std::getline(std::cin, username);
  sendToServer(username);

  std::cout << "Write something:\n";

  while (true) {
    std::string s;
    std::getline(std::cin, s);
    if (s == ":q")
      break;
    sendToServer(s);
  }

  _running = false;
  receiver.join();
}

void ChatClient::sendToServer(const std::string &s) const {
  if (send(_sockfd, s.c_str(), s.size(), 0) == -1) {
    std::cerr << std::format("Error: {}\n", strerror(errno)) << std::flush;
  }
}

std::string ChatClient::getFromServer() {
  char buf[DEFAULT_BUFLEN];
  int nbytes = recv(_sockfd, buf, sizeof(buf), 0);
  if (nbytes <= 0) {
    _running = false;
    return "[Disconnected from server]";
  }
  return std::string(buf, nbytes);
}