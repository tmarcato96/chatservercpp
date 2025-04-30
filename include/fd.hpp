#pragma once

class FileDescriptor {

public:
  FileDescriptor();
  explicit FileDescriptor(int fd);

  // Non copyable
  FileDescriptor(const FileDescriptor &) = delete;
  FileDescriptor &operator=(const FileDescriptor &) = delete;

  // Movable
  FileDescriptor(FileDescriptor &&other) noexcept;
  FileDescriptor &operator=(FileDescriptor &&other) noexcept;

  ~FileDescriptor();

  void reset(int new_fd = -1);

  int get() const;

  operator int() const;

  bool isValid() const;

private:
  int _fd;
};