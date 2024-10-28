#include "util.hpp"

#include <fmt/format.h>

#include <fstream>

namespace gbcxx::fs {
std::vector<u8> read(const std::filesystem::path& file) {
  std::ifstream ifs(file, std::ios::binary);
  if (!ifs.is_open()) {
    LOG_ERROR("Failed to open file: {}", file.string());
    return {};
  }

  std::vector<u8> buf(std::filesystem::file_size(file));
  buf.insert<std::istreambuf_iterator<char>>(buf.begin(), ifs, {});
  return buf;
}
}  // namespace gbcxx::fs
