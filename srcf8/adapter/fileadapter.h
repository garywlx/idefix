#ifndef IDEFIX_FILEADAPTER_H
#define IDEFIX_FILEADAPTER_H

#include <iostream>
#include "brokeradapter.h"

namespace idefix {
	class FileAdapter: public BrokerAdapter {
	public:
		FileAdapter(const std::string filename, Datacenter* datacenter);
	};
}

#endif