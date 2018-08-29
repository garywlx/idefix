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
		// AUD/USD
		RenkoChart audusd_chart( "AUD/USD", 1 );
		audusd_chart.set_strategy( new RenkoStrategy() );
		fixmanager.add_chart( &audusd_chart );

		// EUR/USD
		RenkoChart eurusd_chart( "EUR/USD", 1 );
		eurusd_chart.set_strategy( new RenkoStrategy() );
		fixmanager.add_chart( &eurusd_chart );
		
		// GBP/USD
		RenkoChart gbpusd_chart( "GBP/USD", 1 );
		gbpusd_chart.set_strategy( new RenkoStrategy() );
		fixmanager.add_chart( &gbpusd_chart );

		// NZD/USD
		RenkoChart nzdusd_chart( "NZD/USD", 1 );
		nzdusd_chart.set_strategy( new RenkoStrategy() );
		fixmanager.add_chart( &nzdusd_chart );

		// USD/CAD
		RenkoChart usdcad_chart( "USD/CAD", 1 );
		usdcad_chart.set_strategy( new RenkoStrategy() );
		fixmanager.add_chart( &usdcad_chart );

		// USD/CHF
		RenkoChart usdchf_chart( "USD/CHF", 1 );
		usdchf_chart.set_strategy( new RenkoStrategy() );
		fixmanager.add_chart( &usdchf_chart );

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

		fixmanager.disconnect();

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