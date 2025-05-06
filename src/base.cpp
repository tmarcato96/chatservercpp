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
}

ServerBase::~ServerBase() { freeaddrinfo(_servinfo); }

void ServerBase::throwError() const {
  throw std::runtime_error(std::format("Error: {}\n", strerror(errno)));
}