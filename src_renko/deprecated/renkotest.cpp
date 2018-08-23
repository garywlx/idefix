#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../src/indicator/RenkoBrick.h"
#include "../src/indicator/SimpleMovingAverage.h"
#include "../src/Math.h"
#include "../src/Operators.h"
#include "../src/Console.h"
#include <quickfix/FieldConvertors.h>
#include "Strategy.h"
#include "RenkoStrategy.h"

using namespace std;
using namespace IDEFIX;
using namespace FIX;



// ---------------------------------------------------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	if ( argc < 2 ) {
		cerr << "Renko Strategy Test." << endl;
		cerr << "Usage: " << argv[0] << " <option(s)> renkobricks.csv" << endl;
		cerr << "Options:" << endl;
		cerr << "\t-p,--pyramid int    \t How many trades in one direction? Default is 0. That means only 1 Position at a time." << endl;
		cerr << "\t-v,--verbose        \t Show entries and exits when they happen." << endl;
		cerr << "\t-t,--trades         \t Show trade list" << endl;
		cerr << "\t-a,--account double \t Account start value. Default is 10.000 USD." << endl;
		cerr << "\t-r,--risk double    \t Risk per trade in percent of free margin. Default is 1%." << endl;
		cerr << "\t-q,--qty double     \t Quantity for new positions. Default is 100000." << endl;
		cerr << "\t-c,--chart          \t Show chart, desktop only." << endl;
		cerr << "\t-o,--out file       \t Write trades to file." << endl;

		return EXIT_FAILURE;
	}

	bool show_verbose    = false;
	bool show_trade_list = false;
	bool show_chart      = false;
	int pyramid          = 0;
	double account       = 10000;
	double risk          = 1;
	double qty           = 100000;
	std::string input_file;
	std::string output_file;

	for ( int i = 1; i < argc; i++ ) {
		std::string arg = argv[i];
		if ( ( arg == "-p" ) || ( arg == "--pyramid" ) ) {
			if ( i + 1 < argc ) {
				pyramid = IntConvertor::convert( argv[ i + 1 ] );
			} else {
				cerr << "-p,--pyramid option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-a" ) || ( arg == "--account" ) ) {
			if ( i + 1 < argc ) {
				account = DoubleConvertor::convert( argv[ i + 1 ] );
			} else {
				cerr << "-a,--account option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-r" ) || ( arg == "--risk" ) ) {
			if ( i + 1 < argc ) {
				risk = DoubleConvertor::convert( argv[ i + 1] );
			} else {
				cerr << "-r,--risk option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-q" ) || ( arg == "--qty" ) ) {
			if ( i + 1 < argc ) {
				qty = DoubleConvertor::convert( argv[ i + 1 ] );
			} else {
				cerr << "-q,--qty option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-o" ) || ( arg == "--out" ) ) {
			if ( i + 1 < argc ) {
				output_file = argv[ i + 1 ];
				str::trim( output_file );
				if ( output_file.empty() ) {
					cerr << "-o,--out option needs one argument." << endl;
					return EXIT_FAILURE;
				}
			} else {
				cerr << "-o,--out option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-v" ) || ( arg == "--verbose" ) ) {
			show_verbose = true;
		} else if ( ( arg == "-t" ) || ( arg == "--trades" ) ) {
			show_trade_list = true;
		} else if ( ( arg == "-c" ) || ( arg == "--chart" ) ) { 
			show_chart = true;
		} else {
			input_file = argv[i];
			str::trim( input_file );
			if ( input_file.empty() ) {
				cerr << "no input file found" << endl;
				return EXIT_FAILURE;
			}
		}
	}

	ifstream ifile( input_file );
	if ( ! ifile.good() ) {
		cerr << "File could not be opened." << endl;
		ifile.close();
		return EXIT_FAILURE;
	}

	std::vector<RenkoBrick> bricks;

	int line_i = 0;
	std::string line;
	while ( std::getline( ifile, line ) ) {
		std::stringstream line_stream( line );
		std::string cell;

		int cell_i = 0;

		// skip headlines
		if ( line_i == 0 ) {
			line_i++;
			continue;
		}

		RenkoBrick brick;

		// proceed in fields
		while ( std::getline( line_stream, cell, ',' ) ) {
			switch( cell_i ) {
				case 0: // status
					if ( cell == "-1" ) {
						brick.status = RenkoBrick::STATUS::NOSTATUS;
					} else if ( cell == "0" ) {
						brick.status = RenkoBrick::STATUS::LONG;
					} else if ( cell == "1" ) {
						brick.status = RenkoBrick::STATUS::SHORT;
					}
					break;
				case 1: // open time
					brick.open_time = cell;
					break;
				case 2: // open
					brick.open_price = DoubleConvertor::convert( cell );
					break;
				case 3: // high
					brick.high_price = DoubleConvertor::convert( cell );
					break;
				case 4: // low
					brick.low_price = DoubleConvertor::convert( cell );
					break;
				case 5: // close
					brick.close_price = DoubleConvertor::convert( cell );
					break;
				case 6: // close time
					brick.close_time = cell;
					break;
				case 7: // period
					brick.period = DoubleConvertor::convert( cell );
					break;
				case 8: // volume
					brick.volume = IntConvertor::convert( cell );
					break;
				case 9: // pointsize
					brick.point_size = DoubleConvertor::convert( cell );
					break;
			}

			cell_i++;
		}

		brick.diff = Math::get_spread( brick.open_price, brick.close_price, brick.point_size );

		bricks.push_back( brick );

		line_i++;
	}

	RenkoStrategy strategy( pyramid, show_verbose, output_file );
	strategy.begin_tradelog();

	int ticks = 0;
	for ( auto& b : bricks ) {
		strategy.onBrick( b );
		ticks += b.volume;
	}

	strategy.end_tradelog();

	if ( show_trade_list ) {
		cout << "Trades:" << endl;

		strategy.show_trade_list();	
		cout << "==============================================" << endl;
	}
	cout << "File:                  " << input_file << endl;
	cout << "Start Time:            " << bricks[0].open_time << endl;
	cout << "Stop Time:             " << bricks.back().close_time << endl;
	cout << "Period:                " << setprecision(0) << bricks[0].period << endl;
	cout << "Bricks:                " << setprecision(0) << bricks.size() << endl;
	cout << "Ticks:                 " << setprecision(0) << ticks << endl;
	cout << "Pyramid Trades:        " << setprecision(0) << pyramid << endl;
	cout << "==============================================" << endl;
	cout << "Total Long Positions:  " << setprecision(0) << strategy.total_long_positions() << endl;
	cout << "Total Short Positions: " << setprecision(0) << strategy.total_short_positions() << endl;
	cout << "Account begin:         " << setprecision(2) << fixed << account << " USD" << endl;
	cout << "Account end:           " << setprecision(2) << fixed << ( account + strategy.total_profit() ) << " USD" << endl;
	cout << "Total Profit:          " << setprecision(2) << fixed << strategy.total_profit() << " USD" << endl;
	cout << "----------------------------------------------" << endl;

	// --------------------------------------------------------------------------------------------------------------
	// Draw chart with qt
	// --------------------------------------------------------------------------------------------------------------
	if ( ! show_chart ) {
		return EXIT_SUCCESS;
	}

	// open chart with chart data
	system("if [ -f './html/chart.html' ]; then open html/chart.html; fi");

	return EXIT_SUCCESS;
}