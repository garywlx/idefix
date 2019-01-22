#include <iostream>
#include <thread>

#include "webinterface/WebContext.h"

#ifdef CMAKE_PROJECT_VERSION
	#define PROJECT_VERSION CMAKE_PROJECT_VERSION
#else
	#define PROJECT_VERSION "unknown"
#endif

int main(int argc, char const *argv[])
{

	idefix::WebContext webContext(8891);
	
	std::thread t1([&webContext](){
		webContext.start();
	});

	std::cout << PROJECT_VERSION << std::endl;

	// try to send data to the connected clients
	int i = 0;
	while(true) {

		std::string msg = "hallo " + to_string( i );
		webContext.send( msg );

		i++;

		if ( i == 10 ) break;

		this_thread::sleep_for( chrono::seconds(2) );
	}

	t1.join();

	return 0;
}