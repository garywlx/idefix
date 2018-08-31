#include <iostream>
#include <cstring>
#include <cmath>
#include <iomanip>

using namespace std;

int main(int argc, char const *argv[])
{

	if ( argc < 4 ) {
		cout << "Usage: " << argv[0] << " <open> <close> <side(1long,0short)>" << endl;
		return EXIT_SUCCESS;
	}

	double open = atof( argv[ 1 ] );
	double close = atof( argv[ 2 ] );
	int side = atoi( argv[ 3 ] );
	double point_size = 0.0001;
	double qty = 100000;

	cout << "open   : " << setprecision(5) << fixed << open << endl;
	cout << "close  : " << setprecision(5) << fixed << close << endl;
	cout << "side   : " << side << " " << ( side == 0 ? "SHORT" : "LONG" ) << endl;
	cout << "point  : " << point_size << endl;
	cout << "qty    : " << qty << endl;


	// calculate profit/loss in points
	double pips_moved   = ( close - open ) / point_size;
	// calculate point profit
	double point_profit = qty * point_size;
	// calculate profit/loss
	double pnl = pips_moved * point_profit;

	cout << "pips   : " << pips_moved << endl;
	cout << "points : " << point_profit << endl;
	cout << "pnl    : " << pnl << endl;

	// wenn long
	// wenn close < open
	// looser
	
	// wenn short
	// wenn close > open
	// looser


	if ( ( side == 1 && close < open ) || ( side == 0 && close > open ) ) {
		pnl *= -1;
	}

	cout << "Profit : " << setprecision(5) << fixed << pnl << endl;
	
	return EXIT_SUCCESS;
}