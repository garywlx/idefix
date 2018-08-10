#include <iostream>
#include <vector>
#include <iterator>

using namespace std;

int main(int argc, char const *argv[])
{

	/**
	 * Test Moving Average Calculations
	 */

	std::vector<int> v {5,6,7,8,9};
	
	int period = 3;
	for( int begin = 0; begin < 3; begin++ ) {

		double sum = 0;
		auto it = std::next( v.rbegin(), begin );

		for ( int i = 0; i < period; i++ ) {
			auto vi = std::next( it, i );
			sum += *vi;

			cout << *vi << ( i + 1 < period ? " + ": "" );
		}

		cout << " = " << sum << endl;

		double ma = sum / period;

		std::cout << "MovingAverage: " << ma << std::endl;
	}
		
	return EXIT_SUCCESS;
}