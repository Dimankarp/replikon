#ifndef REPLIKON_SERIAL_SERDE_H
#define REPLIKON_SERIAL_SERDE_H

#include "utils.h"
#include <cstddef>
#include <cstring>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
namespace replikon::serde {

using Buffer = std::vector<std::byte>;

namespace detail {

class BufferView {
public:
  BufferView(const std::byte *p, size_t sz) : _data{p}, _size{sz} {}
  BufferView(const Buffer &b) : _data{b.data()}, _size{b.size()} {}

public:
  const std::byte *data() const { return _data; }
  size_t size() const { return _size; }
  void consume(size_t n) {
    REPLIKON_ASSERT(n <= _size);
    _size -= n;
    _data += n;
  }

private:
  const std::byte *_data;
  size_t _size;
};

} // namespace detail

template <typename T, typename Enable = void> class Serializer;

// Helper-functions
template <typename T> inline void serialize(Buffer &buf, const T &value) {
  Serializer<std::decay_t<T>>::serialize(buf, value);
}

template <typename T> inline std::optional<T> deserialize(const Buffer &buf) {
  auto view = detail::BufferView{buf};
  return Serializer<std::decay_t<T>>::deserialize(view);
}
template <typename T>
inline std::optional<T> deserialize(detail::BufferView &buf) {
  return Serializer<std::decay_t<T>>::deserialize(buf);
}

// Arithmetic
template <typename T>
struct Serializer<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
  static void serialize(Buffer &buf, const T &value) {
    const std::byte *ptr = reinterpret_cast<const std::byte *>(&value);
    buf.insert(buf.end(), ptr, ptr + sizeof(T));
  }

  static std::optional<T> deserialize(detail::BufferView &view) {
    if (sizeof(T) > view.size()) {
      return std::nullopt;
    }
    T value;
    std::memcpy(&value, view.data(), sizeof(T));
    view.consume(sizeof(T));
    return std::make_optional(value);
  }
};

// String
template <> struct Serializer<std::string> {
  static void serialize(Buffer &buf, const std::string &value) {
    replikon::serde::serialize(buf, static_cast<size_t>(value.size()));
    auto ptr = reinterpret_cast<const std::byte *>(value.data());
    buf.insert(buf.end(), ptr, ptr + value.size());
  }

  static std::optional<std::string> deserialize(detail::BufferView &view) {
    auto size_opt = replikon::serde::deserialize<size_t>(view);
    if (!size_opt.has_value()) {
      return std::nullopt;
    }
    auto size = *size_opt;
    if (size > view.size()) {
      return std::nullopt;
    }

    std::string value;
    value.assign(reinterpret_cast<const char *>(view.data()), size);

    view.consume(size);
    return std::make_optional(std::move(value));
  }
};

namespace detail {

template <typename T, std::size_t N> struct Tie;

template <typename T> struct Tie<T, 0> {
  static auto tie(T &) { return std::tuple<>{}; }
};

template <typename T> struct Tie<T, 1> {
  static auto tie(T &v) {
    auto &[a] = v;
    return std::tie(a);
  }
};

template <typename T> struct Tie<T, 2> {
  static auto tie(T &v) {
    auto &[a, b] = v;
    return std::tie(a, b);
  }
};

template <typename T> struct Tie<T, 3> {
  static auto tie(T &v) {
    auto &[a, b, c] = v;
    return std::tie(a, b, c);
  }
};

template <typename T> struct Tie<T, 4> {
  static auto tie(T &v) {
    auto &[a, b, c, d] = v;
    return std::tie(a, b, c, d);
  }
};

template <typename T> struct Tie<T, 5> {
  static auto tie(T &v) {
    auto &[a, b, c, d, e] = v;
    return std::tie(a, b, c, d, e);
  }
};
} // namespace detail

template <typename... Args> //
struct Serializer<std::tuple<Args...>> {
  template <std::size_t... Is>
  static void serializeImpl(Buffer &buf, const std::tuple<Args...> &t,
                            std::index_sequence<Is...>) {
    (..., replikon::serde::serialize(buf, std::get<Is>(t)));
  }

  template <std::size_t... Is>
  static std::optional<std::tuple<std::decay_t<Args>...>>
  deserializeImpl(detail::BufferView &view, std::index_sequence<Is...>) {
    std::tuple<std::optional<std::decay_t<Args>>...> results;
    bool success =
        (... && ((std::get<Is>(results) =
                      Serializer<std::decay_t<Args>>::deserialize(view)),
                 std::get<Is>(results).has_value()));

    if (success) {
      return std::tuple<std::decay_t<Args>...>(
          std::move(*std::get<Is>(results))...);
    }
    return std::nullopt;
  }

  static void serialize(Buffer &buf, const std::tuple<Args...> &t) {
    serializeImpl(buf, t, std::index_sequence_for<Args...>{});
  }

  static std::optional<std::tuple<std::decay_t<Args>...>>
  deserialize(detail::BufferView &view) {
    return deserializeImpl(view, std::index_sequence_for<Args...>{});
  }
};

} // namespace replikon::serde

#define SERIALIZABLE(struct_name, fields_count)                                \
  namespace replikon::serde {                                                  \
  template <> struct Serializer<struct_name> {                                 \
    static void serialize(Buffer &buf, const struct_name &v) {                 \
      struct_name temp = v;                                                    \
      replikon::serde::serialize(                                              \
          buf, detail::Tie<struct_name, fields_count>::tie(temp));             \
    }                                                                          \
                                                                               \
    static std::optional<struct_name> deserialize(detail::BufferView &view) {  \
      using fields_tuple =                                                     \
          decltype(detail::Tie<struct_name, fields_count>::tie(                \
              std::declval<struct_name &>()));                                 \
      auto res = replikon::serde::Serializer<fields_tuple>::deserialize(view); \
      if (!res)                                                                \
        return std::nullopt;                                                   \
                                                                               \
      return std::apply(                                                       \
          [](auto &&...args) {                                                 \
            return struct_name{std::forward<decltype(args)>(args)...};         \
          },                                                                   \
          std::move(*res));                                                    \
    }                                                                          \
  };                                                                           \
  }

#endif // REPLIKON_SERIAL_SERDE_H