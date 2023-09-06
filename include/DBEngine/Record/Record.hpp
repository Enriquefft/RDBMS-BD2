#ifndef RECORD_HPP
#define RECORD_HPP

#include <cstdint>
#include <initializer_list>
#include <string>

template <typename... Atribute_types> struct Record {

  Record(std::initializer_list<std::string>);

  template <typename... Args> explicit Record(Args...);

  std::string m_buffer;
};

#endif // !RECORD_HPP
