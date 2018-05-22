#include <iostream>
#include "idefix.h"
#include "adapter/fileadapter.h"
#include "assets/eurusd.h"

int main(int argc, char** argv){
	
	// set asset for this session
	idefix::assets::EurUsd asset;
	// set datacenter with the asset
	idefix::Datacenter datacenter(asset);
	// load price data
	idefix::FileAdapter fa("../test/eurusd.csv", &datacenter);
	
	return 0;
}