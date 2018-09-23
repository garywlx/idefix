
#include <random>
#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <thread>
#include <chrono>

#include <quickfix/application.h>

using namespace FIX;
using namespace std;

// Entry
int main(int argc, char** argv) {
	// log initialization
	try {
		UtcTimeStamp nowTS;
		int year,month,day;
		nowTS.getYMD( year, month, day );

		auto console = spdlog::stdout_color_mt( "console" );
		console->info( "Start logging..." );

		// get daily rotating tradelogger, create new log file on 23:59 every day
		auto tradelog = spdlog::daily_logger_mt("tradelog", "trades/trades.log", 0, 0 );
		// set tradelog pattern
		tradelog->set_pattern( "%Y-%m-%d %T.%e,%v" );
		// flush logger every
		spdlog::flush_every( chrono::seconds( 3 ) );

		// generate random numbers DEBUG ONLY
		uniform_real_distribution<double> unif(1.0, 2.0);
		default_random_engine re;

		console->trace("I am a trace message");
		console->debug("I am a debug message");
		console->info("I am an info message");
		console->error("I am an error message");
		console->warn("I am a warning message");
		console->critical("I am a critical message");


		int rounds = 0;

		// while( true ) {
		// 	console->info( "Round: {:d}", rounds );

		// 	double entry = unif(re);
		// 	double tp = unif(re);

		// 	double pips = entry - tp;
		// 	tradelog->info( "{:s},L,{:1.5f},0,{:1.5f},{:1.2f},{:1.2f}", "EUR/USD", entry, tp, pips, pips * 10 );

		// 	rounds++;

		// 	this_thread::sleep_for( chrono::seconds( 1 ) );

		// 	if ( rounds == 120 ) {
		// 		break;
		// 	}
		// }

		console->info( "Done with {:d} rounds.", rounds );

	} catch( const spdlog::spdlog_ex& ex ) {
		cout << "Log init failed: " << ex.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}