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

    _threadPool.enqueue([this, client_fd]() {
      FileDescriptor fd(client_fd);
      auto session = std::make_shared<ClientSession>();
      session->fd = std::move(fd);
      // Get username
      std::string username = getFromClient(session->fd.get());

      if (username.empty())
        return;

      initializeClientSession(session, username);

      // Get second username
      std::string secondUser = getFromClient(session->fd.get());
      if (secondUser.empty())
        return;
      while (true) {
        char buf[DEFAULT_BUFLEN];
        memset(buf, 0, sizeof(buf));
        int nbytes = recv(session->fd.get(), buf, sizeof(buf), 0);
        if (nbytes == -1) {
          std::cerr << std::format("Errror: {}\n", strerror(errno))
                    << std::flush;
          break;
        } else if (nbytes == 0) {
          std::cout << "Client disconnected" << std::endl;
          break;
        }
        std::string s(buf, nbytes);

        {
          std::lock_guard lock(_userMapMutex);
          auto it = _users.find(secondUser);
          if (it != _users.end()) {
            std::string msg = '@' + username + ": " + s;
            sendToClient(it->second, msg);
          }
        }
      }

      {
        std::lock_guard lock(_userMapMutex);
        _users.erase(username);
        _clientReadyCV.notify_all();
      }
    });
  }
}

void ChatServer::initializeClientSession(
    const std::shared_ptr<ClientSession> &session,
    const std::string &username) {
  std::lock_guard<std::mutex> lock(_connectMutex);

  size_t userCount;
  {
    std::lock_guard lock(_userMapMutex);
    _users.emplace(username, session);
    userCount = _users.size();
  }

  std::string connectMsg = username + " has connected.\n";

  // Prepare user list for everyone
  std::string userList = "List of active users: ";
  for (const auto &[name, _] : _users) {
    userList += name + ", ";
  }
  userList += '\n';

  // 1. Broadcast to all other users
  for (auto &[name, clientSession] : _users) {
    if (name != username) {
      sendToClient(clientSession, connectMsg);
      sendToClient(clientSession, userList); // update list for others
    }
  }

  // 2. Send welcome messages only to the new user
  {
    std::lock_guard<std::mutex> sessionLock(session->mutex);
    session->messageQueue.push(userList);
    if (userCount > 1) {
      session->messageQueue.push("Select user:\n");
    }
  }

  // If we now have exactly 2 users, notify the first one
  if (userCount == 2) {
    for (const auto &[name, clientSession] : _users) {
      if (name != username) {
        sendToClient(clientSession, "Select user:\n");
        break; // only the first existing user
      }
    }
  }

  // 3. Start sender thread
  std::thread senderThread([session]() {
    std::unique_lock lock(session->mutex);
    while (session->active) {
      session->cv.wait(lock, [&]() {
        return !session->messageQueue.empty() || !session->active;
      });

      while (!session->messageQueue.empty()) {
        std::string msg = std::move(session->messageQueue.front());
        session->messageQueue.pop();
        lock.unlock();
        ::send(session->fd.get(), msg.c_str(), msg.size(), 0);
        lock.lock();
      }
    }
  });
  senderThread.detach();
}

std::string ChatServer::getFromClient(int fd) {
  char buf[DEFAULT_BUFLEN];
  int nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes == -1) {
    std::cerr << std::format("Error: {}\n", strerror(errno));
    return {};
  } else if (nbytes == 0) {
    std::cout << "Client disconneted\n";
    return {};
  }
  return std::string(buf, nbytes);
}

void ChatServer::sendToClient(const std::shared_ptr<ClientSession> &session,
                              const std::string &s) {
  {
    std::lock_guard lock(session->mutex);
    session->messageQueue.push(s);
  }
  session->cv.notify_one();
}
