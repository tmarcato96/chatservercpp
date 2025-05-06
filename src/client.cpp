#include <cstring>
#include <format>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <client.hpp>

ClientServer::ClientServer() : ServerBase() { connectSocket(); }

void ClientServer::connectSocket() {

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

void ClientServer::run() {
  std::cout << "Write something:\n";

  while (true) {
    std::string s;
    std::getline(std::cin, s);

    if (send(_sockfd, s.c_str(), s.size(), 0) == -1) {
      std::cerr << std::format("Error: {}\n", strerror(errno));
    }
  }
}