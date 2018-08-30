#define RS_SHOW_DEBUG true

#include "RenkoStrategy.h"
#include "../Chart.h"
#include "../RenkoChart.h"
#include "../MarketSnapshot.h"
#include "../indicator/Bar.h"
#include "../Math.h"
#include "../MarketOrder.h"
#include <quickfix/FixValues.h>

#if RS_SHOW_DEBUG
#include "../Console.h"
#include <sstream>
#endif

namespace IDEFIX {

RenkoStrategy::RenkoStrategy() {}

// is called on initialization
void RenkoStrategy::on_init(Chart* chart) {
#if RS_SHOW_DEBUG
	console()->info("[RenkoStrategy:{}] {}", chart->symbol(), __FUNCTION__ );
#endif

	// set max values
	// how many parallel short positions are allowed?
	m_max_short_pos = 3;
	// how many short positions actually?
	m_short_pos     = 0;
	// how many parallel long positions are allowed?
	m_max_long_pos  = 3;
	// how many long positions actually?
	m_long_pos      = 0;
	// wait for at least 5 bricks before entering the markets
	m_wait_bricks   = 5;
	// maximum risk per trade in percent
	m_max_risk      = 1;
}

// is called on chart tick
void RenkoStrategy::on_tick(Chart* chart, const MarketSnapshot& tick) {
#if RS_SHOW_DEBUG
	//console()->info("[RenkoStrategy:{}] {} {}", chart->symbol(), __FUNCTION__, ss.str() );
#endif
}

// is called on chart bar
void RenkoStrategy::on_bar(Chart* chart, const Bar& bar) {
	try {
		auto derived_chart = static_cast<RenkoChart*>( chart );

		int brick_count = derived_chart->brick_count();
		if ( brick_count < m_wait_bricks ) {
			console()->info("[RenkoStrategy] Waiting for minimum bar count ({:d}/{:d})", brick_count, m_wait_bricks );
			return;
		}

#if RS_SHOW_DEBUG
		std::stringstream ss;
		ss << bar;
		console()->info("[RenkoStrategy:{}] {}", chart->symbol(), ss.str() );
#endif

		// add value to moving average
		m_sma5.add( ( bar.open_price + bar.close_price + bar.high_price + bar.low_price ) / 4 );

		// get previous bar
		//auto previous_bar = chart->at( 1 );
		auto previous_bar = derived_chart->at( 1 );

		// shortcuts
		double last_ma    = m_sma5.value();
		double open       = bar.open_price;
		double close      = bar.close_price;
		double open_1     = previous_bar.open_price;
		double close_1    = previous_bar.close_price;
		bool enter_long   = false;
		bool exit_long    = false;
		bool enter_short  = false;
		bool exit_short   = false;
		bool bar_long     = open < close;
		bool bar_1_long   = open_1 < close_1;
		bool bar_above_ma = open > last_ma && close > last_ma;
		bool bar_below_ma = open < last_ma && close < last_ma;

		// RULES
		// ------------------------------
		// entry signal long
		// ------------------------------
		// brick_1 == SHORT
		// brick   == LONG
		// OR
		// brick_1 == LONG
		// brick   == LONG
		// 
		// brick > last_ma
		if ( ( ( bar_long && ! bar_1_long ) || ( bar_long && bar_1_long ) ) && bar_above_ma ) {
			enter_long = true;
		}

		// ------------------------------
		// exit signal long
		// ------------------------------
		// brick_1 == LONG
		// brick   == SHORT
		// brick.close <= last_ma
		if ( ! bar_long && bar_1_long && close <= last_ma ) {
			exit_long = true;
		}

		// ------------------------------
		// entry signal short
		// ------------------------------
		// brick_1 == LONG
		// brick   == SHORT
		// OR
		// brick_1 == SHORT
		// brick   == SHORT
		// 
		// brick < last_ma
		if ( ( ( bar_1_long && ! bar_long ) || ( ! bar_1_long && ! bar_long ) ) && bar_below_ma ) {
			enter_short = true;
		}

		// ------------------------------
		// exit singal short
		// ------------------------------
		// brick_1 == SHORT
		// brick   == LONG
		// brick.close >= last_ma
		if ( ! bar_1_long && bar_long && close >= last_ma ) {
			exit_short = true;
		}

		// ------------------------------
		// EXECUTE
		// ------------------------------
		auto conversion_price = 0;
		auto free_margin = 10000;

		MarketOrder mo;
		mo.setSymbol( chart->symbol() );

		if ( free_margin <= 0 ) {
			enter_long = false;
			enter_short = false;
		} else {
			// calculate quantity;
			auto qty = Math::get_unit_size( free_margin, 1, 1, conversion_price );
			mo.setQty( qty );
		}

		// LONG ENTRY
		if ( enter_long && m_long_pos < m_max_long_pos ) {
			console()->info("[EnterLong] {:d} {}", m_long_pos, chart->symbol() );

			mo.setSide( FIX::Side_BUY );

			ss.str("");
			ss << mo;
			console()->info("{}", ss.str() );

			//chart->api()->marketOrder();
			m_long_pos++;
		}
		// LONG EXIT
		if ( exit_long && m_long_pos > 0 ) {
			console()->warn("[ExitLong] {:d} {}", m_long_pos, chart->symbol() );

			//chart->api()->closeAllPositions( chart->symbol() );

			m_long_pos = 0;
		}
		// SHORT ENTRY
		if ( enter_short && m_short_pos < m_max_short_pos ) {
			console()->error("[EnterShort] {:d} {}", m_short_pos, chart->symbol() );

			mo.setSide( FIX::Side_SELL );

			ss.str("");
			ss << mo;
			console()->info("{}", ss.str() );

			m_short_pos++;
		}
		// SHORT EXIT
		if( exit_short && m_short_pos > 0 ) {
			console()->warn("[ExitShort] {:d} {}", m_short_pos, chart->symbol() );

			//chart->api()->closeAllPositions( chart->symbol() );

			m_short_pos = 0;
		}
	} catch( IDEFIX::out_of_range* oor ) {
		console()->warn("{}", oor->what() );
	} catch( IDEFIX::element_not_found* enf ) {
		console()->warn("{}", enf->what() );
	} catch(...) {

	}
}

// is called on exit
void RenkoStrategy::on_exit(Chart* chart) {
#if RS_SHOW_DEBUG
	console()->info("[RenkoStrategy:{}] {}", chart->symbol(), __FUNCTION__ );
#endif
}

};