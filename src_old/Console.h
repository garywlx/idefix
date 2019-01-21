#ifndef IDEFIX_CONSOLE_H
#define IDEFIX_CONSOLE_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// used in AwesomeStrategy.cpp
// used in RenkoChart.cpp

namespace IDEFIX {
	inline std::shared_ptr<spdlog::logger> console() {
		return spdlog::get("console");
	}
};

#endif