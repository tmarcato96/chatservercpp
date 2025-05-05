#include <format>
#include <iostream>
#include <stdexcept>
#include <string.h>

#include <base.hpp>

void *get_in_addr(sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((sockaddr_in *)sa)->sin_addr);
  }

  return &(((sockaddr_in6 *)sa)->sin6_addr);
}

ServerBase::ServerBase() {
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

ServerBase::~ServerBase() { freeaddrinfo(_servinfo); }

void ServerBase::bindSocket() {
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

void ServerBase::throwError() const {
  throw std::runtime_error(std::format("Error: {}\n", strerror(errno)));
}