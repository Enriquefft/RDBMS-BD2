#ifndef INDEX_HPP
#define INDEX_HPP

#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>
#include <variant>

#include "Sequential/sequential_index.hpp"

template <typename T>
// T must be int, float, string or bool
concept ValidType = std::same_as<T, int> || std::same_as<T, float> ||
                    std::same_as<T, std::string> || std::same_as<T, bool>;

template <typename T> auto str_cast(const T &value) -> std::string {
  return std::to_string(value);
}
template <> inline auto str_cast(const std::string &value) -> std::string {
  return value;
}

template <typename T> class AvlIndex {
public:
  [[nodiscard]] auto get_attribute_name() const -> std::string { return {}; }
  [[nodiscard]] auto get_table_name() const -> std::string { return {}; }

  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    spdlog::info("Searching range [{}, {}] in AVL index", str_cast(begin),
                 str_cast(end));
    return {};
  }

  [[nodiscard]] auto remove(T key) const -> std::streampos {
    spdlog::info("Removing key {} from AVL index", str_cast(key));
    return {};
  }

  [[nodiscard]] auto search(T key) const -> std::streampos {
    spdlog::info("Searching key {} in AVL index", str_cast(key));

    return {};
  }

  [[nodiscard]] auto add(T key, std::streampos pos) const -> bool {
    spdlog::info("Adding key {} to AVL index with pos {}", str_cast(key),
                 str_cast(pos));
    return {};
  }
  [[nodiscard]] auto
  bulk_insert(const std::vector<std::pair<T, std::streampos>> &elements) const
      -> std::vector<bool> {
    spdlog::info("Inserting {} elements to AVL index", elements.size());
    return {};
  }
};

template <typename T> class IsamIndex {
public:
  [[nodiscard]] auto get_attribute_name() const -> std::string { return {}; }
  [[nodiscard]] auto get_table_name() const -> std::string { return {}; }
  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    spdlog::info("Searching range [{}, {}] in ISAM index", str_cast(begin),
                 str_cast(end));
    return {};
  }
  [[nodiscard]] auto remove(T key) const -> std::streampos {
    spdlog::info("Removing key {} from ISAM index", str_cast(key));
    return {};
  }
  [[nodiscard]] auto search(T key) const -> std::streampos {
    spdlog::info("Searching key {} in ISAM index", str_cast(key));
    return {};
  }
  [[nodiscard]] auto add(T key, std::streampos pos) const -> bool {
    spdlog::info("Adding key {} to ISAM index with pos {}", str_cast(key),
                 str_cast(pos));

    return {};
  }
  [[nodiscard]] auto
  bulk_insert(const std::vector<std::pair<T, std::streampos>> &elements) const
      -> std::vector<bool> {
    spdlog::info("Inserting {} elements to ISAM index", elements.size());
    return {};
  }
};

template <typename T> class SequentialIndex {
public:
  using MIN_BULK_INSERT_SIZE = std::integral_constant<size_t, 1000>;

  [[nodiscard]] auto get_attribute_name() const -> std::string {
    spdlog::info("Getting attribute name from sequential index");
    return {};
  }
  [[nodiscard]] auto get_table_name() const -> std::string {
    spdlog::info("Getting table name from sequential index");
    return {};
  }
  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    spdlog::info("Searching range [{}, {}] in sequential index",
                 str_cast(begin), str_cast(end));
    return {};
  }
  [[nodiscard]] auto remove(T key) const -> std::streampos {
    spdlog::info("Removing key {} from sequential index", str_cast(key));
    return {};
  }
  [[nodiscard]] auto search(T key) const -> std::streampos {
    spdlog::info("Searching key {} in sequential index", str_cast(key));
    return {};
  }

  [[nodiscard]] auto add(T key, std::streampos pos) const -> bool {
    spdlog::info("Adding key {} to sequential index with pos {}", str_cast(key),
                 str_cast(pos));
    return {};
  }

  [[nodiscard]] auto
  bulk_insert(const std::vector<std::pair<T, std::streampos>> &elements) const
      -> std::vector<bool> {
    spdlog::info("Inserting {} elements to sequential index", elements.size());
    return {};
  }
};

struct AvlIndexContainer {
  template <ValidType T>
  explicit AvlIndexContainer(SequentialIndex<T> &idx) : m_idx{std::move(idx)} {}

  template <ValidType T>
  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    return std::get<AvlIndex<T>>(m_idx).range_search(begin, end);
  }

  [[nodiscard]] auto get_attribute_name() const -> std::string {
    return std::visit(
        [](const auto &index) { return index.get_attribute_name(); }, m_idx);
  }
  [[nodiscard]] auto get_table_name() const -> std::string {
    return std::visit([](const auto &index) { return index.get_table_name(); },
                      m_idx);
  }
  template <ValidType T>
  [[nodiscard]] auto remove(T key) const -> std::streampos {
    return std::get<AvlIndex<T>>(m_idx).remove(key);
  }

  template <ValidType T>
  [[nodiscard]] auto search(T key) const -> std::streampos {
    return std::get<AvlIndex<T>>(m_idx).search(key);
  }

  template <ValidType T>
  [[nodiscard]] auto add(T key, std::streampos pos) -> bool {
    return std::get<AvlIndex<T>>(m_idx).add(key, pos);
  }

  template <ValidType T>
  auto bulk_insert(const std::vector<std::pair<T, std::streampos>> &elements)
      -> std::vector<bool> {
    return std::get<AvlIndex<T>>(m_idx).bulk_insert(elements);
  }
  std::variant<AvlIndex<int>, AvlIndex<float>, AvlIndex<std::string>> m_idx;
};

struct SequentialIndexContainer {

  template <ValidType T>
  explicit SequentialIndexContainer(SequentialIndex<T> &idx)
      : m_idx{std::move(idx)} {}

  template <ValidType T>
  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    return std::get<SequentialIndex<T>>(m_idx).range_search(begin, end);
  }

  [[nodiscard]] auto get_attribute_name() const -> std::string {
    return std::visit(
        [](const auto &index) { return index.get_attribute_name(); }, m_idx);
  }
  [[nodiscard]] auto get_table_name() const -> std::string {
    return std::visit([](const auto &index) { return index.get_table_name(); },
                      m_idx);
  }
  template <ValidType T>
  [[nodiscard]] auto remove(T key) const -> std::streampos {
    return std::get<SequentialIndex<T>>(m_idx).remove(key);
  }

  template <ValidType T>
  [[nodiscard]] auto search(T key) const -> std::streampos {
    return std::get<SequentialIndex<T>>(m_idx).search(key);
  }

  template <ValidType T>
  [[nodiscard]] auto add(T key, std::streampos pos) -> bool {
    return std::get<SequentialIndex<T>>(m_idx).add(key, pos);
  }

  template <ValidType T>
  auto bulk_insert(const std::vector<std::pair<T, std::streampos>> &elements)
      -> std::vector<bool> {
    return std::get<SequentialIndex<T>>(m_idx).bulk_insert(elements);
  }

  std::variant<SequentialIndex<int>, SequentialIndex<float>,
               SequentialIndex<std::string>>
      m_idx;
};

struct IsamIndexContainer {
  template <ValidType T>
  explicit IsamIndexContainer(IsamIndex<T> &idx) : m_idx{std::move(idx)} {}

  template <ValidType T>
  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    return std::get<IsamIndex<T>>(m_idx).range_search(begin, end);
  }

  [[nodiscard]] auto get_attribute_name() const -> std::string {
    return std::visit(
        [](const auto &index) { return index.get_attribute_name(); }, m_idx);
  }
  [[nodiscard]] auto get_table_name() const -> std::string {
    return std::visit([](const auto &index) { return index.get_table_name(); },
                      m_idx);
  }
  template <ValidType T>
  [[nodiscard]] auto remove(T key) const -> std::streampos {
    return std::get<IsamIndex<T>>(m_idx).remove(key);
  }

  template <ValidType T>
  [[nodiscard]] auto search(T key) const -> std::streampos {
    return std::get<IsamIndex<T>>(m_idx).search(key);
  }

  template <ValidType T>
  [[nodiscard]] auto add(T key, std::streampos pos) -> bool {
    return std::get<IsamIndex<T>>(m_idx).add(key, pos);
  }

  template <ValidType T>
  auto bulk_insert(const std::vector<std::pair<T, std::streampos>> &elements)
      -> std::vector<bool> {
    return std::get<IsamIndex<T>>(m_idx).bulk_insert(elements);
  }
  std::variant<IsamIndex<int>, IsamIndex<float>, IsamIndex<std::string>> m_idx;
};

#endif // !INDEX_HPP
