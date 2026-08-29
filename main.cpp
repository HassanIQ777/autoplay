#include "Globals.hpp"
#include "helpers.hpp"
#include "libutils/CLIParser.hpp"

int main(int argc, char **argv) {
  Globals &g = Globals::getInstance();
  g.parser.setArgs(argc, argv);
  parseArgs(g);
  g.files.init();

  while (g.state != AppState::Quit) {
    funcs::clearTerminal();
    printLogo();

    switch (g.state) {
    case AppState::Start:
      //   state_start();
      break;
    case AppState::Watching:
      //   state_watching();
      break;
    case AppState::Settings:
      //   state_settings();
      break;
      // case AppState::Settings:
      //   state_settings();
      break;
    case AppState::Quit:
      break;
    }
  }
}