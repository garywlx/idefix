#ifndef IDEFIX_CONSOLE_H
#define IDEFIX_CONSOLE_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace IDEFIX {
	inline void registerConsole() {
		// setup console
		auto console = spdlog::stdout_color_mt( "console" );
		// set console pattern
		console->set_pattern( "%Y-%m-%d %T.%e: %^%v%$" );
	}

	inline std::shared_ptr<spdlog::logger> console() {
		return spdlog::get("console");
	}
};

#endif