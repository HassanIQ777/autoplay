#pragma once

#include "Globals.hpp"
#include "libutils/color.hpp"
#include "libutils/funcs.hpp"
#include <cstdlib>

inline void LOG(const std::string &msg) {
  Globals &globals = Globals::getInstance();
  std::string date = funcs::currentTime();
  std::string output = date + " -> " + msg;
  File::insertline(globals.files.logs_file, output, 0);
}

inline void printHelp(Globals &globals) {
  const std::string program_name = globals.parser.getArg(0);
  print("Usage:\n");
  print("  ", program_name, "\n");
  print("  ", program_name, " <URL>\n");
  print("  ", program_name, " <HOME_DIR>\n");
  print("  ", program_name, " -h    print this help message\n");
  print("  ", program_name, " -v    print version\n");
}

inline void parseArgs(Globals &globals) {
  const std::string first_arg = globals.parser.getArg(1);
  if (first_arg == "-h") {
    printHelp(globals);
    exit(0);
  } else if (first_arg == "-v") {
    print("autoplay ", globals.VERSION, "\n");
    exit(0);
  }

  // if no args were provided, use $HOME the environment variable
  if (char *home = getenv("HOME");
      home != nullptr &&
      (globals.parser.getArgc() == 1 || !File::isdirectory(first_arg))) {
    globals.files.home_dir = home;
    return;
  }

  if (File::isdirectory(first_arg)) {
    globals.files.home_dir = first_arg;
  }
}

inline std::string getdate() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t time = std::chrono::system_clock::to_time_t(now);
  const std::tm tm = *std::localtime(&time);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%I:%M %p");
  //  this format looks like: 01:09 AM
  return oss.str();
}

inline void printLogo() {
  static constexpr const char *LOGO =
      R"(        ▄████▄ ▄▄ ▄▄ ▄▄▄▄▄▄ ▄▄▄  ▄▄▄▄  ▄▄     ▄▄▄  ▄▄ ▄▄ 
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

inline bool isVideoFile(const std::string &filename) {
  static const std::vector<std::string> VIDEO_EXTENSIONS = {
      ".mp4", ".mkv", ".mov", ".avi", ".wmv", ".flv", ".webm", ".mpeg"};

  std::string ext = File::getExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 ::tolower); // Convert to lowercase
  return std::find(VIDEO_EXTENSIONS.begin(), VIDEO_EXTENSIONS.end(), ext) !=
         VIDEO_EXTENSIONS.end();
}