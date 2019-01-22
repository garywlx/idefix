#include "adapter.h"
#include <dlfcn.h>
#include <iostream>

namespace idefix {

Adapter::~Adapter() {}

Adapter* Adapter::load(const std::string &sofile) {
	std::cout << "Trying to load " << sofile << std::endl;

	auto handle = dlopen( sofile.c_str(), RTLD_NOW );
	if ( !handle ) {
		std::cerr << dlerror() << std::endl;
		return nullptr;
	}

	auto create_func = (CFunc) dlsym( handle, "create" );
	if ( !create_func ) {
		std::cerr << dlerror() << std::endl;
		return nullptr;
	}

	auto out = create_func();
	if ( out ) {
		out->m_create_func = [=]() { 
			return create_func(); 
		};
	}

	return out;
}
};

