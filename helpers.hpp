#pragma once

#include "Globals.hpp"
#include "libutils/color.hpp"

inline void LOG(const std::string &msg) {
  Globals &globals = Globals::getInstance();
  std::string date = funcs::currentTime();
  std::string output = date + " -> " + msg;
  File::insertline(globals.files.logs_file, output, 0);
}

inline void printChoice(const std::string &num, const std::string &msg) {
  print(color::TXT_YELLOW, num, ") ", color::TXT_GREEN, msg, color::A_RESET,
        "\n");
}

inline void printMainMenu() {
  print("Select choice:\n");
  printChoice("1", "Video (Best Quality)");
  printChoice("2", "Video (720p)");
  printChoice("3", "Video (480p)");
  printChoice("4", "Video (360p)");
  printChoice("5", "Audio Only");
  print("> ");
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
  static constexpr const char *LOGO = R"()";
  std::string date = getdate();
  funcs::printLeftMiddleRight("", "", date);
  print("\n", color::TXT_YELLOW, color::A_BOLD, LOGO, color::A_RESET, "\n");
}