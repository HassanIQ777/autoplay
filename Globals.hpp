#pragma once

#include "json.hpp"
#include "libutils/CLIParser.hpp"
#include "libutils/File.hpp"
#include "libutils/Log.hpp"
#include "libutils/funcs.hpp"
#include <filesystem>
#include <string>

using funcs::print;
using json = nlohmann::json;
namespace fs = std::filesystem;

struct Settings {
  std::string download_dir = "";

  static Settings defaults() { return Settings{}; }

  static Settings fromJson(const json &j) {
    Settings s = defaults(); // start from defaults
    s.download_dir = j.value("download_dir", s.download_dir);
    return s;
  }

  json toJson() const { return json{{"download_dir", download_dir}}; }

  void save(const std::string &filepath) {
    // Make sure parent dirs exist first
    std::filesystem::create_directories(
        std::filesystem::path(filepath).parent_path());

    std::ofstream file(filepath);
    if (!file.is_open())
      throw std::runtime_error("save(): couldn't open file: " + filepath);

    file << toJson().dump(4);

    if (!file.good())
      throw std::runtime_error("save(): write failed for: " + filepath);
  }

  // Returns true if it had to create the file (useful for "first run" logic)
  bool loadOrCreate(const std::string &filepath) {
    namespace fs = std::filesystem;

    if (!fs::exists(filepath)) {
      *this = defaults();
      save(filepath);
      return true;
    }

    std::ifstream file(filepath);
    if (!file.is_open())
      throw std::runtime_error("load(): couldn't open file: " + filepath);

    json data;
    try {
      file >> data; // parsing happens here
      *this = fromJson(data);
    } catch (const json::exception &e) {

      *this = defaults();
      save(filepath);
      return true;
    }

    return false;
  }
};

struct FilePaths {
  std::string home_dir, program_dir;    // directories
  std::string settings_file, logs_file; // files

  void init() {
    assignPaths();
    create();
  }

  void assignPaths() {
    // print("Chose ", home_dir, " as a home directory\n");
    program_dir = fs::path(home_dir) / ".autoplay";
    settings_file = fs::path(program_dir) / "settings.json";
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

    // settings_file & logs_file
    if (!File::isfile(settings_file)) {
      if (!File::createfile(settings_file)) {
        Log::error(1, "Unable to create: '", settings_file, "'");
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

enum class AppState { MainMenu, Downloading, Settings, Quit };

struct Globals {
  std::string VERSION = "26.8.31-1";
  FilePaths files;
  CLIParser parser;
  AppState state = AppState::MainMenu;
  Settings settings;

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