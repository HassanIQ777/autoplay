#include "libutils/color.hpp"
#include "libutils/funcs.hpp"
using funcs::print;

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