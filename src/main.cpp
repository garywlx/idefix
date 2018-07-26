/*!
 * Copyright(c)2018, Arne Gockeln. All rights reserved.
 * https://arnegockeln.com
 */

#include <iostream>
#include <thread>
#include <chrono>
#include "FIXManager.h"
#include "LogaStrategy.h"

using namespace std;
using namespace IDEFIX;

#define IDEFIX_VERSION "0.1"

int main(int argc, char** argv) {
	try {
		if ( argc != 2 ) {
			cout << "Integrated Development Environment for Financial Information Exchange (IDEFIX " << IDEFIX_VERSION << ")" << endl;
			cout << "Usage:" << endl;
			cout << "   idefix <configfile>" << endl;
			cout << endl;
			
			return EXIT_FAILURE;
		}

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