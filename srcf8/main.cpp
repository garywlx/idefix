#include <iostream>
// #include "idefix.h"
// #include "adapter/fileadapter.h"
// #include "assets/eurusd.h"
 
#include <fix8/f8includes.hpp>
#include "adapter/fix_client.h"

int main(int argc, char** argv){
	
	try {

		if ( argc != 2 ){
			throw std::invalid_argument(std::string("Usage: ") + argv[0] + " cfgfile");
		}

		const std::string client_conf_file(argv[1]);

		// FIX8 Client Session
		FIX8::ClientSession<fix_client>::ClientSession_ptr trading_client(new FIX8::ClientSession<fix_client>(FIX8::IDEFIX::ctx(), client_conf_file, "TRADING"));
    trading_client->start(true);



    FIX8::ClientSession<fix_client>::ClientSession_ptr market_client(new FIX8::ClientSession<fix_client>(FIX8::IDEFIX::ctx(), client_conf_file, "MARKET"));
    market_client->start(true);


	} catch(const FIX8::f8Exception& e){
	  std::cerr << e.what() << std::endl;
	  return EXIT_FAILURE;
	} catch(const std::exception& e){
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	// set asset for this session
	// idefix::assets::EurUsd asset;
	// // set datacenter with the asset
	// idefix::Datacenter datacenter(asset);
	// datacenter.set_lowest_bar_period(10);
	// // load price data
	// idefix::FileAdapter fa("../test/eurusd.csv", &datacenter);
	

	return EXIT_SUCCESS;
}