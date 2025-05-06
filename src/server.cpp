#include <arpa/inet.h>
#include <cstring>
#include <format>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <server.hpp>

ChatServer::ChatServer() : ServerBase() { connectSocket(); }

void ChatServer::connectSocket() {

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

    if ((bind(_sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
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

void ChatServer::run() {
  if (listen(_sockfd, BACKLOG)) {
    throwError();
  }

  std::cout << "Server is listening on port " << PORT << "...\n";

  while (true) {
    socklen_t sin_size = sizeof(_client_addr);
    int client_fd = accept(_sockfd.get(), (sockaddr *)&_client_addr, &sin_size);

    if (client_fd == -1) {
      std::cout << std::format("Error: {}\n", strerror(errno));
      continue;
    }
    // Print client info
    char ip[INET6_ADDRSTRLEN];
    inet_ntop(_client_addr.ss_family, get_in_addr((sockaddr *)&_client_addr),
              ip, sizeof(ip));
    std::cout << std::format("Accepted connection from {}\n", ip);

    _threadPool.enqueue([client_fd]() {
      FileDescriptor fd(client_fd);
      while (true) {
        char buf[DEFAULT_BUFLEN];
        memset(buf, 0, sizeof(buf));
        int nbytes = recv(fd.get(), buf, sizeof(buf), 0);
        if (nbytes == -1) {
          std::cerr << std::format("Error: {}\n", strerror(errno));
          break;
        } else if (nbytes == 0) {
          std::cout << "Client disconneted\n";
          break;
        }
        std::cout << buf << std::endl;
      }
    });
  }
}
