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
#include "../src/String.h"

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
		cout << "\t-ps,--psize double  \t Point size, defaults to 0.0001." << endl;
		cout << "\t-v,--verbose        \t Show renko brick color output, defaults to false." << endl;
		cout << "\t-s,--symbol name    \t The symbol name like EUR/USD." << endl;
		return EXIT_SUCCESS;
	}

	std::string input_file;
	std::string output_file = "output.txt";
	std::string format      = "FXCM";
	std::string symbol;
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
		} else if ( ( arg == "-ps" ) || ( arg == "--psize" ) ) {
			if ( i + 1 < argc ) {
				double tmp = DoubleConvertor::convert( argv[ i+1 ] );
				if ( tmp > 0 ) {
					point_size = tmp;
				} else {
					cerr << "-ps,--psize value needs to be greater than 0." << endl;
					return EXIT_FAILURE;
				}
			} else {
				cerr << "-ps,--psize option requires one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-s" ) || ( arg == "--symbol" ) ) {
			if ( i + 1 < argc ) {
				symbol = argv[ i + 1 ];
				str::trim( symbol );
				if ( format.empty() ) {
					cerr << "-s,--symbol value needs to be a string." << endl;
					return EXIT_FAILURE;
				}
			} else {
				cerr << "-s,--symbol option requires on argument." << endl;
				return EXIT_SUCCESS;
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
	RenkoBrick last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
	// for the very first brick to build
	RenkoBrick init_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};

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

		if ( tick.datetime != "" && tick.bid > 0 && tick.ask > 0 ) {
			// add tick to pool
			ticks.push_back( tick );	
		} else {
			cout << "no valid tick" << endl;
			cout << tick << endl;
			continue;
		}

		// get previous brick
		if ( bricks.size() == 0 ) {		
			// init first brick
			if ( init_brick.status == RenkoBrick::STATUS::NOSTATUS && init_brick.volume == 0 ) {
				init_brick.open_time  = tick.datetime;
				init_brick.open_price = tick.bid;
				init_brick.low_price  = tick.bid;
				init_brick.volume     = 1;
				init_brick.point_size = point_size;	
			} else {
				init_brick.volume++;
			}

			// make initial brick
			// add LONG brick
			if ( init_brick.volume > 1 && tick.bid > init_brick.open_price && Math::get_spread( tick.bid, init_brick.open_price, point_size ) >= renko_period ) {
				init_brick.diff        = Math::get_spread( tick.bid, init_brick.open_price, point_size );
				init_brick.status      = RenkoBrick::STATUS::LONG;
				init_brick.close_price = tick.bid;
				init_brick.close_time  = tick.datetime;

				if ( init_brick.diff > renko_period ) {
					init_brick.diff        = renko_period;
					init_brick.close_price = init_brick.open_price + ( renko_period * point_size );
				}

				init_brick.high_price  = init_brick.close_price;

				if ( show_bricks ) {
					cout << "0+LONG  " << init_brick << endl;	
				}

				bricks.push_back( init_brick );
				last_brick = init_brick;
				init_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
				line_i++;
				continue;
			}
			// add short brick
			else if ( init_brick.volume > 1 && tick.bid < init_brick.open_price && Math::get_spread( tick.bid, init_brick.open_price, point_size ) >= renko_period ) {
				init_brick.diff        = Math::get_spread( tick.bid, init_brick.open_price, point_size );
				init_brick.status      = RenkoBrick::STATUS::SHORT;
				init_brick.close_price = tick.bid;
				init_brick.close_time  = tick.datetime;

				if ( init_brick.diff > renko_period ) {
					init_brick.diff        = renko_period;
					init_brick.close_price = init_brick.open_price - ( renko_period * point_size );
				}

				init_brick.high_price  = init_brick.open_price;
				init_brick.low_price   = init_brick.close_price;

				if ( show_bricks ) {
					cout << "0+SHORT " << init_brick << endl;	
				}
				
				bricks.push_back( init_brick );
				last_brick = init_brick;
				init_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
				line_i++;
				continue;
			}
		} else {
			last_brick = bricks.back();
		}

		// renko calculation
		if ( current_brick.status == RenkoBrick::STATUS::NOSTATUS && current_brick.volume == 0 ) {
			current_brick.open_time  = last_brick.close_time;
			current_brick.open_price = last_brick.close_price;
			current_brick.low_price  = last_brick.close_price;
			current_brick.volume     = 1;
			current_brick.point_size = point_size;
		} 

		// increase volume
		current_brick.volume++;

		// make brick
		switch( last_brick.status ) {
			// last brick is LONG 
			case RenkoBrick::STATUS::LONG:
				// open new long brick
				if ( tick.bid > last_brick.close_price && Math::get_spread( tick.bid, last_brick.close_price, point_size ) >= renko_period ) {
					current_brick.open_time   = last_brick.close_time;
					current_brick.open_price  = last_brick.close_price;
					current_brick.diff        = Math::get_spread( tick.bid, last_brick.close_price, point_size );
					current_brick.close_price = tick.bid;
					current_brick.close_time  = tick.datetime;
					current_brick.status      = RenkoBrick::STATUS::LONG;

					// correct price and diff if it is != renko_period
					if ( current_brick.diff > renko_period ) {
						current_brick.close_price = last_brick.close_price + ( renko_period * point_size );
						current_brick.diff        = renko_period;
					}

					current_brick.low_price  = current_brick.open_price;
					current_brick.high_price = current_brick.close_price;

					if ( show_bricks ) {
						cout << "L+LONG  " << current_brick << endl;	
					}
					
					// add brick to stack
					bricks.push_back( current_brick );
					//last_brick = current_brick;
					// reset current and last brick
					current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
					last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
				}
				// open new short brick
				else if ( tick.bid < last_brick.open_price && Math::get_spread( tick.bid, last_brick.open_price, point_size ) >= renko_period ) {
					current_brick.open_time   = last_brick.open_time;
					current_brick.open_price  = last_brick.open_price;
					current_brick.diff        = Math::get_spread( tick.bid, last_brick.open_price, point_size );
					current_brick.close_price = tick.bid;
					current_brick.close_time  = tick.datetime;
					current_brick.status      = RenkoBrick::STATUS::SHORT;

					if ( current_brick.diff > renko_period ) {
						current_brick.close_price = last_brick.open_price - ( renko_period * point_size );
						current_brick.diff        = renko_period;
					}

					current_brick.low_price  = current_brick.close_price;
					current_brick.high_price = current_brick.open_price;

					if ( show_bricks ) {
						cout << "L+SHORT " << current_brick << endl;
					}

					// add brick to stack
					bricks.push_back( current_brick );
					//last_brick = current_brick;
					// reset current and last brick
					current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
					last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
				}
				break;
			// last brick is SHORT
			case RenkoBrick::STATUS::SHORT:
				// open new short brick
				if ( tick.bid < last_brick.close_price && Math::get_spread( tick.bid, last_brick.close_price, point_size ) >= renko_period ) {
					current_brick.open_time   = last_brick.close_time;
					current_brick.open_price  = last_brick.close_price;
					current_brick.diff        = Math::get_spread( tick.bid, last_brick.close_price, point_size );
					current_brick.close_price = tick.bid;
					current_brick.close_time  = tick.datetime;
					current_brick.status      = RenkoBrick::STATUS::SHORT;

					if ( current_brick.diff > renko_period ) {
						current_brick.close_price = last_brick.close_price - ( renko_period * point_size );
						current_brick.diff        = renko_period;
					}

					current_brick.low_price  = current_brick.close_price;
					current_brick.high_price = current_brick.open_price;

					if ( show_bricks ) {
						cout << "S+SHORT " << current_brick << endl;
					}

					// add brick to stack
					bricks.push_back( current_brick );
					//last_brick = current_brick;
					// reset current and last brick
					current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
					last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
				}
				// open new long brick
				else if ( tick.bid > last_brick.open_price && Math::get_spread( tick.bid, last_brick.open_price, point_size ) >= renko_period ) {
					current_brick.open_time   = last_brick.open_time;
					current_brick.open_price  = last_brick.open_price;
					current_brick.diff        = Math::get_spread( tick.bid, last_brick.open_price, point_size );
					current_brick.close_price = tick.bid;
					current_brick.close_time  = tick.datetime;
					current_brick.status      = RenkoBrick::STATUS::LONG;

					if ( current_brick.diff > renko_period ) {
						current_brick.close_price = last_brick.open_price + ( renko_period * point_size );
						current_brick.diff        = renko_period;
					}

					current_brick.low_price = current_brick.open_price;
					current_brick.high_price = current_brick.close_price;

					if ( show_bricks ) {
						cout << "S+LONG  " << current_brick << endl;
					}

					// add brick to stack
					bricks.push_back( current_brick );
					//last_brick = current_brick;
					// reset current and last brick
					current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
					last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
				}
				break;
			case RenkoBrick::STATUS::NOSTATUS:
				// do nothing
				break;
		}

		line_i++;
	}

	cout << "Write to file..." << endl;

	// write output	
	// FXCM
	ofile << "# Status,OpenTime,Open,High,Low,Close,CloseTime,Period,Volume,PointSize" << endl;		
	
	for( auto& b : bricks ) {

		if ( b.open_time == "" || b.open_price == 0 || b.close_time == "" || b.close_price == 0 ) {
			cout << "error brick " << b << endl;
			continue;
		}

		// FXCM
		// convert timestamps from 08/05/2018 21:03:56.102 to yyyymmdd-H:i:s.u
		std::vector<std::string> ot = str::explode( b.open_time, ' ' );
		std::vector<std::string> ott = str::explode( ot[0], '/' );
		std::stringstream open_time;
		open_time << ott[2] << ott[0] << ott[1] << "-" << ot[1];

		std::vector<std::string> ct = str::explode( b.close_time, ' ' );
		std::vector<std::string> ctt = str::explode( ct[0], '/' );
		std::stringstream close_time;
		close_time << ctt[2] << ctt[0] << ctt[1] << "-" << ct[1];

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
	ofile.close();
	
	cout << "Input file :  " << input_file << endl;
	cout << "Output file:  " << output_file << endl;
	cout << "Renko Period: " << setprecision(2) << fixed << renko_period << endl;
	cout << "Point Size:   " << setprecision(5) << fixed << point_size << endl;
	cout << "Ticks:        " << ticks.size() << endl;
	cout << "Bricks:       " << bricks.size() << endl;

	return EXIT_SUCCESS;
}