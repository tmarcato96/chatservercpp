#include <unistd.h>

#include <fd.hpp>

FileDescriptor::FileDescriptor() : _fd(-1) {}

FileDescriptor::FileDescriptor(int fd) : _fd(fd) {}

// Move constructor
FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
    : _fd(other._fd) {
  other._fd = -1;
}
// Move assignent
FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other) noexcept {
  if (this != &other) {
    reset();
    _fd = other._fd;
    other._fd = -1;
  }
  return *this;
}

FileDescriptor::~FileDescriptor() { reset(); }

void FileDescriptor::reset(int new_fd) {
  if (_fd != -1) {
    close(_fd);
  }
  _fd = new_fd;
}

int FileDescriptor::get() const { return _fd; }

FileDescriptor::operator int() const { return _fd; }

bool FileDescriptor::isValid() const { return _fd != -1; }