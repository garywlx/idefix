#pragma once

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "core/logger.h"

namespace idefix {
	namespace config {

		typedef boost::property_tree::ptree ptree_t;

		/**
		 * Try to read a json file into a boost property_tree 
		 * 
		 * @param const std::string&           file_path	path to the json file
		 * @param idefix::config::ptree_t&     ptree 		reference to the ptree
		 */
		static bool read_json(const std::string& file_path, ptree_t& ptree) {
			// check file
			if ( file_path.empty() ) {
				SPDLOG_ERROR( "File path is empty." );
				return false;
			}

			if ( ! boost::filesystem::exists( file_path ) ) {
				SPDLOG_ERROR( "File {} does not exist.", file_path );
				return false;
			}

			try {
				// parse file
				boost::property_tree::json_parser::read_json( file_path, ptree );
			} catch( boost::property_tree::json_parser::json_parser_error& json_error ) {
				SPDLOG_ERROR( "Json Parsing Error: {}", json_error.what() );
				return false;
			} catch( std::exception &e ) {
				SPDLOG_ERROR( "std::exception: {}", e.what() );
				return false;
			} catch( ... ) {
				SPDLOG_ERROR( "Unknown error." );
				return false;
			}

			return true;
		}
	};
};