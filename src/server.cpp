#include <arpa/inet.h>
#include <format>
#include <iostream>
#include <signal.h>
#include <stdexcept>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <server.hpp>

void *get_in_addr(sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((sockaddr_in *)sa)->sin_addr);
  }

  return &(((sockaddr_in6 *)sa)->sin6_addr);
}

void sigchild_handler(int s) {
  int saved_errno = errno;
  while (waitpid(-1, nullptr, WNOHANG) > 0)
    ;
  errno = saved_errno;
}

ChatServer::ChatServer() {
  // Make sure _hints is filled with zeros
  memset(&_hints, 0, sizeof(_hints));
  _hints.ai_family = AF_UNSPEC;
  _hints.ai_socktype = SOCK_STREAM;
  _hints.ai_flags = AI_PASSIVE;

  if (int rv = getaddrinfo(nullptr, PORT, &_hints, &_servinfo); rv != 0) {
    throw std::runtime_error(
        std::format("getaddrinfo: {}\n", gai_strerror(rv)));
  }

  bindSocket();
}

ChatServer::~ChatServer() { freeaddrinfo(_servinfo); }

void ChatServer::bindSocket() {
  addrinfo *p;
  for (p = _servinfo; p != nullptr; p = p->ai_next) {
    if ((_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      std::cout << std::format("Error: {}\n", strerror(errno));
      continue;
    }

    int yes = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      throwError();
    }

    if ((bind(_sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
      close(_sockfd);
      std::cout << std::format("Error: {}\n", strerror(errno));
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

  struct sigaction sa;
  sa.sa_handler = sigchild_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
    throwError();
  }

  while (true) {
    socklen_t sin_size = sizeof(_client_addr);
    _new_fd = accept(_sockfd, (sockaddr *)&_client_addr, &sin_size);
    if (_new_fd == -1) {
      std::cout << std::format("Error: {}\n", strerror(errno));
      continue;
    }
    // Print client info
    char ip[INET6_ADDRSTRLEN];
    inet_ntop(_client_addr.ss_family, get_in_addr((sockaddr *)&_client_addr),
              ip, sizeof(ip));
    std::cout << std::format("Accepted connection from {}\n", ip);

    if (!fork()) {
      close(_sockfd);
      if (send(_new_fd, "Hello, world!", 13, 0) == -1) {
        std::cout << std::format("Error: {}\n", strerror(errno));
      }
      close(_new_fd);
      _exit(0);
    }
    close(_new_fd);
  }
}

void ChatServer::throwError() const {
  throw std::runtime_error(std::format("Error: {}\n", strerror(errno)));
}