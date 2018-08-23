/*!
 * Copyright(c)2018, Arne Gockeln. All rights reserved.
 * https://arnegockeln.com
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "FIXManager.h"
#include "RenkoStrategy.h"

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
		// init strategy
		RenkoStrategy renkostrategy;
		// init fix manager
		FIXManager fixmanager( config_file, &renkostrategy );

		if ( ! fixmanager.isExiting() ) {
			// output 
			fixmanager.console()->info( "[INFO] - Press 0 to exit! -" );
		}

		// start loop
		while ( ! fixmanager.isExiting() ) {
			int command = 0;
			cin >> command;

			if ( command == 0 ) {
				fixmanager.setExiting( true );
				break;
			}
		}

		// Call onExit handler
		fixmanager.onExit();

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