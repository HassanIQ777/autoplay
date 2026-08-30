#pragma once

#include "Globals.hpp"
#include "helpers.hpp"
#include "libutils/Input.hpp"
#include "libutils/color.hpp"
#include "libutils/funcs.hpp"
#include <filesystem>
#include <string>

inline void printChoice(const std::string &num, const std::string &msg) {
  print(color::TXT_MAGENTA, num, ") ", color::TXT_GREEN, msg, color::A_RESET,
        "\n");
}

inline void stateMainMenu() {
  static constexpr const char *LOGO = R"(▖  ▖▄▖▄▖▖ ▖  ▖  ▖▄▖▖ ▖▖▖
▛▖▞▌▌▌▐ ▛▖▌  ▛▖▞▌▙▖▛▖▌▌▌
▌▝ ▌▛▌▟▖▌▝▌  ▌▝ ▌▙▖▌▝▌▙▌
                        )";

  print(LOGO, "\n");
  Globals &g = Globals::getInstance();

  printChoice("1", "Download");
  printChoice("2", "Settings");
  print("\n");
  printChoice("9", "Quit");

  std::string inp = funcs::getKeyPress();
  if (inp == "1") {
    if (!File::isdirectory(g.settings.download_dir)) {
      Log::error(false,
                 "Download directory invalid, please set it in settings.");
      return;
    }
    g.state = AppState::Downloading;
  } else if (inp == "2") {
    g.state = AppState::Settings;
  } else if (inp == "9") {
    g.state = AppState::Quit;
  }
}

inline bool commandExists(const std::string &name) {
  return system(("command -v " + shq(name) + " >/dev/null 2>&1").c_str()) == 0;
}

inline bool ytdlpSupportsJsRuntimes() {
  FILE *pipe = popen("yt-dlp --help 2>/dev/null", "r");
  if (!pipe)
    return false;
  std::string output;
  char buf[4096];
  while (fgets(buf, sizeof(buf), pipe))
    output += buf;
  pclose(pipe);
  return output.find("--js-runtimes") != std::string::npos;
}

// not a real state
inline void stateWatching(const std::string &path) {
  while (1) {
    funcs::clearTerminal();
    printLogo();
    static constexpr const char *LOGO = R"(▖  ▖  ▗   ▌ ▘    
▌▞▖▌▀▌▜▘▛▘▛▌▌▛▌▛▌
▛ ▝▌█▌▐▖▙▖▌▌▌▌▌▙▌
               ▄▌)";

    print(LOGO, "\n");
    Globals &g = Globals::getInstance();

    printChoice("1", "Play");
    printChoice("2", "Delete media");
    print("\n");
    printChoice("9", "Return to start");

    std::string inp = funcs::getKeyPress();
    if (inp == "1") {
      std::string cmd = "mpv " + shq(path);
      int rc = system(cmd.c_str());
      if (rc != 0) {
        auto msg = "[!] mpv exited with a non-zero status.";
        LOG(msg);
        print(msg, "\n");
        funcs::getKeyPress();
      }
    } else if (inp == "2") {
      if (!File::removefile(path)) {
        Log::warn("Failed to remove '", path, "'");
        funcs::getKeyPress();
      }
      break;
    } else if (inp == "9") {
      g.state = AppState::MainMenu;
      break;
    }
  }
}

inline void stateDownloading(std::string URL = "") {
  static constexpr const char *LOGO = R"(▄        ▜      ▌▘    
▌▌▛▌▌▌▌▛▌▐ ▛▌▀▌▛▌▌▛▌▛▌
▙▘▙▌▚▚▘▌▌▐▖▙▌█▌▙▌▌▌▌▙▌
                    ▄▌)";

  print(LOGO, "\n");
  Globals &g = Globals::getInstance();

  auto url_inp = Input::readline<std::string>("URL: ");
  if (!url_inp) {
    return;
  }
  URL = *url_inp;

  printChoice("1", "Video (Best Quality)");
  printChoice("2", "Video (720p)");
  printChoice("3", "Video (480p)");
  printChoice("4", "Video (360p)");
  printChoice("5", "Audio Only");
  print("> ");
  std::string inp = funcs::getKeyPress();

  // --- flags shared by every mode ---
  std::string commonFlags =
      "--downloader aria2c"
      " --downloader-args " +
      shq("aria2c:-x 16 -s 16 -k 1M") +
      " --embed-metadata"
      " --embed-thumbnail"
      " --sponsorblock-remove sponsor"
      " --download-archive " +
      shq(g.files.program_dir / fs::path("archive.txt")) +
      " --ignore-errors"
      " --sleep-subtitles 2"
      " --user-agent " +
      shq("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
          "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

  // Deno is yt-dlp's default JS runtime and is auto-detected if it's on
  // PATH, so --js-runtimes is only needed for a non-default runtime or a
  // nonstandard Deno path. Only add it if deno is actually installed AND
  // this yt-dlp build supports the flag, so this never breaks on an
  // older/different yt-dlp binary that lacks it.
  if (commandExists("deno") && ytdlpSupportsJsRuntimes()) {
    commonFlags += " --js-runtimes deno";
  }

  static const std::string videoFlags = "--merge-output-format mkv"
                                        " --embed-subs"
                                        " --write-auto-subs"
                                        " --sub-langs " +
                                        shq("en.*,ar.*");

  std::string format;
  bool audioOnly = false;

  // VIDEO
  if (inp == "1") {
    format = "bestvideo+bestaudio/best";
  } else if (inp == "2") {
    format = "bestvideo[height<=720]+bestaudio/best[height<=720]";
  } else if (inp == "3") {
    format = "bestvideo[height<=480]+bestaudio/best[height<=480]";
  } else if (inp == "4") {
    format = "bestvideo[height<=360]+bestaudio/best[height<=360]";
  }
  // AUDIO ONLY
  else {
    audioOnly = true;
  }

  std::string cmd = "yt-dlp ";
  if (audioOnly) {
    cmd += "-x --audio-format mp3 --audio-quality 0 " + commonFlags;
  } else {
    cmd += "-f " + shq(format) + " " + commonFlags + " " + videoFlags;
  }
  std::string outPath =
      joinOutPath(g.settings.download_dir, "%(title)s.%(ext)s");
  cmd += " -o " + shq(outPath) + " " + shq(URL);

  int rc = system(cmd.c_str());
  if (rc != 0) {
    LOG("[!] yt-dlp exited with a non-zero status.");
  }

  std::string title = getSanitizedTitle(URL);
  std::string ext = audioOnly ? "mp3" : "mkv";
  std::string downloadedPath =
      joinOutPath(g.settings.download_dir, title + "." + ext);

  if (!File::isfile(downloadedPath)) {
    print("Failed to download media.\n");
    funcs::getKeyPress();
    g.state = AppState::MainMenu;
    return;
  }

  stateWatching(downloadedPath);
}

inline void stateSettings() {
  static constexpr const char *LOGO = R"(▄▖  ▗ ▗ ▘      
▚ █▌▜▘▜▘▌▛▌▛▌▛▘
▄▌▙▖▐▖▐▖▌▌▌▙▌▄▌
           ▄▌  )";

  print(LOGO, "\n");
  Globals &g = Globals::getInstance();

  std::string download_path = g.settings.download_dir;
  if (!download_path.empty()) {
    download_path = fs::absolute(g.settings.download_dir);
  }
  printChoice("1", "Download path = " + download_path);
  // printChoice("2", "");
  print("\n");
  printChoice("9", "Back");

  std::string inp = funcs::getKeyPress();

  if (inp == "1") {
    print("New path: ");
    auto path = Input::readline<std::string>();
    if (!path) {
      return;
    }
    if (!File::isdirectory(*path)) {
      Log::warn("Invalid path to a directory.");
      funcs::getKeyPress();
      return;
    }
    g.settings.download_dir = *path;

    // } else if (inp == "2") {
  } else if (inp == "9") {
    g.state = AppState::MainMenu;
  }

  g.settings.save(g.files.settings_file);
}
