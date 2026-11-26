// The command-line tool. Every command prints one JSON document on stdout: its result,
// or {"error": {"message": ...}} with exit code 1.

#include "coredcc/base/json_writer.h"

#include <cstdio>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#ifndef MELEVEL_COREDCC_COMMIT
#define MELEVEL_COREDCC_COMMIT "unknown"
#endif

namespace {

constexpr const char *kTool = "melevel";
constexpr const char *kVersion = "0.1.0";

void print(std::string text)
{
  text.push_back('\n');
  std::fwrite(text.data(), 1, text.size(), stdout);
  std::fflush(stdout);
}

int printError(std::string_view message)
{
  coredcc::JsonWriter w;
  w.beginObject().key("error").beginObject().key("message").string(message);
  w.endObject().endObject();
  print(w.take());
  return 1;
}

int version()
{
  coredcc::JsonWriter w;
  w.beginObject();
  w.key("tool").string(kTool);
  w.key("version").string(kVersion);
  w.key("coredcc").string(MELEVEL_COREDCC_COMMIT);
  w.endObject();
  print(w.take());
  return 0;
}

} // namespace

int main(int argc, char **argv)
{
#ifdef _WIN32
  // The JSON's bytes exactly, without "\n" turned into "\r\n".
  _setmode(_fileno(stdout), _O_BINARY);
#endif
  if (argc < 2) {
    return printError("usage: melevel <command>; commands: version");
  }
  std::string_view command = argv[1];
  if (command == "version") {
    return version();
  }
  return printError("unknown command '" + std::string(command) + "'; commands: version");
}
