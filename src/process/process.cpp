#include "process.hpp"

#include "logging.hpp"

#include "SDL3/SDL.h"

int PROC_init() {
  Log(LogLevel::info) << LOG_HEADER << "Initialising Processes";
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    Log(LogLevel::critical) << LOG_HEADER << "Failed to initialise SDL";
    return -1;
  }

  return 0;
}

void PROC_quit() {
  Log(LogLevel::info) << LOG_HEADER << "Stopping Processes";
  SDL_Quit();
}
