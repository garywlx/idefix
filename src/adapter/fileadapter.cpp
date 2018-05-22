#include "fileadapter.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <math.h>

namespace idefix {
	FileAdapter::FileAdapter(const std::string filename, Datacenter* datacenter_ptr) {	
		// assign datacenter ptr
		m_datacenter_ptr = datacenter_ptr;

		// call onInit callback
		onInit();

		std::string line;
		// load file
		std::ifstream file(filename, std::ios::binary);

		if( ! file.is_open() ){
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
				tick_struct tick{_symbol, fields[0], bid, ask, spread, datacenter()->asset().decimal_places};
				// add new tick
				datacenter()->add_tick(tick);
				// callback
				onTick(tick);
			} catch(...){
				onError("converting bid and ask prices");
			}
		}

		file.close();
		
		// Call exit callback
		onExit();
	}

	void FileAdapter::onError(const std::string &error){
		std::cout << "[onError] " << error << std::endl;
	}

	void FileAdapter::onTick(const idefix::tick_struct &tick){
		std::cout << tick << std::endl;
	}

	void FileAdapter::onInit() {
		std::cout << "[onInit]" << std::endl;
	}

	void FileAdapter::onExit(){
		std::cout << "[onExit]" << std::endl;
	}
};