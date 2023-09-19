#include "IndexManager/Indexes.hpp"

template <ValidType T>
auto SequentialIndexContainer::range_search(T begin, T end) const
    -> std::vector<std::streampos> {
  return std::get<SequentialIndex<T>>(m_idx).range_search(begin, end);
}

template <ValidType T>
auto SequentialIndexContainer::remove(T key) const -> std::streampos {
  return std::get<SequentialIndex<T>>(m_idx).remove(key);
}

template <ValidType T>
auto SequentialIndexContainer::search(T key) const -> std::streampos {
  return std::get<SequentialIndex<T>>(m_idx).search(key);
}

template <ValidType T>
auto SequentialIndexContainer::add(T key, std::streampos pos) const -> bool {
  return std::get<SequentialIndex<T>>(m_idx).add(key, pos);
}

auto SequentialIndexContainer::get_attribute_name() const -> std::string {
  return std::visit(
      [](const auto &index) { return index.get_attribute_name(); }, m_idx);
}

auto SequentialIndexContainer::get_table_name() const -> std::string {
  return std::visit([](const auto &index) { return index.get_table_name(); },
                    m_idx);
}

// Explicit instantiation of templates

template auto SequentialIndexContainer::search(int) const -> std::streampos;
template auto SequentialIndexContainer::search(float) const -> std::streampos;
template auto SequentialIndexContainer::search(bool) const -> std::streampos;
template auto SequentialIndexContainer::search(std::string) const
    -> std::streampos;

template auto SequentialIndexContainer::add(int, std::streampos) const

    -> bool;
template auto SequentialIndexContainer::add(float, std::streampos) const

    -> bool;
template auto SequentialIndexContainer::add(bool, std::streampos) const -> bool;
template auto SequentialIndexContainer::add<std::string>(std::string,
                                                         std::streampos) const
    -> bool;

template auto SequentialIndexContainer::remove(int) const -> std::streampos;
template auto SequentialIndexContainer::remove(float) const -> std::streampos;
template auto SequentialIndexContainer::remove(bool) const -> std::streampos;
template auto SequentialIndexContainer::remove<std::string>(std::string) const
    -> std::streampos;

template auto SequentialIndexContainer::range_search(int, int) const
    -> std::vector<std::streampos>;
template auto SequentialIndexContainer::range_search(float, float) const
    -> std::vector<std::streampos>;
template auto SequentialIndexContainer::range_search(bool, bool) const
    -> std::vector<std::streampos>;
template auto SequentialIndexContainer::range_search(std::string,
                                                     std::string) const
    -> std::vector<std::streampos>;
