/*!
 * Author: Arne Gockeln, www.arnegockeln.com
 * Version: 1.0
 * Description: Requires input file from https://github.com/fxcm/FXCMTickData
 */

#define MAJOR 1
#define MINOR 0

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <quickfix/FieldConvertors.h>
#include "../src/Math.h"
#include "../src/indicator/RenkoBrick.h"

using namespace std;
using namespace FIX;
using namespace IDEFIX;

struct Tick {
	std::string datetime;
	double bid;
	double ask;
};

inline ostream& operator<<(ostream& out, const Tick& tick) {
	out << "Tick " << tick.datetime << " bid " << setprecision(5) << fixed << tick.bid << " ask " << setprecision(5) << fixed << tick.ask;
	return out;
};

int main(int argc, char const *argv[])
{
	if ( argc < 3 ) {
		cout << "The program converts fxcm tickdata.csv files to renko files. Version " << MAJOR << "." << MINOR << endl;	
		cout << "Usage: " << argv[0] << " <option(s)> tickdata.csv" << endl;
		cout << "Options:" << endl;
		cout << "\t-o,--out file       \t Output file for renko bricks, defaults to output.txt" << endl;
		cout << "\t-p,--period double  \t Renko period in pips, default is 10." << endl;
		cout << "\t-s,--psize  double  \t Point size, defaults to 0.0001." << endl;
		cout << "\t-v,--verbose        \t Show renko brick color output, defaults to false." << endl;
		return EXIT_SUCCESS;
	}

	std::string input_file;
	std::string output_file = "output.txt";
	double renko_period     = 10;
	double point_size       = 0.0001;
	bool show_bricks        = false;
	// parse commandline arguments
	for ( int i = 1; i < argc; ++i ) {
		std::string arg = argv[ i ];
		if ( ( arg == "-o" ) || ( arg == "--out") ) {
			if ( i + 1 < argc ) {
				output_file = argv[ i+1 ];
			} else {
				cerr << "-o,--out option requires one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-p" ) || ( arg == "--period" ) ) {
			if ( i + 1 < argc ) {
				double tmp = DoubleConvertor::convert( argv[ i+1 ] );
				if ( tmp > 0 ) {
					renko_period = tmp;
				} else {
					cerr << "-p,--period value needs to be greater than 0." << endl;
					return EXIT_FAILURE;
				}
			} else {
				cerr << "-p,--period option requires one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-s" ) || ( arg == "--psize" ) ) {
			if ( i + 1 < argc ) {
				double tmp = DoubleConvertor::convert( argv[ i+1 ] );
				if ( tmp > 0 ) {
					point_size = tmp;
				} else {
					cerr << "-s,--psize value needs to be greater than 0." << endl;
					return EXIT_FAILURE;
				}
			} else {
				cerr << "-s,--psize option requires one argument." << endl;
			}
		} else if ( ( arg == "-v" ) || ( arg == "--verbose" ) ) {
			show_bricks = true;
		} else {
			input_file = argv[ i ];
		}
	}

	// check if file exists
	// 
	std::ifstream ifile( input_file );
	if ( ! ifile.good() ) {
		cout << "File does not exit." << endl;
		return EXIT_FAILURE;
	}

	// renko output
	std::ofstream ofile( output_file );
	if ( ! ofile.good() ) {
		cout << "Output file error." << endl;
		ofile.close();
		ifile.close();
		return EXIT_FAILURE;
	}

	std::vector<Tick> ticks;
	std::vector<RenkoBrick> bricks;

	RenkoBrick current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};

	// parse tick data to renko
	// 
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

		Tick tick;

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

		// add tick to pool
		ticks.push_back( tick );

		// renko calculation
		if ( current_brick.status == RenkoBrick::STATUS::NOSTATUS && current_brick.volume == 0 ) {
			current_brick.open_time  = tick.datetime;
			current_brick.open_price = tick.bid;
			current_brick.low_price  = tick.bid;			
			current_brick.volume     = 1;
			current_brick.point_size = point_size;
		} else {
			// increase volume
			current_brick.volume++;
		}

		// check if distance between open_price and current tick price is 3 pips
		const double pip_distance = Math::get_spread( tick.bid, current_brick.open_price, point_size );

		// calculate high and low of renko brick
		current_brick.high_price = std::max( current_brick.high_price, tick.bid );
		current_brick.low_price  = std::min( current_brick.low_price, tick.bid );

		if ( pip_distance >= renko_period ) {
			current_brick.diff        = pip_distance;
			current_brick.close_price = tick.bid;
			current_brick.close_time  = tick.datetime;
			current_brick.status      = current_brick.open_price < current_brick.close_price ? RenkoBrick::STATUS::LONG : RenkoBrick::STATUS::SHORT;

			bricks.push_back( current_brick );

			if ( show_bricks ) {
				cout << current_brick << endl;	
			}
			
			// reset current brick
			current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
		} 

		line_i++;
	}

	// write output
	ofile << "Status,OpenTime,Open,High,Low,Close,CloseTime,Period,Volume,PointSize" << endl;
	for( auto& b : bricks ) {
		ofile << b.status << ",";
		ofile << b.open_time << ",";
		ofile << b.open_price << ",";
		ofile << b.high_price << ",";
		ofile << b.low_price << ",";
		ofile << b.close_price << ",";
		ofile << b.close_time << ",";
		ofile << b.diff << ",";
		ofile << b.volume << ",";
		ofile << b.point_size << endl;
	}
	ofile.close();
	
	cout << "Input file :  " << input_file << endl;
	cout << "Output file:  " << output_file << endl;
	cout << "Renko Period: " << setprecision(2) << fixed << renko_period << endl;
	cout << "Point Size:   " << setprecision(5) << fixed << point_size << endl;
	cout << "Ticks:        " << ticks.size() << endl;
	cout << "Bricks:       " << bricks.size() << endl;

	return EXIT_SUCCESS;
}