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
}