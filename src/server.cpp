#include <arpa/inet.h>
#include <cstring>
#include <format>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <server.hpp>

void ChatServer::run() {
  if (listen(_sockfd, BACKLOG)) {
    throwError();
  }

  std::cout << "Server is listening on port " << PORT << "...\n";

  while (true) {
    socklen_t sin_size = sizeof(_client_addr);
    FileDescriptor client_fd(
        accept(_sockfd.get(), (sockaddr *)&_client_addr, &sin_size));

    if (!client_fd.isValid()) {
      std::cout << std::format("Error: {}\n", strerror(errno));
      continue;
    }
    // Print client info
    char ip[INET6_ADDRSTRLEN];
    inet_ntop(_client_addr.ss_family, get_in_addr((sockaddr *)&_client_addr),
              ip, sizeof(ip));
    std::cout << std::format("Accepted connection from {}\n", ip);

    // Handle client in a new thread
    std::thread([fd = std::move(client_fd)]() mutable {
      const char *msg = "Hello, World!";
      if (send(fd.get(), msg, strlen(msg), 0) == -1) {
        std::cerr << std::format("Error: {}\n", strerror(errno));
      }
    }).detach();
  }
}
