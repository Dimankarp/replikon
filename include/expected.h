#ifndef REPLIKON_EXPECTED_H
#define REPLIKON_EXPECTED_H

#include "utils.h"
#include <utility>

#define RETURN_IF_ERROR(expected)                                              \
  do {                                                                         \
    if (!expected.hasValue()) {                                                \
      return Unexpected{expected.error()};                                     \
    }                                                                          \
  } while (false)

namespace replikon {

template <typename E> struct Unexpected {
  E error;
  explicit Unexpected(E e) : error(std::move(e)) {}
};

template <typename Value, typename Error> class Expected {
public:
  ~Expected() { destroy(); }

  Expected(const Value &value) : _value{value}, _is_ok{true} {}
  Expected(Value &&value) : _value{std::move(value)}, _is_ok{true} {}

  Expected(const Unexpected<Error> &unexp)
      : _error{unexp.error}, _is_ok{false} {}
  Expected(Unexpected<Error> &&unexp)
      : _error{std::move(unexp.error)}, _is_ok{false} {}

  Expected(const Expected &other) : _is_ok(other._is_ok) {
    if (_is_ok) {
      new (&_value) Value(other._value);
    } else {
      new (&_error) Error(other._error);
    }
  }

  Expected &operator=(const Expected &other) {
    if (this != &other) {
      destroy();
      _is_ok = other._is_ok;
      if (_is_ok) {
        new (&_value) Value(other._value);
      } else {
        new (&_error) Error(other._error);
      }
    }
    return *this;
  }

  Expected(Expected &&other) : _is_ok(other._is_ok) {
    if (_is_ok) {
      new (&_value) Value(std::move(other._value));
    } else {
      new (&_error) Error(std::move(other._error));
    }
  }

  Expected &operator=(Expected &&other) {
    if (this != &other) {
      destroy();
      _is_ok = other._is_ok;
      if (_is_ok) {
        new (&_value) Value(std::move(other._value));
      } else {
        new (&_error) Error(std::move(other._error));
      }
    }
    return *this;
  }

public:
  bool hasValue() const { return _is_ok; }

  Value &&value() && {
    REPLIKON_ASSERT(_is_ok);
    return std::move(_value);
  }

  Error &&error() && {
    REPLIKON_ASSERT(!_is_ok);
    return std::move(_error);
  }

  Value &value() & {
    REPLIKON_ASSERT(_is_ok);
    return _value;
  }

  Error &error() & {
    REPLIKON_ASSERT(!_is_ok);
    return _error;
  }

private:
  void destroy() {
    if (_is_ok) {
      _value.~Value();
    } else {
      _error.~Error();
    }
  }

  union {
    Value _value;
    Error _error;
  };
  bool _is_ok;
};

} // namespace replikon

#endif // REPLIKON_EXPECTED_H