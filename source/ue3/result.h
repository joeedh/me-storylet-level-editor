#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>

/**
 * Errors in the ue3 layer. There are no exceptions: anything that can fail returns a
 * Status or a Result<T>. ue3 doesn't depend on coredcc, so this is its own small type
 * rather than coredcc's Status; the app layer converts between them.
 */
namespace ue3 {

namespace detail {
/** Prints `what` to stderr and aborts. For misuse of a Result, which is a bug. */
[[noreturn]] void fail(const char *what);
} // namespace detail

/**
 * What went wrong, as text the CLI prints in its JSON. Each layer that passes an error up
 * adds where it was, outermost first:
 * "export 12 (TheWorld.PersistentLevel.Foo): property Bar at 0x40: unknown struct".
 */
class Error {
public:
  explicit Error(std::string message) : message_(std::move(message)) {}

  const std::string &message() const { return message_; }

  /** The same error, with `where` and ": " in front of its message. */
  Error within(std::string_view where) &&;

private:
  std::string message_;
};

/** Ok, or an Error. */
class [[nodiscard]] Status {
public:
  Status() = default;
  Status(Error error) : error_(std::move(error)) {}

  bool ok() const { return !error_.has_value(); }

  const Error &error() const
  {
    if (ok()) {
      detail::fail("Status::error() on an ok Status");
    }
    return *error_;
  }

  Error takeError() &&
  {
    if (ok()) {
      detail::fail("Status::takeError() on an ok Status");
    }
    return std::move(*error_);
  }

private:
  std::optional<Error> error_;
};

/** A T, or an Error. */
template <typename T> class [[nodiscard]] Result {
public:
  Result(T value) : value_(std::move(value)) {}
  Result(Error error) : error_(std::move(error)) {}

  bool ok() const { return value_.has_value(); }

  T &value() &
  {
    check();
    return *value_;
  }
  const T &value() const &
  {
    check();
    return *value_;
  }
  T &&value() &&
  {
    check();
    return std::move(*value_);
  }

  const Error &error() const
  {
    if (ok()) {
      detail::fail("Result::error() on an ok Result");
    }
    return *error_;
  }

  Error takeError() &&
  {
    if (ok()) {
      detail::fail("Result::takeError() on an ok Result");
    }
    return std::move(*error_);
  }

  Status status() const { return ok() ? Status() : Status(*error_); }

private:
  void check() const
  {
    if (!ok()) {
      detail::fail("Result::value() on an error");
    }
  }

  std::optional<T> value_;
  std::optional<Error> error_;
};

} // namespace ue3

#define UE3_CAT_INNER(a, b) a##b
#define UE3_CAT(a, b) UE3_CAT_INNER(a, b)

/** Returns the error of a Status or Result expression, if it has one. */
#define UE3_TRY(expr)                                                                    \
  do {                                                                                   \
    auto ue3_try_ = (expr);                                                              \
    if (!ue3_try_.ok()) {                                                                \
      return std::move(ue3_try_).takeError();                                            \
    }                                                                                    \
  } while (0)

/** `lhs = value of expr`, or returns its error. `lhs` may be a declaration. */
#define UE3_TRY_ASSIGN(lhs, expr) UE3_TRY_ASSIGN_IMPL(UE3_CAT(ue3_try_, __LINE__), lhs, expr)
#define UE3_TRY_ASSIGN_IMPL(tmp, lhs, expr)                                              \
  auto tmp = (expr);                                                                     \
  if (!tmp.ok()) {                                                                       \
    return std::move(tmp).takeError();                                                   \
  }                                                                                      \
  lhs = std::move(tmp).value()
