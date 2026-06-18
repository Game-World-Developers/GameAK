#pragma once

#ifndef GAME_AK_ENABLE_LOGGING
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#ifdef GAME_AK_HAVE_SPDLOG
#include <spdlog/spdlog.h>
#else
#define SPDLOG_DEBUG(...) ((void)(0 && (__VA_ARGS__, 1)))
#define SPDLOG_WARN(...)  ((void)(0 && (__VA_ARGS__, 1)))
#endif
