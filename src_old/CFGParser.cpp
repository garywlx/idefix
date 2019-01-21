#include "CFGParser.h"
#include "StringHelper.h"
#include <fstream>
#include <vector>
#include <algorithm>

namespace IDEFIX {
	CFGParser::CFGParser(const std::string cfg_file) throw ( IDEFIX::file_not_found, out_of_range ) {
		m_has_error = false;

		// check if file exists
		std::ifstream file( cfg_file );
		if ( ! file.good() ){
			m_has_error = true;
			throw file_not_found(__FILE__, __LINE__);
		}

		// parse file
		std::string line_buffer;
		while ( std::getline(file, line_buffer) ) {

			// check for empty line
			if ( line_buffer.size() == 0 ) {
				continue;
			}

			// check for comment
			if ( line_buffer.at( 0 ) == '#' ) {
				continue;
			}

			// check for equal sign
			if ( std::count( line_buffer.begin(), line_buffer.end(), '=' ) == 0 ) {
				continue;
			}

			// parse entry
			std::vector<std::string> parts = str::explode( line_buffer, '=' );
			if ( parts.size() < 2 ) {
				continue;
			}

			// add pair to map
			m_values.insert( std::pair<std::string, std::string>( parts[0], parts[1] ) );
		}

		file.close();
	}

	/*!
	 * Return bool of having an error
	 * 
	 * @return bool
	 */
	bool CFGParser::has_error() {
		return m_has_error;
	}

	/*!
	 * Get values as map[key] = value
	 * 
	 * @return std::map<std::string, std::string>
	 */
	std::map<std::string, std::string> CFGParser::values() {
		return m_values;
	}

	/*!
	 * Get value for key
	 * 
	 * @param const std::string&  key 
	 * @return std::string
	 */
	std::string CFGParser::value(const std::string& key) {
		std::string value;
		auto it = m_values.find( key );
		if ( it != m_values.end() ) {
			value = it->second;
		}
		return value;
	}
};