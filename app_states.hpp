#pragma once

#include "Globals.hpp"
#include "helpers.hpp"
#include "libutils/Input.hpp"
#include "libutils/color.hpp"
#include "libutils/cursor.hpp"
#include "libutils/funcs.hpp"
#include "libutils/strutils.hpp"
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
  printChoice("3", "Help");

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
  } else if (inp == "3") {
    g.state = AppState::Help;
  } else if (inp == "9" || inp == "q" || inp == "0") {
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
    printChoice("9", "Keep media");

    std::string inp = funcs::getKeyPress();
    if (inp == "1") {
      // launch in Android's MPV Player
      LOG("playing: '" + path + "'");
      if (g.isMobileDevice && isVideoFile(path)) {
        const std::string command =
            "am start -a android.intent.action.VIEW -d \"file://" + path +
            "\" -n is.xyz.mpv/.MPVActivity";
        system(command.c_str());
      } else {
        std::string cmd = "mpv "
                          "--audio-display=no " +
                          shq(path);
        int rc = system(cmd.c_str());
        if (rc != 0) {
          auto msg = "[!] mpv exited with a non-zero status.";
          LOG(msg);
          print(msg, "\n");
          funcs::getKeyPress();
        }
      }
    } else if (inp == "2") {
      g.state = AppState::MainMenu;
      if (!File::removefile(path)) {
        auto msg = "[autoplay] Failed to remove '" + path + "'";
        Log::warn(msg);
        LOG(msg);
      } else {
        auto msg =
            "[autoplay] Successfully removed '" + File::getFileName(path) + "'";
        print(msg, "\n");
        LOG(msg);
      }
      print("Press anything to go back to main menu\n");
      funcs::getKeyPress();
      return;
    } else if (inp == "9" || inp == "q" || inp == "0") {
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

  bool goUp = false;
  if (URL == "") {
    auto url_inp = Input::readline<std::string>("URL: ");
    if (!url_inp) {
      return;
    }
    URL = *url_inp;
    goUp = true;
  }

  // cancel downloading
  if (URL == "") {
    g.state = AppState::MainMenu;
    return;
  }

  if (goUp) {
    cursor::up();
  }
  print("Select option for '", URL, "':\n");
  print("─────────────────────", strutils::repeat("─", URL.size()), "\n");
  printChoice("0", "Cancel");
  printChoice("1", "Video (Best Quality)");
  printChoice("2", "Video (720p)");
  printChoice("3", "Video (480p)");
  printChoice("4", "Video (360p)");
  printChoice("5", "Audio Only");
  print("> ");
  std::string inp = funcs::getKeyPress();

  if (inp == "9" || inp == "q" || inp == "0") {
    g.state = AppState::MainMenu;
    return;
  }

  print("\nStarted downloading...\n");
  LOG("Started downloading: (" + URL = ")");

  static int sleep_subsitles = 2;
  // --- flags shared by every mode ---
  std::string commonFlags =
      "--downloader aria2c"
      " --downloader-args " +
      shq("aria2c:-x 16 -s 16 -k 1M") +
      " --ignore-errors"
      " --sleep-subtitles " +
      funcs::str(sleep_subsitles) + " --user-agent " +
      shq("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
          "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

  // Deno is yt-dlp's default JS runtime and is auto-detected if it's on
  // PATH, so --js-runtimes is only needed for a non-default runtime or a
  // nonstandard Deno path. Only add it if deno is actually installed AND
  // this yt-dlp build supports the flag, so this never breaks on an
  // older/different yt-dlp binary that lacks it.
  if (commandExists("deno") && ytdlpSupportsJsRuntimes())
    commonFlags += " --js-runtimes deno";

  if (g.settings.add_thumbnail)
    commonFlags += " --embed-thumbnail";

  if (g.settings.add_metadata)
    commonFlags += " --embed-metadata";

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
    cmd += "-x --audio-format mp3 --audio-quality 0 --no-video " + commonFlags;
  } else {
    cmd += "-f " + shq(format) + " " + commonFlags + " " + videoFlags;
  }
  std::string outPath =
      joinOutPath(g.settings.download_dir, "%(title)s.%(ext)s");
  cmd += " -o " + shq(outPath) + " " + shq(URL);

  bool download_failed = false;
  int rc = system(cmd.c_str());
  if (rc != 0) {
    LOG("[!] yt-dlp exited with a non-zero status.");
    download_failed = true;
  }

  std::string title = getSanitizedTitle(URL);
  std::string ext = audioOnly ? "mp3" : "mkv";
  std::string downloadedPath =
      joinOutPath(g.settings.download_dir, title + "." + ext);

  if (!File::isfile(downloadedPath)) {
    print("[autoplay] Failed to download media.\n");
    download_failed = true;
  }

  if (download_failed) {
    if (!audioOnly) { // only inscrease cooldown if it's video
      sleep_subsitles += 1;
      print("[Settings] sleep subtitles has been increased to ",
            sleep_subsitles, " seconds.\n\n");
    }
    auto choice = Input::readline<std::string>("\nRetry download [Y/n]? ");
    if (funcs::uppercase(*choice) == "N") {
      g.state = AppState::MainMenu;
      return;
    }
    funcs::clearTerminal();
    printLogo();
    stateDownloading(URL);
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
  std::string add_metadata_str = (g.settings.add_metadata) ? "Yes" : "No";
  std::string add_thumbnail_str = (g.settings.add_thumbnail) ? "Yes" : "No";
  if (!download_path.empty()) {
    download_path = fs::absolute(g.settings.download_dir);
  }
  print("\nUsing '", g.files.program_dir, "' as the program's directory.\n");
  print("\n");
  printChoice("1", "Download path: " + download_path);
  printChoice("2", "Add metadata: " + add_metadata_str);
  printChoice("3", "Add thumbnail: " + add_thumbnail_str);
  print("\n");
  printChoice("9", "Back");

  const std::string inp = funcs::getKeyPress();

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
    LOG("[Settings] Changed download path to: '" + *path + "'");

  } else if (inp == "3") {
    g.settings.add_thumbnail = !g.settings.add_thumbnail;
    add_thumbnail_str = (g.settings.add_thumbnail) ? "Yes" : "No";
    LOG("[Settings] Add thumbnail: " + add_thumbnail_str);
  } else if (inp == "2") {
    g.settings.add_metadata = !g.settings.add_metadata;
    add_metadata_str = (g.settings.add_metadata) ? "Yes" : "No";
    LOG("[Settings] Add metadata: " + add_metadata_str);
  }

  else if (inp == "9" || inp == "q" || inp == "0") {
    g.state = AppState::MainMenu;
  }

  g.settings.save(g.files.settings_file);
}

inline void stateHelp() {
  static constexpr const char *LOGO = R"(▖▖  ▜   
▙▌█▌▐ ▛▌
▌▌▙▖▐▖▙▌
      ▌ )";

  print(LOGO, "\n");
  Globals &g = Globals::getInstance();

  print("\nAutoplay ", g.VERSION,
        "\nA program written by HassanIQ777 "
        "(https://github.com/hassaniq777)\n");
  print("Its purpose is to quickly but temporarily download media and play, "
        "you can optionally keep it too.\n\n");
  printHelp();
  print("\n\n");
  print("Note: audio only is the default downloading option, so you can just "
        "hit 'Enter' to choose it.\n\n");

  printChoice("9", "Back");

  std::string inp = funcs::getKeyPress();
  if (inp == "9" || inp == "q" || inp == "0") {
    g.state = AppState::MainMenu;
  }
}
