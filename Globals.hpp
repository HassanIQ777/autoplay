#pragma once

#include "libutils/CLIParser.hpp"
#include "libutils/File.hpp"
#include "libutils/Log.hpp"
#include "libutils/funcs.hpp"
#include <filesystem>
#include <string>

using funcs::print;
namespace fs = std::filesystem;

struct FilePaths {
  std::string home_dir, program_dir;  // directories
  std::string config_file, logs_file; // files

  void init() {
    assignPaths();
    create();
  }

  void assignPaths() {
    // print("Chose ", home_dir, " as a home directory\n");
    program_dir = fs::path(home_dir) / ".autoplay";
    config_file = fs::path(program_dir) / "config.ini";
    logs_file = fs::path(program_dir) / "logs.txt";
  }

  void create() {
    // create program_dir & days_dir
    if (!File::isdirectory(program_dir)) {
      if (!File::createdir(program_dir)) {
        Log::error(1, "Unable to create program's home directory at: '",
                   program_dir, "'");
      }
    }

    // config_file & logs_file
    if (!File::isfile(config_file)) {
      if (!File::createfile(config_file)) {
        Log::error(1, "Unable to create: '", config_file, "'");
      }
    }
    if (!File::isfile(logs_file)) {
      if (!File::createfile(logs_file)) {
        Log::error(1, "Unable to create: '", logs_file, "'");
      }
    }
  }

  bool createFile(const std::string &fp) {
    if (!File::isfile(fp)) {
      if (File::createfile(fp)) {
        // LOG("Successfully created '" + fp + "'");
        return true; // we newly created this
      } else {
        // LOG("Failed to create '" + fp + "'");
        exit(-3);
      }
    }
    return false; // already created
  }
};

enum class AppState { Start, Watching, Settings, Quit };

struct Globals {
  std::string VERSION = "26.8.30";
  FilePaths files;
  CLIParser parser;
  AppState state = AppState::Start;

  static Globals &getInstance() {
    static Globals g;
    return g;
  }

  Globals(const Globals &) = delete;
  Globals(Globals &&) = delete;
  Globals &operator=(const Globals &) = delete;
  Globals &operator=(Globals &&) = delete;

private:
  Globals() = default;
}; // Singleton struct Globals