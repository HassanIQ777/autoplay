#pragma once

#include "Globals.hpp"
#include "libutils/color.hpp"

inline void LOG(const std::string &msg) {
  Globals &globals = Globals::getInstance();
  std::string date = funcs::currentTime();
  std::string output = date + " -> " + msg;
  File::insertline(globals.files.logs_file, output, 0);
}

inline void printHelp(Globals &globals) {
  const std::string program_name = globals.parser.getArg(0);
  print("Usage:\n");
  print("  ", program_name, " <HOME_DIR>\n");
  print("  ", program_name, " -h    print this help message\n");
  print("  ", program_name, " -v    print version\n");
}

inline void parseArgs(Globals &globals) {
  if (int argc = globals.parser.getArgc(); argc != 2) {
    if (argc == 1) {
      Log::error(0, "One argument is required but nothing was provided.");

    } else {

      Log::error(0, "One argument is required but " +
                        funcs::str(globals.parser.getArgc() - 1) +
                        " arguments were provided.");
    }
    printHelp(globals);
    exit(EXIT_FAILURE);
  }

  const std::string first_arg = globals.parser.getArg(1);
  if (first_arg == "-h") {
    printHelp(globals);
    exit(0);
  } else if (first_arg == "-v") {
    print("about-today version ", globals.VERSION, "\n");
    exit(0);
  }

  if (File::isdirectory(first_arg)) {
    globals.files.home_dir = first_arg;
  } else {
    Log::error(1, "The provided path is not a directory.");
  }
}

inline std::string getdate() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t time = std::chrono::system_clock::to_time_t(now);
  const std::tm tm = *std::localtime(&time);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%a %b %d %I:%M %p");
  //  this format looks like: Sat Jun 06 01:09 AM
  return oss.str();
}

inline void printLogo() {
  static constexpr const char *LOGO =
      R"(▄████▄ ▄▄ ▄▄ ▄▄▄▄▄▄ ▄▄▄  ▄▄▄▄  ▄▄     ▄▄▄  ▄▄ ▄▄ 
██▄▄██ ██ ██   ██  ██▀██ ██▄█▀ ██    ██▀██ ▀███▀ 
██  ██ ▀███▀   ██  ▀███▀ ██    ██▄▄▄ ██▀██   █   
)";
  std::string date = getdate();
  funcs::printLeftMiddleRight("", "", date);
  print("\n", color::TXT_YELLOW, color::A_BOLD, LOGO, color::A_RESET, "\n");
}

inline std::string shq(const std::string &s) {
  std::string out = "'";
  for (char c : s) {
    if (c == '\'')
      out += "'\\''";
    else
      out += c;
  }
  out += "'";
  return out;
}

// Joins a directory with a filename template, collapsing a trailing slash
// on dir so you never end up with "//%(title)s..." either way.
inline std::string joinOutPath(const std::string &dir,
                               const std::string &tmpl) {
  if (!dir.empty() && dir.back() == '/')
    return dir + tmpl;
  return dir + "/" + tmpl;
}

// Metadata-only lookup: no download happens, since --print without a
// later WHEN: stage implies --simulate. Returns the sanitized title
// yt-dlp would actually use in the filename.
inline std::string getSanitizedTitle(const std::string &url) {
  std::string cmd = "yt-dlp --print filename -o " + shq("%(title)s") + " " +
                    shq(url) + " 2>/dev/null";
  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    throw std::runtime_error("popen failed");
  std::string title;
  char buf[4096];
  while (fgets(buf, sizeof(buf), pipe))
    title += buf;
  int rc = pclose(pipe);
  while (!title.empty() && (title.back() == '\n' || title.back() == '\r'))
    title.pop_back();
  if (rc != 0 || title.empty())
    throw std::runtime_error("failed to resolve title for " + url);
  return title;
}