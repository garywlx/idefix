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

using namespace std;
using namespace IDEFIX;
using namespace FIX;

struct Position {
	enum STATUS {
		OPEN, CLOSED
	};
	double open_price;
	double close_price;
	double profit;
	double qty;
	std::string open_time;
	std::string close_time;
	RenkoBrick::STATUS direction;
	STATUS status;
};

inline std::ostream& operator<<(std::ostream& out, const Position& pos) {
	out << "\033[" << ( pos.direction == RenkoBrick::STATUS::SHORT ? "31" : "32" ) << "m";
	out << "POS " << pos.open_time << "@" << setprecision(2) << fixed << pos.open_price;
	if ( pos.status == Position::STATUS::CLOSED ) {
		out << " " << pos.close_time << "@" << setprecision(2) << fixed << pos.close_price;	
	}
	out << " Qty " << setprecision(0) << pos.qty << " Profit " << setprecision(2) << fixed << pos.profit << " USD"; 
	out << "\033[0m";
	return out;
}

class Strategy {
protected:
	std::vector<RenkoBrick> m_bricks;
	std::vector<Position> m_positions;
	int m_open_long;
	int m_open_short;

	int m_total_long_positions;
	int m_total_short_positions;

	int m_max_pyramid;
	bool m_verbose;

	// trade log
	ofstream m_ofile;
	bool m_trade_log;

public:
	Strategy(const int pyramid, const bool verbose = false): 
	  m_open_short(0), m_open_long(0), m_total_short_positions(0), 
	  m_total_long_positions(0), m_max_pyramid(pyramid), m_verbose(verbose), m_trade_log(false) {
		registerConsole();
	}

	inline int total_long_positions() const {
		return m_total_long_positions;
	}

	inline int total_short_positions() const {
		return m_total_short_positions;
	}

	inline double total_profit() const {
		double profit = 0;

		for ( auto& pos : m_positions ) {
			if ( pos.status == Position::STATUS::CLOSED ) {
				profit += pos.profit;
			}
		}

		return profit;
	}

	inline void show_trade_list() {
		for ( auto& pos : m_positions ) {
			cout << pos << endl;
		}
	}

	inline double profit(const Position& pos, const RenkoBrick& brick) {
		double pip_diff_decimal = 0;
		double pnl = 0;
		double pip_value = 10;

		if ( pos.direction == RenkoBrick::STATUS::LONG ) {
			pip_diff_decimal = brick.close_price - pos.open_price;
		} else {
			pip_diff_decimal = pos.open_price - brick.close_price;
		}

		double pips = pip_diff_decimal / brick.point_size;

		pnl = ( ( pips * ( pos.qty / 100000 ) ) * pip_value );

		console()->info( "pnl {:f}", pnl );

		return pnl;
	}

	virtual void onBrick(const RenkoBrick& brick) = 0;

	/*!
	 * Close position
	 * 
	 * @param status [description]
	 * @param brick  [description]
	 */
	inline void close_position(const RenkoBrick::STATUS status, const RenkoBrick& brick) {
		for ( auto& pos : m_positions ) {
			if ( pos.direction == status && pos.status == Position::STATUS::OPEN ) {
				// set values
				pos.close_price = brick.close_price;
				pos.close_time  = brick.close_time;

				// calculate profit						
				pos.profit = profit(pos, brick);

				// set status
				pos.status = Position::STATUS::CLOSED;

				if ( status == RenkoBrick::STATUS::LONG ) {

					console()->info( "exit long position" );

					m_open_long--;
					if ( m_open_long < 0 ) {
						m_open_long = 0;
					}
				} else if ( status == RenkoBrick::STATUS::SHORT ) {

					console()->info( "exit short position" );

					m_open_short--;
					if ( m_open_long < 0 ) {
						m_open_short = 0;
					}
				}

				add_tradelog( pos );
			}
		}
	}

	/*!
	 * Open position
	 * 
	 * @param direction [description]
	 * @param brick     [description]
	 * @param qty       [description]
	 */
	inline void open_position(const RenkoBrick::STATUS direction, const RenkoBrick& brick, const double qty) {
		Position pos;
		pos.open_time  = brick.close_time;
		pos.open_price = brick.close_price;
		pos.profit     = 0;
		pos.direction  = direction;
		pos.status     = Position::STATUS::OPEN;
		pos.qty        = qty;

		m_positions.push_back( pos );
		if ( pos.direction == RenkoBrick::STATUS::LONG ) {

			console()->info( "open long position" );

			m_open_long++;
			m_total_long_positions++;
		} else {

			console()->info( "open short position" );

			m_open_short++;
			m_total_short_positions++;
		}

		add_tradelog( pos );
	}

	/*!
	 * Begin trade log output
	 */
	inline void begin_tradelog() {
		if ( ! m_trade_log ) {
			return;
		}

		m_ofile << "var tradeseries = [";
	}

	/*!
	 * Add trade log line
	 * 
	 * @param const Position& pos Position reference.
	 */
	inline void add_tradelog(const Position& pos) {
		if ( ! m_trade_log ) {
			return;
		}

		m_ofile << "[";
		m_ofile << "'" << ( pos.status == Position::STATUS::OPEN ? pos.open_time : pos.close_time ) << "',";
		m_ofile << std::setprecision(5) << fixed << pos.open_price << ",";
		m_ofile << std::setprecision(5) << fixed << pos.close_price << ",";
		m_ofile << std::setprecision(0) << pos.qty << ",";
		m_ofile << std::setprecision(2) << fixed << pos.profit << ",";
		m_ofile << "'" << ( pos.status == Position::STATUS::OPEN ? "open" : "close" ) << "',";
		m_ofile << "'" << ( pos.direction == RenkoBrick::STATUS::LONG ? "buy" : "sell" ) << "'";
		m_ofile << "]," << endl;
	}

	/*!
	 * End trade log output
	 */
	inline void end_tradelog() {
		if ( ! m_trade_log ) {
			return;
		}

		m_ofile << "];" << endl;
	}
};

class RenkoStrategy: public Strategy {
private:
	SimpleMovingAverage m_sma5;
	// SimpleMovingAverage m_sma10;

public:
	RenkoStrategy(const double pyramid, const bool verbose, const std::string output_file): Strategy(pyramid, verbose) {
		m_sma5.set_period( 5 );
		// m_sma10 = new SimpleMovingAverage( 10 );
		
		// open output file
		if ( ! output_file.empty() ) {
			m_ofile.open( output_file, std::ofstream::out | std::ofstream::trunc );
			
			if ( ! m_ofile.good() ) {
				cerr << "ERROR: output file has an error!" << endl;
				m_ofile.close();
			} else {
				m_trade_log = true;
			}
		}
	}

	void onBrick(const RenkoBrick& brick) {

		if ( m_bricks.size() == 0 ) {
			m_bricks.push_back( brick );
			m_sma5.add( ( brick.open_price + brick.high_price + brick.low_price + brick.close_price ) / 4 );
			// m_sma10.add( ( brick.open_price + brick.high_price + brick.low_price + brick.close_price ) / 4 );
			return;
		}

		// calculate smas
		m_sma5.add( ( brick.open_price + brick.high_price + brick.low_price + brick.close_price ) / 4 );
		// m_sma10.add( ( brick.open_price + brick.high_price + brick.low_price + brick.close_price ) / 4 );

		// get last brick
		auto last_brick = m_bricks.back();
		// last brick is long?
		bool brick_1_long = last_brick.status == RenkoBrick::STATUS::LONG;
		// current brick is long?
		bool brick_long = brick.status == RenkoBrick::STATUS::LONG;

		// get values for moving average
		std::vector<double> value_list;
		for( auto& b : m_bricks ) {
			value_list.push_back( ( b.open_price + b.high_price + b.low_price + b.close_price ) / 4 );
		}

		// fast moving average
		double last_ma = m_sma5.value();
		// double slow_ma = m_sma10.value();

		// is current brick above last_ma?
		bool brick_above_ma = brick.open_price > last_ma && brick.close_price > last_ma;
		// is current brick below last ma?
		bool brick_below_ma = brick.open_price < last_ma && brick.close_price < last_ma;

		// enter long
		// brick_1 == SHORT
		// brick   == LONG
		// OR
		// brick_1 == LONG
		// brick   == LONG
		// 
		// brick > last_ma
		bool enter_long = ! brick_1_long && brick_long;
		if ( ! enter_long ) {
			enter_long = brick_1_long && brick_long;
		}
		if ( enter_long ) {
			enter_long = brick_above_ma;
		}

		// exit long
		// brick_1 == LONG
		// brick   == SHORT
		// brick.close < last_ma
		bool exit_long = false;
		if ( m_open_long > 0 ) {
			exit_long = brick_1_long && ! brick_long;
		}
		if ( exit_long ) {
			exit_long = brick.close_price < last_ma;
		}
		
		// enter short
		// brick_1 == LONG
		// brick   == SHORT
		// OR
		// brick_1 == SHORT
		// brick   == SHORT
		// 
		// brick < last_ma
		bool enter_short = brick_1_long && ! brick_long;
		if ( ! enter_short ) {
			enter_short = ! brick_1_long && ! brick_long;
		}
		if ( enter_short ) {
			enter_short = brick_below_ma;
		}

		// exit short
		bool exit_short = false;
		if ( m_open_short > 0 ) {
			exit_short = ! brick_1_long && brick_long;
		}
		if ( exit_short ) {
			exit_short = brick.close_price > last_ma;
		}

		// Risk Management
		// Calculate position size
		// 
		// Set max drawdown 25% of equity
		// 
		
		// open positions
		// buy
		if ( enter_long && m_open_long == 0 ) {
			open_position( RenkoBrick::STATUS::LONG, brick, 100000 );
		}
		// sell
		// 
		if ( enter_short && m_open_short == 0 ) {
			open_position( RenkoBrick::STATUS::SHORT, brick, 100000 );
		}

		// close positions
		// long
		// 
		if ( exit_long ) {
			close_position( RenkoBrick::STATUS::LONG, brick );
		}
		// short
		// 
		if ( exit_short ) {
			close_position( RenkoBrick::STATUS::SHORT, brick );
		}

		// add brick to list
		m_bricks.push_back( brick );
	}
};

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