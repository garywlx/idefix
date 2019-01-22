#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// SPDLOG_INFO("Welcome to spdlog!");
// SPDLOG_ERROR("Some error message with arg: {}", 1);
// SPDLOG_WARN("Easy padding in numbers like {:08d}", 12);
// SPDLOG_CRITICAL("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
// SPDLOG_DEBUG("This message should be displayed..");    

// SPDLOG_INFO("Support for floats {:03.2f}", 1.23456);
// SPDLOG_INFO("Positional args are {1} {0}..", "too", "supported");
// SPDLOG_INFO("{:<30}", "left aligned");

// spdlog::set_level(spdlog::level::debug); // Set global log level to debug
// SPDLOG_DEBUG("This message should be displayed..");    

// change log pattern
// spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
// https://github.com/gabime/spdlog/wiki/3.-Custom-formatting