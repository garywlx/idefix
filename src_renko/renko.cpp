/*!
 * Convert FXCM Tickdata to renko bars and test strategy 
 * against these data. Create a report with chart. 
 * 
 * @dependencies: idefix, quickfix
 */

#define MAJOR 1
#define MINOR 7

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#include <quickfix/FieldConvertors.h>

#include "../src/indicator/Tick.h"
#include "../src/indicator/Renko.h"
#include "../src/indicator/RenkoBrick.h"
#include "../src/indicator/SimpleMovingAverage.h"
#include "../src/String.h"

#include "RenkoStrategy.h"

using namespace std;
using namespace IDEFIX;
using namespace FIX;

/*!
 * Check argument with option if option value exists.
 * If not show cerr message
 * 
 * @param const int         argc        
 * @param const int         i           
 * @param const std::string arg
 * @param const std::string failure_msg 
 * @return bool
 */
inline bool check_argument_option(const int argc, const int i, const std::string arg, const std::string failure_msg = " option requires one argument.") {
	if ( i + 1 < argc ) {
		return true;
	} 

	cerr << arg << failure_msg << endl;
	return false;
}

int main(int argc, char const *argv[])
{
	if ( argc < 3 ) {
		// show usage information
		cerr << "Convert FXCM Tickdata to renko bar data and test strategy. Creates a report with chart." << endl;
		cerr << "Usage: " << argv[0] << " <option(s)> tickdata.csv" << endl;
		cerr << "Options:" << endl;
		cerr << "\t-o file            \t Output file for renko bricks, defaults to output.csv." << endl;
		cerr << "\t-p value           \t Renko period in pips, defaults to 10." << endl;
		cerr << "\t-ps value          \t Symbol point size, defaults to 0.0001." << endl;
		cerr << "\t-f value           \t The output format, either FXCM or anychart, defaults to FXCM." << endl;
		cerr << "\t-v                 \t Verbose mode. Show everything while processing." << endl;
		cerr << "\t-s value           \t Use strategy, defaults to renko." << endl;
		cerr << "\t-c                 \t Generate chart.html and data.js." << endl;

		return EXIT_FAILURE;
	}

	// Defaults
	string output_file   = "output.csv";
	string input_file;
	string output_format = "FXCM";
	double renko_period  = 10;
	double point_size    = 0.0001;
	double account_size  = 10000;
	bool verbose_mode    = false;
	bool generate_chart  = false;
	bool use_strategy    = false;
	string strategy      = "renko";

	// parse commandline arguments
	for ( int i = 0; i < argc; i++ ) {
		string arg = argv[ i ];

		// output file
		if ( arg == "-o" ) {
			if ( ! check_argument_option( argc, i, arg ) ) {
				return EXIT_FAILURE;
			}

			output_file = argv[ i + 1 ];
			str::trim( output_file );
		} 
		// renko period
		else if ( arg == "-p" ) {
			if ( ! check_argument_option( argc, i, arg ) ) {
				return EXIT_FAILURE;
			}

			renko_period = DoubleConvertor::convert( argv[ i + 1 ] );
		}
		// point size
		else if ( arg == "-ps" ) {
			if ( ! check_argument_option( argc, i, arg ) ) {
				return EXIT_FAILURE;
			}

			point_size = DoubleConvertor::convert( argv[ i + 1 ] );
		}
		// format
		else if ( arg == "-f" ) {
			if ( ! check_argument_option( argc, i, arg ) ) {
				return EXIT_FAILURE;
			}

			output_format = argv[ i + 1 ];
		} 
		// use strategy
		else if ( arg == "-s" ) {
			if ( ! check_argument_option( argc, i, arg ) ) {
				return EXIT_FAILURE;
			}

			use_strategy = true;
			strategy = argv[ i + 1 ];
		}
		// generate chart
		else if ( arg == "-c" ) {
			generate_chart = true;
			output_format = "anychart";
		}
		// verbose mode
		else if ( arg == "-v" ) {
			verbose_mode = true;
		} 
		// input file
		else {
			input_file = argv[ i ];
		}
	}

	// Error handling
	str::trim( input_file );
	if ( input_file.empty() ) {
		cerr << "No input file found." << endl;
		return EXIT_FAILURE;
	}

	// check if file exists
	// 
	std::ifstream ifile( input_file );
	if ( ! ifile.good() ) {
		cout << "File does not exit." << endl;
		return EXIT_FAILURE;
	}

	// convert data
	Renko renko( renko_period, verbose_mode );

	// parse tick data to renko
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

		Tick tick = {"",0,0,point_size};

		// proceed in fields
		while ( std::getline( line_stream, cell, ',' ) ) {
			if ( cell_i == 0 ) {
				tick.datetime = cell;
			} else if ( cell_i == 1 ) {
				tick.bid = DoubleConvertor::convert( cell );
			} else if ( cell_i == 2 ) {
				tick.ask = DoubleConvertor::convert( cell );
			}

			cell_i++;
		}

		if ( tick.datetime != "" && tick.bid > 0 && tick.ask > 0 ) {
			// add tick to pool
			renko.add_tick( tick );
		} else {
			cout << "no valid tick" << endl;
			cout << tick << endl;
			continue;
		}

		line_i++;
	}

	ifile.close();

	// overwrite output_file with data.js for chart
	if ( generate_chart ) {
		output_file = "data.js";
	}

	// write output
	// overwrite if file exists
	std::ofstream ofile( output_file, std::ofstream::trunc );
	if ( ! ofile.good() ) {
		cout << "Output file error." << endl;
		ofile.close();
		ifile.close();
		return EXIT_FAILURE;
	}

	auto bricks = renko.brick_list();

	int file_line = 0;
	int brick_count = bricks.size();
	SimpleMovingAverage sma5( 5 );
	SimpleMovingAverage sma10( 10 );

	// FXCM
	if ( output_format == "FXCM" || output_format == "fxcm" ) {
		ofile << "# Status,OpenTime,Open,High,Low,Close,CloseTime,Period,Volume,PointSize" << endl;			
	} else if ( output_format == "anychart" || generate_chart ) {
		ofile << "var seriesfile = [";
	}

	// use strategy?
	RenkoStrategy strategy_handler(account_size, 1, verbose_mode);

	for ( auto& b : renko.brick_list() ) {

		if ( b.open_time == "" || b.open_price == 0 || b.close_time == "" || b.close_price == 0 ) {
			cout << "error brick " << b << endl;
			continue;
		}

		// call strategy on brick callback
		strategy_handler.on_brick( b );

		// Add SimpleMovingAverage value
		sma5.add( ( b.open_price + b.high_price + b.low_price + b.close_price ) / 4 );
		// sma10.add( ( b.open_price + b.high_price + b.low_price + b.close_price ) / 4 );

		if ( output_format == "anychart" || generate_chart ) {
			// ANYCHART
			std::vector<std::string> ct = str::explode( b.close_time, ' ' );
			std::vector<std::string> ctt = str::explode( ct[0], '/' );
			std::stringstream close_time;
			close_time << ctt[2] << "-" << ctt[0] << "-" << ctt[1] << " " << ct[1];

			std::stringstream sline;

			sline << "['" << close_time.str() << "',";
			sline << b.open_price << ",";
			sline << b.high_price << ",";
			sline << b.low_price << ",";
			sline << b.close_price << ",";
			sline << b.volume << ",";
			sline << sma5 << ",";
			sline << sma10 << "]";

			if ( file_line + 1 < brick_count ) {
				sline << ",";
				sline << endl;
			}

			ofile << sline.str();

		} else if ( output_format == "FXCM" || output_format == "fxcm" ) {
			// FXCM
			// convert timestamps from 08/05/2018 21:03:56.102 to yyyymmdd-H:i:s.u
			std::vector<std::string> ot = str::explode( b.open_time, ' ' );
			std::vector<std::string> ott = str::explode( ot[0], '/' );
			std::stringstream open_time;
			open_time << ott[2] << "-" << ott[0] << "-" << ott[1] << " " << ot[1];

			std::vector<std::string> ct = str::explode( b.close_time, ' ' );
			std::vector<std::string> ctt = str::explode( ct[0], '/' );
			std::stringstream close_time;
			close_time << ctt[2] << "-" << ctt[0] << "-" << ctt[1] << " " << ct[1];

			ofile << b.status << ",";
			ofile << open_time.str() << ",";
			ofile << b.open_price << ",";
			ofile << b.high_price << ",";
			ofile << b.low_price << ",";
			ofile << b.close_price << ",";
			ofile << close_time.str() << ",";
			ofile << b.diff << ",";
			ofile << b.volume << ",";
			ofile << b.point_size << endl;
		}

		file_line++;
	}

	if ( output_format == "anychart" || generate_chart ) {
		ofile << "];";

		if ( use_strategy ) {
			std::stringstream trade_log;
			trade_log << "var tradeseries = [";
			trade_log << strategy_handler.tradelog();
			trade_log << "];" << endl;

			ofile << endl << trade_log.str();
		}
	}

	ofile.close();

	cout << "Input file:    " << input_file << endl;
	cout << "Output file:   " << output_file << endl;
	cout << "Output format: " << output_format << endl;
	cout << "Renko Period:  " << setprecision(2) << fixed << renko_period << endl;
	cout << "Point Size:    " << setprecision(5) << fixed << point_size << endl;
	cout << "Ticks:         " << renko.tick_list().size() << endl;
	cout << "Bricks:        " << brick_count << endl;

	if ( use_strategy ) {
		cout << "========= Strategy =========" << endl;
		strategy_handler.stats();
	}

	// if generate chart, copy html and js files to output dir
	if ( generate_chart ) {
		cout << "Create chart in ./chart ...";
		system("mkdir -p chart");
		system("mv data.js chart/");
		system("cp html/* chart/");
		system("open chart/chart.html");
		cout << "done." << endl;
	} 

	return EXIT_SUCCESS;
}