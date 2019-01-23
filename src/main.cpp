#include <iostream>
#include <thread>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "core/logger.h"
#include "core/utility.h"
#include "core/mainapplication.h"

#ifdef CMAKE_PROJECT_VERSION
	#define PROJECT_VERSION CMAKE_PROJECT_VERSION
#else
	#define PROJECT_VERSION "unknown"
#endif

// shorthands for boost 
namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;
namespace bpt = boost::property_tree;

int main(int argc, char const *argv[])
{

	std::string config_file_path;

	try {

		bpo::options_description config("Configuration");
		config.add_options()
			("help,h", "produce help message")
			("config_file,c", bpo::value<std::string>( &config_file_path )->default_value( "idefix.conf" ), "idefix configuration file" )
		;

		bpo::variables_map vm;

		bpo::store( bpo::parse_command_line( argc, argv, config ), vm );
		bpo::notify( vm );

		if ( vm.count( "help" ) ) {
			std::cout << "Integrated Development Environment for Financial Information Exchange Version " << PROJECT_VERSION << std::endl;
			std::cout << config << std::endl;
			return EXIT_FAILURE;
		}

	} catch( bpo::error& e) {
		SPDLOG_ERROR("Bad options: {}", e.what() );
		return EXIT_FAILURE;
	}

	// variable declaration for further usage in MainApplication
	unsigned int webctx_port;
	idefix::NetworkAdapter* network_adapter;

	try {
		// parse idefix.conf file
		bpt::ptree prop_tree;
		bpt::json_parser::read_json( config_file_path, prop_tree );

		// database
		auto db_host = prop_tree.get<std::string>( "db.host", "" );
		auto db_user = prop_tree.get<std::string>( "db.user", "" );
		auto db_pwd  = prop_tree.get<std::string>( "db.pwd", "" );
		auto db_name = prop_tree.get<std::string>( "db.dbname", "" );
		
		// webcontext
		webctx_port = prop_tree.get<unsigned int>( "api.port", 0 );

		if ( ! webctx_port ) {
			SPDLOG_ERROR( "WebContext Port is not set." );
			return EXIT_FAILURE;
		}

		// adapter
		auto adapter_libfile = prop_tree.get<std::string>( "adapter.libfile", "" );
		auto adapter_configfile = prop_tree.get<std::string>( "adapter.configfile", "" );

		if ( adapter_libfile.empty() ) {
			SPDLOG_ERROR("No adapter lib file found.");
			return EXIT_FAILURE;
		}

		// prefix adapter folder path
		adapter_libfile = "./adapter/" + adapter_libfile;

		if ( adapter_configfile.empty() ) {
			SPDLOG_ERROR("No adapter config file found.");
			return EXIT_FAILURE;
		}

		SPDLOG_INFO( "Adapter: lib={} cfg={}", adapter_libfile, adapter_configfile );

		// try to load adapter
		auto adapter = idefix::Adapter::load( adapter_libfile );

		if ( ! adapter ) {
			SPDLOG_ERROR( "Could not load adapter!" );
			return EXIT_FAILURE;
		}

		// cast to network adapter
		network_adapter = dynamic_cast<idefix::NetworkAdapter*>( adapter ); 

		if ( ! network_adapter ) {
			SPDLOG_ERROR( "Could not load network adapter!" );
			return EXIT_FAILURE;
		}

		// set config file path
		network_adapter->setConfigFile( adapter_configfile );
		
	} catch( bpo::error& e ) {
		SPDLOG_ERROR("Options Error: {}", e.what() );
		return EXIT_FAILURE;
	} catch( bpt::json_parser::json_parser_error& json_error ) {
		SPDLOG_ERROR("Json Error: {}", json_error.what() );
		return EXIT_FAILURE;
	}
	

	// initiate WebContext
	idefix::WebContext webContext( webctx_port );

	// initiate system
	idefix::MainApplication mainApp;
	mainApp.setAdapter( network_adapter );
	mainApp.setWebContext( &webContext );

	mainApp.start();

	int i = 0;
	while(true) {
		if ( i > 9 ) break;
		i++;

		this_thread::sleep_for( chrono::seconds( 2 ) );
	}

	mainApp.stop();
	
	return 0;
}