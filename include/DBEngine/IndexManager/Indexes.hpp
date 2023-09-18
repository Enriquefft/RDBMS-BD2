#ifndef INDEX_HPP
#define INDEX_HPP

#include <spdlog/spdlog.h>
#include <string>

template <typename T> class AVLIndex {
public:
  [[nodiscard]] static auto get_attribute_name() -> std::string { return {}; }
  [[nodiscard]] static auto get_table_name() -> std::string { return {}; }

  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    spdlog::info("Searching range [{}, {}] in AVL index", begin, end);
    return {};
  }

  [[nodiscard]] auto remove(T key) const -> std::streampos {
    spdlog::info("Removing key {} from AVL index", key);
    return {};
  }

  [[nodiscard]] auto search(T key) const -> std::streampos {
    spdlog::info("Searching key {} in AVL index", key);

    return {};
  }

  [[nodiscard]] auto add(T key, std::streampos pos) const -> bool {
    spdlog::info("Adding key {} to AVL index with pos {}", key, pos);
    return {};
  }
};

template <typename T> class IsamIndex {
public:
  [[nodiscard]] static auto get_attribute_name() -> std::string { return {}; }
  [[nodiscard]] static auto get_table_name() -> std::string { return {}; }
  auto range_search(T begin, T end) const -> std::vector<std::streampos> {
    spdlog::info("Searching range [{}, {}] in ISAM index", begin, end);
    return {};
  }
  auto remove(T key) const -> std::streampos {
    spdlog::info("Removing key {} from ISAM index", key);
    return {};
  }
  auto search(T key) const -> std::streampos {
    spdlog::info("Searching key {} in ISAM index", key);
    return {};
  }
  auto add(T key, std::streampos pos) const -> bool {
    spdlog::info("Adding key {} to ISAM index with pos {}", key, pos);
    return {};
  }
};

template <typename T> class SequentialIndex {
public:
  [[nodiscard]] static auto get_attribute_name() -> std::string { return {}; }
  [[nodiscard]] static auto get_table_name() -> std::string { return {}; }
  [[nodiscard]] auto range_search(T begin, T end) const
      -> std::vector<std::streampos> {
    spdlog::info("Searching range [{}, {}] in sequential index", begin, end);
    return {};
  }
  [[nodiscard]] auto remove(T key) const -> std::streampos {
    spdlog::info("Removing key {} from sequential index", key);
    return {};
  }
  [[nodiscard]] auto search(T key) const -> std::streampos {
    spdlog::info("Searching key {} in sequential index", key);
    return {};
  }

  [[nodiscard]] auto add(T key, std::streampos pos) const -> bool {
    spdlog::info("Adding key {} to sequential index with pos {}", key, pos);
    return {};
  }
};

class AvlIndexContainer {
  std::variant<AVLIndex<int>, AVLIndex<float>, AVLIndex<std::string>,
               AVLIndex<bool>>
      m_idx;
};
class SequentialIndexContainer {
  std::variant<SequentialIndex<int>, SequentialIndex<float>,
               SequentialIndex<std::string>, SequentialIndex<bool>>
      m_idx;
};

class IsamIndexContainer {
  std::variant<IsamIndex<int>, IsamIndex<float>, IsamIndex<std::string>,
               IsamIndex<bool>>
      m_idx;
};

#endif // !INDEX_HPP
