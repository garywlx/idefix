#include "fileadapter.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <math.h>
#include <chrono>
#include "../times.h"

namespace idefix {
	FileAdapter::FileAdapter(const std::string filename, Datacenter* datacenter_ptr) {	
		// assign datacenter ptr
		m_datacenter_ptr = datacenter_ptr;

		// call datacenter init
		datacenter()->on_init();

		std::string line;
		// load file
		std::ifstream file(filename, std::ios::binary);

		if( ! file.is_open() ){
			datacenter()->on_error("read file");
			std::exit(EXIT_FAILURE);
		}

		// the symbol
		std::string _symbol = datacenter()->symbol();		
		// skip first line
		std::getline(file, line);
		// read every line
		while( std::getline(file, line) ){
			// field vector
			std::vector<std::string> fields;

			// parse line
			std::istringstream sstream(line);
			std::string field;
			while( std::getline(sstream, field, ',') ){
				fields.push_back(field);
			}

			try {
				// convert to double
				double bid = std::stod(fields[1]);
				double ask = std::stod(fields[2]);
				double spread = abs(((ask - bid) * datacenter()->asset().contract_size) * 0.1);

				// make new tick
				auto tp = convert::to_time_point(fields[0]);
				// std::cout << tp.time_since_epoch() << std::endl;
				tick_struct tick{_symbol, fields[0], tp, bid, ask, spread, datacenter()->asset().decimal_places};
				// add new tick
				datacenter()->add_tick(tick);
			} catch(...){
				datacenter()->on_error("converting bid and ask prices");
			}
		}

		file.close();
		
		// Call exit callback
		datacenter()->on_exit();
	}
};