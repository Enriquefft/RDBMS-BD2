#ifdef __clang__
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

#include "Record/Record.hpp"
#include "Utils/gsl.hpp"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <numeric>

using DB_ENGINE::Record;

auto Record::write(std::fstream &file, const std::vector<Type> &types)
    -> std::ostream & {
  int64_t buffer_size = std::accumulate(
      types.begin(), types.end(), static_cast<uint8_t>(0),
      [](uint8_t sum, const Type &type) { return sum + type.size; });

  std::unique_ptr<char[]> tmp_buffer(
      new char[static_cast<uint64_t>(buffer_size)]);

  auto buffer_current_pos = 0;
  for (size_type i = 0; i < types.size(); ++i) {

    auto field_size = types.at(i).size;

    if (buffer_current_pos + field_size > buffer_size) {
      // Handle the error, e.g., throw an exception, return an error code, etc.
      throw std::runtime_error("Buffer overflow detected");
    }
    std::memcpy(tmp_buffer.get() + buffer_current_pos, m_fields.at(i).data(),
                field_size);
    buffer_current_pos += field_size;
  }

  auto &response = file.write(tmp_buffer.get(), buffer_size);
  return response;
}

auto Record::read(std::fstream &file, const std::vector<Type> &types)
    -> std::istream & {
  int64_t buffer_size = std::accumulate(
      types.begin(), types.end(), static_cast<uint8_t>(0),
      [](uint8_t sum, const Type &type) { return sum + type.size; });

  std::unique_ptr<char[]> tmp_buffer(
      new char[static_cast<uint64_t>(buffer_size)]);

  auto &response = file.read(tmp_buffer.get(), buffer_size);

  if (!response) {
    return response;
  }

  auto curr_buffer_pos = 0;

  m_fields.resize(types.size());

  for (size_type i = 0; i < types.size(); ++i) {
    auto field_size = types.at(i).size;
    m_fields.at(i).resize(field_size);
    std::memcpy(m_fields.at(i).data(), tmp_buffer.get() + curr_buffer_pos,
                field_size);
    curr_buffer_pos += field_size;
  }
  return response;
}
