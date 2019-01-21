#include <iostream>
#include "webinterface/WebContext.h"

#ifdef CMAKE_PROJECT_VERSION
	#define PROJECT_VERSION CMAKE_PROJECT_VERSION
#else
	#define PROJECT_VERSION "unknown"
#endif

int main(int argc, char const *argv[])
{

	idefix::WebContext webContext(8080);
	webContext.start();


	std::cout << PROJECT_VERSION << std::endl;

	return 0;
}