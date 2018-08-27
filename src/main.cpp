/*!
 * Copyright(c)2018, Arne Gockeln. All rights reserved.
 * https://arnegockeln.com
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "FIXManager.h"
#include "RenkoChart.h"
#include "strategies/RenkoStrategy.h"
#include "indicator/SimpleMovingAverage.h"

using namespace std;
using namespace IDEFIX;

int main(int argc, char** argv) {
	try {
		if ( argc != 2 ) {
			cout << "IDEFIX "; IDEFIX_VERSION(); cout << endl;
			cout << "Usage:" << endl;
			cout << "   idefix <configfile>" << endl;
			cout << endl;
			
			return EXIT_SUCCESS;
		}

		// config file
		const std::string config_file = argv[1];
		
		// init fix manager
		FIXManager fixmanager;
		
		// Add chart with strategy
		RenkoStrategy strategy;
		RenkoChart chart( 3 );
		chart.set_symbol( "EUR/USD" );
		chart.set_strategy( &strategy );

		// SimpleMovingAverage sma5( 5 );
		// chart.add_indicator( &sma5 );

		fixmanager.add_chart( &chart );

		// connect 
		fixmanager.connect( config_file );

		// start loop
		while ( ! fixmanager.isExiting() ) {
			int command = 0;
			cin >> command;

			if ( command == 0 ) {
				fixmanager.setExiting( true );
				break;
			}
		}

		// give another 1 second to clean up
		//this_thread::sleep_for( chrono::seconds(1) );

	} catch(std::exception& e) {
		cout << "something horrible went wrong: " << e.what() << endl;
		return EXIT_FAILURE;
	} catch(...) {
		cout << "Unknonwn exception" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}