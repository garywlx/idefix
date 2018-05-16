#include "fileadapter.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace idefix {
	FileAdapter::FileAdapter(const std::string filename){
		// call onInit callback
		onInit();

		std::string line;
		// load file
		std::ifstream file(filename, std::ios::binary);

		if( ! file.is_open() ){
			std::exit(EXIT_FAILURE);
		}

		std::string _symbol = "EUR/USD";

		tick_map_t tick_map;
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
				// make new tick
				tick_struct tick{fields[0], bid, ask, 5};
				// callback
				onTick(tick);
				// add tick to symbol map
				if( hasSymbol(tick_map, _symbol) ){
					tick_map[_symbol].push_front(tick);
				} else {
					tick_deq_t ticks;
					ticks.push_front(tick);
					tick_map.insert(std::make_pair(_symbol, ticks));
				}
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