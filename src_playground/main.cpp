#include <iostream>
#include <vector>
#include <iterator>
#include "../src/Math.h"

using namespace std;
using namespace IDEFIX;

int main(int argc, char const *argv[])
{

	if ( argc < 3 ) {
		cout << "Usage: " << argv[0] << " <period> <offset>" << endl;
		return EXIT_SUCCESS;
	}

	/**
	 * Test Moving Average Calculations
	 */
	std::vector<double> v;
	
	// dump new list
	for( auto& entry : v ) {
		cout << entry << ",";
	}
	cout << endl;

	int period = atoi( argv[1] );
	int offset = atoi( argv[2] );

	cout << "Period " << period << endl;
	cout << "Offset " << offset << endl;
	cout << "Size   " << v.size() << endl;

	double in;
	while( true ) {
		cout << "Add positive value to list (0=exit): ";
		cin >> in;

		if ( in > 0 ) {
			v.push_back( in );

			// dump new list
			for( auto& entry : v ) {
				cout << entry << ",";
			}
			cout << endl;

			cout << "MA " << Math::get_moving_average( v, period, offset ) << endl;
		} else {
			break;
		}
	}

	// cout << "Elements ";
	// for ( auto& val : v ) {
	// 	cout << val << ",";
	// }
	// cout << endl;

	// double sum = 0;
	// auto it = std::next( v.rbegin(), offset );

	// for ( int i = 0; i < period; i++ ) {
	// 	auto vi = std::next( it, i );
	// 	sum += *vi;

	// 	cout << *vi << ( i + 1 < period ? " + ": "" );
	// }

	// double ma = sum / period;

	// cout << " = " << sum;
	// cout << " / " << period << " = " << ma << endl;
		
	return EXIT_SUCCESS;
}