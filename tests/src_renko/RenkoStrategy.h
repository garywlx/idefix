#include "Strategy.h"

class RenkoStrategy: public Strategy {
private:
	SimpleMovingAverage m_sma5;
	// SimpleMovingAverage m_sma10;

public:
	RenkoStrategy(const double account, const double pyramid, const bool verbose): Strategy(account, pyramid, verbose) {
		m_sma5.set_period( 5 );
		// m_sma10 = new SimpleMovingAverage( 10 );
	}

	void on_brick(const RenkoBrick& brick) {

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