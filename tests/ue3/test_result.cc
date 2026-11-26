#include "ue3/result.h"

#include "test_util.h"

#include <string>

test_init;

using ue3::Error;
using ue3::Result;
using ue3::Status;

static Status failIf(bool fail)
{
  if (fail) {
    return Error("unknown struct");
  }
  return Status();
}

static Status chain(bool fail)
{
  UE3_TRY(failIf(fail));
  return Error("reached the end");
}

static Result<int> parse(std::string s)
{
  if (s.empty()) {
    return Error("empty");
  }
  return int(s.size());
}

static Result<int> doubled(std::string s)
{
  UE3_TRY_ASSIGN(int n, parse(std::move(s)));
  return n * 2;
}

static Status withContext()
{
  Status s = failIf(true);
  if (!s.ok()) {
    return std::move(s).takeError().within("property Bar at 0x40").within("export 12");
  }
  return Status();
}

int main()
{
  {
    Status ok;
    test_assert(ok.ok());
  }
  {
    Status e = chain(true);
    test_assert(!e.ok());
    test_assert(e.error().message() == "unknown struct");
    test_assert(chain(false).error().message() == "reached the end");
  }
  {
    Result<int> r = parse("abc");
    test_assert(r.ok());
    test_assert(r.value() == 3);
    test_assert(r.status().ok());
    Result<int> bad = parse("");
    test_assert(!bad.ok());
    test_assert(bad.error().message() == "empty");
    test_assert(!bad.status().ok());
  }
  {
    test_assert(doubled("abcd").value() == 8);
    test_assert(doubled("").error().message() == "empty");
  }
  {
    test_assert(withContext().error().message() ==
                "export 12: property Bar at 0x40: unknown struct");
  }
  {
    // A move-only value passes through.
    Result<std::string> r = std::string("names");
    std::string taken = std::move(r).value();
    test_assert(taken == "names");
  }
  return test_end();
}
