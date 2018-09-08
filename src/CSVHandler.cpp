#include "CSVHandler.h"
#include <fstream>
#include <iostream>
#include "StringHelper.h"

namespace IDEFIX {
	CSVHandler::CSVHandler() {

	}

	CSVHandler::~CSVHandler() {

	}

	/*!
	 * Set output path without filename
	 * 
	 * @param const std::string& path
	 */
	void CSVHandler::set_path(const std::string &path) {
		if ( m_path != path ) {
			m_path = path;
		}
	}

	/*!
	 * Set output filename without path
	 * 
	 * @param const std::string& name 
	 */
	void CSVHandler::set_filename(const std::string &name) {
		if ( m_filename != name ) {
			m_filename = name;

			// sanitize filename
			str::replace( m_filename, "/", "" );
		}
	}

	/*!
	 * Add line to file, if add_endl is not set it defaults to true
	 * 
	 * @param const std::string& line
	 * @param const bool         add_endl defaults to true
	 */
	void CSVHandler::add_line(const std::string &line, const bool add_endl) {
		if ( m_filename.empty() ) {
			return;
		}

		// concatenate path
		str::trailingslashit( m_path );

		// open file to append
		m_file.open( m_path + m_filename, std::ios::app | std::ios::out );

		// check if everything is ok
		if ( ! m_file.good() ) {
			return;
		}

		// write to file
		m_file << line;

		// add endl
		if ( add_endl ) {
			m_file << std::endl;
		}

		// close file
		m_file.close();
	}
};