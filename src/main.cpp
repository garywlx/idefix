/*!
 * Copyright(c)2018, Arne Gockeln. All rights reserved.
 * https://arnegockeln.com
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "FIXManager.h"
#include "LogaStrategy.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/file_sinks.h>

using namespace std;
using namespace IDEFIX;

int main(int argc, char** argv) {
	try {
		if ( argc != 2 ) {
			cout << "Usage:" << endl;
			cout << "   idefix <configfile>" << endl;
			cout << endl;
			
			return EXIT_FAILURE;
		}

		// log initialization
		try {
			UtcTimeStamp nowTS;
			int year,month,day;
			nowTS.getYMD( year, month, day );

			// trades-YYYY-mm-dd.log
			char buffer[22];
			sprintf(buffer, "trades-%d-%02d-%02d.log", year, month, day);
			std::string trade_log_file = buffer;

			auto logger = spdlog::basic_logger_mt("tradlog", trade_log_file );
			logger->set_pattern( "%Y-%m-%d %T.%e,%v" );

			logger->info( "EUR/USD,L,1.12345,0,1.1236,2,32.89" );
			std::this_thread::sleep_for( chrono::seconds(3) );
			logger->info( "GBP/USD,S,1.2543,0,1.2546,-3,-30.00" );
		} catch( const spdlog::spdlog_ex& ex ) {
			cout << "Log init failed: " << ex.what() << endl;
			return EXIT_FAILURE;
		}
		
		return EXIT_SUCCESS;


		cout << "- Press 0 to exit -" << endl;
		// config file
		const std::string config_file = argv[1];
		// init strategy
		LogaStrategy logaStrategy;
		// init fix manager
		FIXManager fixmanager( config_file, &logaStrategy );

		// start loop
		while ( true ) {
			int command = 0;
			cin >> command;

			if ( command == 0 ) {
				fixmanager.onExit();
				break;
			}
		}

		// End Session and logout
		fixmanager.endSession();

		// give another 1 second to clean up
		this_thread::sleep_for( chrono::seconds(1) );

	} catch(std::exception& e) {
		cout << e.what() << endl;
		return EXIT_FAILURE;
	} catch(...) {
		cout << "Unknonwn exception" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}