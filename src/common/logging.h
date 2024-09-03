#pragma once

#include <SDL2/SDL_log.h>

// TODO: create a proper logging solution using <fmt>

#define LOG_CRITICAL(...) SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define LOG_ERROR(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#ifndef NDEBUG
#define LOG_DEBUG(...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)

#endif
