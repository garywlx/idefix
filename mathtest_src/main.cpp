
/**
 * Test App to Calculate Math methods
 */
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "../src/Math.h"
#include "../src/String.h"
#include "../src/MarketSnapshot.h"

using namespace std;
using namespace IDEFIX;

struct TestCase {
	MarketSnapshot symbol;
	MarketSnapshot conversion_symbol;
};

int main(int argc, char* argv[]) {

	double max_risk         = 1.5;
	double free_margin      = 5000;
	double pip_risk         = 200;
	string account_currency = "USD";

	if ( argc == 1 ) {
		cout << "mathtest - Test Math.h functions." << endl;
		cout << " Usage: " << argv[0] << " risk pip_risk free_margin (account_currency)" << endl;
		return EXIT_SUCCESS;
	}

	if ( argc == 4 ) {
		max_risk         = atof( argv[1] );
		pip_risk         = atof( argv[2] );
		free_margin      = atof( argv[3] );
	} else if ( argc > 4 ) {
		max_risk         = atof( argv[1] );
		pip_risk         = atof( argv[2] );
		free_margin      = atof( argv[3] );
		if ( argc == 5 ) {
			account_currency = argv[4];	
		}
	}

	std::vector<TestCase> cases;

	// Conversion to EUR
	MarketSnapshot eurcad_snapshot;
	eurcad_snapshot.setSymbol( "EUR/CAD" );
	eurcad_snapshot.setPrecision( 5 );
	eurcad_snapshot.setPointSize( 0.0001 );
	eurcad_snapshot.setAsk( 1.52894 );

	MarketSnapshot eurchf_snapshot;
	eurchf_snapshot.setSymbol( "EUR/CHF" );
	eurchf_snapshot.setPrecision( 5 );
	eurchf_snapshot.setPointSize( 0.0001 );
	eurchf_snapshot.setAsk( 1.16298 );

	MarketSnapshot eurjpy_snapshot;
	eurjpy_snapshot.setSymbol( "EUR/JPY" );
	eurjpy_snapshot.setPrecision( 5 );
	eurjpy_snapshot.setPointSize( 0.01 );
	eurjpy_snapshot.setAsk( 129.781 );

	// TestCases
	MarketSnapshot eurusd_snapshot;
	eurusd_snapshot.setSymbol( "EUR/USD" );
	eurusd_snapshot.setBid( 1.4545 );
	eurusd_snapshot.setAsk( 1.4545 );
	eurusd_snapshot.setPointSize( 0.0001 );
	eurusd_snapshot.setPrecision( 5 );

	MarketSnapshot audusd_snapshot;
	audusd_snapshot.setSymbol( "AUD/USD" );
	audusd_snapshot.setBid( 0.74326 );
	audusd_snapshot.setAsk( 0.74326 );
	audusd_snapshot.setPointSize( 0.0001 );
	audusd_snapshot.setPrecision( 5 );
	TestCase audusd = { audusd_snapshot, eurusd_snapshot };
	cases.push_back( audusd );

	TestCase eurusd = { eurusd_snapshot, eurusd_snapshot };
	cases.push_back( eurusd );

	MarketSnapshot gbpusd_snapshot;
	gbpusd_snapshot.setSymbol( "GBP/USD" );
	gbpusd_snapshot.setBid( 1.31835 );
	gbpusd_snapshot.setAsk( 1.31835 );
	gbpusd_snapshot.setPointSize( 0.0001 );
	gbpusd_snapshot.setPrecision( 5 );
	TestCase gbpusd = { gbpusd_snapshot, eurusd_snapshot };
	cases.push_back( gbpusd );

	MarketSnapshot nzdusd_snapshot;
	nzdusd_snapshot.setSymbol( "NZD/USD" );
	nzdusd_snapshot.setBid( 0.68276 );
	nzdusd_snapshot.setAsk( 0.68276 );
	nzdusd_snapshot.setPointSize( 0.0001 );
	nzdusd_snapshot.setPrecision( 5 );
	TestCase nzdusd = { nzdusd_snapshot, eurusd_snapshot };
	cases.push_back( nzdusd );

	MarketSnapshot usdcad_snapshot;
	usdcad_snapshot.setSymbol( "USD/CAD" );
	usdcad_snapshot.setBid( 1.30457 );
	usdcad_snapshot.setAsk( 1.30457 );
	usdcad_snapshot.setPointSize( 0.0001 );
	usdcad_snapshot.setPrecision( 5 );
	TestCase usdcad = { usdcad_snapshot, eurcad_snapshot };
	cases.push_back( usdcad );

	MarketSnapshot usdchf_snapshot;
	usdchf_snapshot.setSymbol( "USD/CHF" );
	usdchf_snapshot.setBid( 0.99229 );
	usdchf_snapshot.setAsk( 0.99229 );
	usdchf_snapshot.setPointSize( 0.0001 );
	usdchf_snapshot.setPrecision( 5 );
	TestCase usdchf = { usdchf_snapshot, eurchf_snapshot };
	cases.push_back( usdchf );

	MarketSnapshot usdjpy_snapshot;
	usdjpy_snapshot.setSymbol( "USD/JPY" );
	usdjpy_snapshot.setBid( 110.733 );
	usdjpy_snapshot.setAsk( 110.733 );
	usdjpy_snapshot.setPointSize( 0.01 );
	usdjpy_snapshot.setPrecision( 3 );
	TestCase usdjpy = { usdjpy_snapshot, eurjpy_snapshot };
	cases.push_back( usdjpy );
	// TestCases EOL

	const double contract_size = 100000;

	cout << "TEST CASES" << endl;
	cout << "================================================================" << endl;
	cout << " max_risk         " << setprecision(2) << fixed << max_risk << " %" << endl;
	cout << " pip_risk         " << setprecision(0) << fixed << pip_risk << " pips" << endl;
	cout << " free_margin      " << setprecision(2) << fixed << free_margin << " " << account_currency << endl;
	cout << " contractsize     " << setprecision(2) << fixed << contract_size << " units" << endl;
	cout << endl;

	for ( auto it = cases.begin(); it != cases.end(); ++it ) {

		TestCase c = *it;

		const double point_size       = c.symbol.getPointSize();
		const double conversion_price = Math::get_conversion_price( c.conversion_symbol.getAsk(), account_currency, c.conversion_symbol );		
		const double units            = Math::get_unit_size( free_margin, max_risk, pip_risk, conversion_price, point_size, contract_size );
		const double std_lot          = units / contract_size;
		const double mini_lot         = units / 10000;
		const double mikro_lot        = units / 1000;
		const double amount_at_risk   = Math::get_amount_at_risk( free_margin, max_risk );
		const int precision           = c.conversion_symbol.getPrecision();

		cout << "[" << c.symbol.getSymbol() << "]" << endl;
		cout << " point_size       " << setprecision( precision ) << fixed << point_size << endl;
		cout << " ask              " << setprecision( precision ) << fixed << c.conversion_symbol.getAsk() << endl;
		cout << " conversion_price " << setprecision( precision ) << fixed << conversion_price << " " << c.conversion_symbol.getSymbol() << endl;	
		cout << " units            " << setprecision(2) << fixed << units << endl;
		cout << " std lot          " << setprecision(4) << fixed << std_lot << endl;
		cout << " mini lot         " << setprecision(4) << fixed << mini_lot << endl;
		cout << " mikro lot        " << setprecision(4) << fixed << mikro_lot << endl;
		cout << " amount at risk   " << setprecision(2) << fixed << amount_at_risk << " " << account_currency << endl;
		cout << endl;
	}

	return EXIT_SUCCESS;
}