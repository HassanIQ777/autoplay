#include "Globals.hpp"
#include "app_states.hpp"
#include "helpers.hpp"
#include "libutils/CLIParser.hpp"

int main(int argc, char **argv) {
  Globals &g = Globals::getInstance();
  g.parser.setArgs(argc, argv);
  parseArgs(g);
  g.files.init();
  g.settings.loadOrCreate(g.files.settings_file);
  LOG("User started program");

  if (funcs::hasSequence(g.parser.getArg(1), "http")) {
    g.state = AppState::Downloading;
    funcs::clearTerminal();
    printLogo(); // بدون مجاملة
    stateDownloading(g.parser.getArg(1));
  }

  while (g.state != AppState::Quit) {
    funcs::clearTerminal();
    printLogo();

    switch (g.state) {
    case AppState::MainMenu:
      stateMainMenu();
      break;
    case AppState::Downloading:
      stateDownloading();
      break;
    case AppState::Settings:
      stateSettings();
      break;
    case AppState::Quit:
      break;
    }
  }
  LOG("User quit program");
}