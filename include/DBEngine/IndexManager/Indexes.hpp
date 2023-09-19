#ifndef INDEX_HPP
#define INDEX_HPP

#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>
#include <variant>

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

template <typename T> class AVLIndex {
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
};

template <typename T> class SequentialIndex {
public:
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
};

class AvlIndexContainer {
  std::variant<AVLIndex<int>, AVLIndex<float>, AVLIndex<std::string>,
               AVLIndex<bool>>
      m_idx;
};
struct SequentialIndexContainer {

  template <ValidType T>
  explicit SequentialIndexContainer(SequentialIndex<T> &idx)
      : m_idx{std::move(idx)} {}

  template <ValidType T>
  auto range_search(T begin, T end) const -> std::vector<std::streampos>;

  [[nodiscard]] auto get_attribute_name() const -> std::string;
  [[nodiscard]] auto get_table_name() const -> std::string;
  template <ValidType T>
  [[nodiscard]] auto remove(T key) const -> std::streampos;

  template <ValidType T>
  [[nodiscard]] auto search(T key) const -> std::streampos;

  template <ValidType T>
  [[nodiscard]] auto add(T key, std::streampos pos) const -> bool;

  std::variant<SequentialIndex<int>, SequentialIndex<float>,
               SequentialIndex<std::string>, SequentialIndex<bool>>
      m_idx;
};

class IsamIndexContainer {
  std::variant<IsamIndex<int>, IsamIndex<float>, IsamIndex<std::string>,
               IsamIndex<bool>>
      m_idx;
};

// Explicit template instantiation declaration
extern template auto SequentialIndexContainer::search(int) const
    -> std::streampos;
extern template auto SequentialIndexContainer::search(float) const
    -> std::streampos;
extern template auto SequentialIndexContainer::search(bool) const
    -> std::streampos;
extern template auto
    SequentialIndexContainer::search<std::string>(std::string) const
    -> std::streampos;

extern template auto SequentialIndexContainer::add(int, std::streampos) const
    -> bool;
extern template auto SequentialIndexContainer::add(float, std::streampos) const
    -> bool;
extern template auto SequentialIndexContainer::add(bool, std::streampos) const
    -> bool;
extern template auto
    SequentialIndexContainer::add<std::string>(std::string,
                                               std::streampos) const -> bool;

extern template auto SequentialIndexContainer::remove(int) const
    -> std::streampos;
extern template auto SequentialIndexContainer::remove(float) const
    -> std::streampos;
extern template auto SequentialIndexContainer::remove(bool) const
    -> std::streampos;
extern template auto
    SequentialIndexContainer::remove<std::string>(std::string) const
    -> std::streampos;

extern template auto SequentialIndexContainer::range_search(int, int) const
    -> std::vector<std::streampos>;
extern template auto SequentialIndexContainer::range_search(float, float) const
    -> std::vector<std::streampos>;
extern template auto SequentialIndexContainer::range_search(bool, bool) const
    -> std::vector<std::streampos>;
extern template auto SequentialIndexContainer::range_search<std::string>(
    std::string, std::string) const -> std::vector<std::streampos>;

#endif // !INDEX_HPP
