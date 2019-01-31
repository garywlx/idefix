#include <iostream>
#include <thread>
#include <fstream>
#include <exception>
#include <memory>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "core/logger.h"
#include "core/utility.h"
#include "core/mainapplication.h"
#include "core/configfile.h"
#include "core/datacontext.h"

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
	idefix::NetworkAdapter* network_adapter_rawptr;
	idefix::MainApplication mainApp;

	try {
		// parse idefix.conf file
		idefix::config::ptree_t prop_tree;
		if ( ! idefix::config::read_json( config_file_path, prop_tree ) ) {
			return EXIT_FAILURE;
		}

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
		network_adapter_rawptr = dynamic_cast<idefix::NetworkAdapter*>( adapter );

		if ( ! network_adapter_rawptr ) {
			SPDLOG_ERROR( "Could not load network adapter!" );
			return EXIT_FAILURE;
		}

		// set config file path
		network_adapter_rawptr->setConfigFile( adapter_configfile );

		// load algos
		for ( auto& algo : prop_tree.get_child( "algos" ) ) {
			auto algo_lib_file  = algo.second.get<std::string>("libfile");
			auto algo_conf_file = algo.second.get<std::string>("configfile");

			auto algo_module = idefix::Algo::load( "./algos/" + algo_lib_file );
			if ( ! algo_module ) {
				SPDLOG_ERROR( "Algo Module {} could not be loaded!", algo_lib_file );
				continue;
			}

			idefix::Algo* algo_cast = dynamic_cast<idefix::Algo*>( algo_module );

			if ( ! algo_cast ) {
				SPDLOG_ERROR( "Could not cast {} to Algo*!", algo_lib_file );
				continue;
			}

			// set config file
			algo_cast->setConfigFile( algo_conf_file );

			// add algo to main application
			mainApp.addAlgo( algo_cast );

			SPDLOG_INFO( "Added algo ./algos/{} with config file {}", algo_lib_file, algo_conf_file );
		}

	} catch( bpo::error& e ) {
		SPDLOG_ERROR( "Options Error: {}", e.what() );
		return EXIT_FAILURE;
	} catch( bpt::json_parser::json_parser_error& json_error ) {
		SPDLOG_ERROR( "Json Error: {}", json_error.what() );
		return EXIT_FAILURE;
	} catch( std::exception &e ) {
		SPDLOG_ERROR( "Error: {}", e.what() );
		return EXIT_FAILURE;
	}
	
	// Main
	try {
		// Initialize network adapter
		std::unique_ptr<idefix::NetworkAdapter> network_adapter( network_adapter_rawptr );
		// Initialize data context
		std::unique_ptr<idefix::DataContext> data_context( new idefix::DataContext( std::move( network_adapter ) ) );
		// Initialize web context
		std::unique_ptr<idefix::WebContext> web_context( new idefix::WebContext( webctx_port ) );
		
		// initiate system
		mainApp.setDataContext( std::move( data_context ) );
		mainApp.setWebContext( std::move( web_context ) );

		// start application
		mainApp.start();

		while( true ) {
			//if ( ! mainApp.isRunning() ) break;

			int cmd = 0;
			std::cin >> cmd;
			break;
		}

		mainApp.stop();	
	} catch( std::runtime_error& e ) {
		SPDLOG_ERROR( "Runtime Error: {}", e.what() );
		return EXIT_FAILURE;
	} catch( ... ) {
		SPDLOG_ERROR( "Unknown Exception." );
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}