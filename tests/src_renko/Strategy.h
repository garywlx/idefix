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

	double m_account;

	std::stringstream m_tradelog;

public:
	Strategy(const double account, const int pyramid, const bool verbose = false): 
	  m_account( account ), m_open_short(0), m_open_long(0), m_total_short_positions(0), 
	  m_total_long_positions(0), m_max_pyramid(pyramid), m_verbose(verbose) {
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

	inline std::vector<Position> positions() {
		return m_positions;
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

		return pnl;
	}

	inline void stats() {
		int ticks = 0;
		for( auto& b : m_bricks ) {
			ticks += b.volume;
		}

		cout << "Start Time:            " << m_bricks[0].open_time << endl;
		cout << "Stop Time:             " << m_bricks.back().close_time << endl;
		cout << "Period:                " << setprecision(0) << m_bricks[0].period << endl;
		cout << "Bricks:                " << setprecision(0) << m_bricks.size() << endl;
		cout << "Ticks:                 " << setprecision(0) << ticks << endl;
		cout << "Pyramid Trades:        " << setprecision(0) << m_max_pyramid << endl;
		cout << "==============================================" << endl;
		cout << "Total Long Positions:  " << setprecision(0) << total_long_positions() << endl;
		cout << "Total Short Positions: " << setprecision(0) << total_short_positions() << endl;
		cout << "Account begin:         " << setprecision(2) << fixed << m_account << " USD" << endl;
		cout << "Account end:           " << setprecision(2) << fixed << ( m_account + total_profit() ) << " USD" << endl;
		cout << "Total Profit:          " << setprecision(2) << fixed << total_profit() << " USD" << endl;
		cout << "----------------------------------------------" << endl;
	}

	virtual void on_brick(const RenkoBrick& brick) = 0;

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
					m_open_long--;
					if ( m_open_long < 0 ) {
						m_open_long = 0;
					}
				} else if ( status == RenkoBrick::STATUS::SHORT ) {
					m_open_short--;
					if ( m_open_long < 0 ) {
						m_open_short = 0;
					}
				}

				m_tradelog << get_tradelog_line( pos );
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
			m_open_long++;
			m_total_long_positions++;
		} else {
			m_open_short++;
			m_total_short_positions++;
		}

		m_tradelog << get_tradelog_line( pos );
	}

	/*!
	 * Add trade log line
	 * 
	 * @param const Position& pos Position reference.
	 * @return std::string
	 */
	inline std::string get_tradelog_line(const Position& pos) {
		std::stringstream ss;

		ss << "[";
		ss << "'" << ( pos.status == Position::STATUS::OPEN ? pos.open_time : pos.close_time ) << "',";
		ss << std::setprecision(5) << fixed << pos.open_price << ",";
		ss << std::setprecision(5) << fixed << pos.close_price << ",";
		ss << std::setprecision(0) << pos.qty << ",";
		ss << std::setprecision(2) << fixed << pos.profit << ",";
		ss << "'" << ( pos.status == Position::STATUS::OPEN ? "open" : "close" ) << "',";
		ss << "'" << ( pos.direction == RenkoBrick::STATUS::LONG ? "buy" : "sell" ) << "'";
		ss << "]," << endl;

		return ss.str();
	}

	/*!
	 * Get trade log as string
	 * 
	 * @return std::string
	 */
	inline std::string tradelog() {
		return m_tradelog.str();
	}
};