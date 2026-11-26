#include "ue3/result.h"

#include <cstdio>
#include <cstdlib>

namespace ue3 {

namespace detail {
void fail(const char *what)
{
  std::fprintf(stderr, "ue3: %s\n", what);
  std::fflush(stderr);
  std::abort();
}
} // namespace detail

Error Error::within(std::string_view where) &&
{
  std::string message;
  message.reserve(where.size() + 2 + message_.size());
  message.append(where);
  message.append(": ");
  message.append(message_);
  return Error(std::move(message));
}

} // namespace ue3
