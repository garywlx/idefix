#include <iostream>
#include "idefix.h"
#include "adapter/fileadapter.h"

int main(int argc, char** argv){
	
	idefix::FileAdapter fa("../test/test.csv");

	return 0;
}