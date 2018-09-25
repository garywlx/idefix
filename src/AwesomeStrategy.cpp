#include "AwesomeStrategy.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <functional>
#include <quickfix/FieldConvertors.h>

#include "Console.h"
#include "Exceptions.h"
#include "MarketOrder.h"
#include "MathHelper.h"
#include "StringHelper.h"
#include "CSVHandler.h"

namespace IDEFIX {
	AwesomeStrategy::AwesomeStrategy(const std::string& symbol, const AwesomeStrategyConfig& config): m_symbol( symbol ) {
		// Constructor
		// Set config values
		m_max_short_pos = config.max_short_pos;
		m_max_long_pos  = config.max_long_pos;
		m_wait_bricks   = config.wait_bricks;
		m_max_risk      = config.max_risk;
		m_max_qty       = config.max_qty;
		m_max_spread    = config.max_spread;
		m_renko_size    = config.renko_size;
		m_sma_size      = config.sma_size;
	}

	AwesomeStrategy::~AwesomeStrategy() {
		if ( m_chart != nullptr ) {
			delete m_chart;
		}
		if ( m_sma5 != nullptr ) {
			delete m_sma5;
		}
	}

	/*!
	 * SLOT gets called when fixmanager is ready for initializing
	 */
	void AwesomeStrategy::on_init() {
		console()->info("[AwesomeStrategy] on_init {}", get_symbol() );
		// how many short positions actually?
		m_short_pos     = 0;		
		// how many long positions actually?
		m_long_pos      = 0;
		// set renko chart periode
		m_chart = new RenkoChart( m_renko_size );
		// set sma periode
		m_sma5 = new SimpleMovingAverage( m_sma_size );
		// connect signal
		m_chart->on_brick.connect( std::bind( &AwesomeStrategy::on_bar, this, std::placeholders::_1 ) );
	}

	/*!
	 * SLOT gets called if there is a new tick available
	 * 
	 * @param MarketSnapshot& tick
	 */
	void AwesomeStrategy::on_tick(const IDEFIX::MarketSnapshot &tick) {
		// update chart
		m_chart->on_tick( tick );
		// set current spread
		m_current_spread = tick.getSpread();
	}

	/*!
	 * SLOT gets called if chart creates a new bar
	 * 
	 * @param const Bar& bar
	 */
	void AwesomeStrategy::on_bar(const Bar &bar) {
		try {

			// call external signal on_bar
			on_bar_signal( bar );

			// add value to moving average
			m_sma5->add( ( bar.open_price + bar.close_price ) / 2 );

#ifndef CMAKE_RELEASE_LOG
			// write bar with sma value to log file
			log_brick( bar, m_sma5->value() );
#endif
			// if ( bar.status == Bar::STATUS::LONG ) {
			// 	console()->info("[AwesomeStrategy] Bar open {:.5f} close {:.5f} sma {:.5f}", bar.open_price, bar.close_price, m_sma5->value() );
			// } else {
			// 	console()->error("[AwesomeStrategy] Bar open {:.5f} close {:.5f} sma {:.5f}", bar.open_price, bar.close_price, m_sma5->value() );
			// }

			if ( ! m_sma5->is_valid() ) {
				console()->info("[AwesomeStrategy] {} Waiting for valid sma.", get_symbol() );
				return;
			}

			int brick_count = m_chart->brick_count();
			if ( brick_count < m_wait_bricks ) {
				console()->info("[AwesomeStrategy] {} Waiting for minimum bar count ({:d}/{:d})", get_symbol(), brick_count, m_wait_bricks );
				return;
			}

			// get previous bar
			auto previous_bar = m_chart->at( 1 );

			// shortcuts
			double last_ma    = m_sma5->value();
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
			if ( ( ( bar_long && ! bar_1_long ) || ( bar_long && bar_1_long ) ) && bar_above_ma && m_current_spread <= m_max_spread ) {
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
			if ( ( ( bar_1_long && ! bar_long ) || ( ! bar_1_long && ! bar_long ) ) && bar_below_ma && m_current_spread <= m_max_spread ) {
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

			// LONG ENTRY
			if ( enter_long && m_long_pos < m_max_long_pos ) {
				console()->info("[SignalLong] {:d} {}", m_long_pos, get_symbol() );

				// Signal
				on_entry_signal( MarketSide::Side_BUY );
				
				m_long_pos++;
			}
			// LONG EXIT
			if ( exit_long && m_long_pos > 0 ) {
				console()->warn("[ExitLong All] {:d} {}", m_long_pos, get_symbol() );

				// Signal
				on_close_all_signal( get_symbol() );

				m_long_pos = 0;
			}
			// SHORT ENTRY
			if ( enter_short && m_short_pos < m_max_short_pos ) {
				console()->error("[SignalShort] {:d} {}", m_short_pos, get_symbol() );

				// Signal
				on_entry_signal( MarketSide::Side_SELL );

				m_short_pos++;
			}
			// SHORT EXIT
			if( exit_short && m_short_pos > 0 ) {
				console()->warn("[ExitShort All] {:d} {}", m_short_pos, get_symbol() );

				// Signal
				on_close_all_signal( get_symbol() );

				m_short_pos = 0;
			}
		} catch( IDEFIX::out_of_range* oor ) {
			console()->warn("{}", oor->what() );
		} catch( IDEFIX::element_not_found* enf ) {
			console()->warn("{}", enf->what() );
		} catch(...) {

		}
	}

	/*!
	 * SLOT gets called on exit
	 */
	void AwesomeStrategy::on_exit() {
		console()->info("[AwesomeStrategy] on_exit {}", get_symbol() );
	}

	/*!
	 * SLOT gets called if a new market order status is available
	 * 
	 * @param const SignalType type
	 * @param const MarketOrder& mo
	 */
	void AwesomeStrategy::on_market_order(const SignalType type, const MarketOrder& mo) {
		if( type == SignalType::MARKET_ORDER_NEW ) {
			console()->info("[AwesomeStrategy] MarketOrder {} {} filled.", mo.getSymbol(), mo.getSideStr() );
		}
	}

	std::string& AwesomeStrategy::get_symbol() {
		return m_symbol;
	}

	double AwesomeStrategy::get_max_risk() const {
		return m_max_risk;
	}

	double AwesomeStrategy::get_max_qty() const {
		return m_max_qty;
	}

	double AwesomeStrategy::get_max_spread() const {
		return m_max_spread;
	}

	/*!
	 * Save brick to <symbol>_bars.csv in running directory
	 * 
	 * @param const Bar& brick
	 * @param const double      sma
	 */
	void AwesomeStrategy::log_brick(const Bar& brick, const double sma) {
		// parse dates
		auto open_dt = FIX::UtcTimeStampConvertor::convert( brick.open_time );
		std::stringstream open_ss;
		open_ss << open_dt.getYear() << "-" 
				 << setfill('0') << setw(2) << open_dt.getMonth() << "-" 
				 << setfill('0') << setw(2) << open_dt.getDay() << " " 
				 << setfill('0') << setw(2) << open_dt.getHour() << ":" 
				 << setfill('0') << setw(2) << open_dt.getMinute() << ":" 
				 << setfill('0') << setw(2) << open_dt.getSecond() << "." << open_dt.getMillisecond();

		auto close_dt = FIX::UtcTimeStampConvertor::convert( brick.close_time );
		std::stringstream close_ss;
		close_ss << close_dt.getYear() << "-" 
				 << setfill('0') << setw(2) << close_dt.getMonth() << "-" 
				 << setfill('0') << setw(2) << close_dt.getDay() << " " 
				 << setfill('0') << setw(2) << close_dt.getHour() << ":" 
				 << setfill('0') << setw(2) << close_dt.getMinute() << ":" 
				 << setfill('0') << setw(2) << close_dt.getSecond() << "." << close_dt.getMillisecond();

		 // make filename for symbol bars
		std::stringstream symbol_filename_ss;
		symbol_filename_ss << brick.symbol.c_str() << "_bars.csv";

		// create line
		std::stringstream line_ss;
		line_ss << open_ss.str() << ",";
		line_ss << brick.open_price << ",";
		line_ss << brick.high_price << ",";
		line_ss << brick.low_price << ",";
		line_ss << brick.close_price << ",";
		line_ss << close_ss.str() << ",";
		line_ss << brick.volume << ",";
		line_ss << sma << ",";
		line_ss << brick.point_size;

		// write to csv file
		CSVHandler csv;
		csv.set_path( "public_html/" );
		csv.set_filename( symbol_filename_ss.str() );
		csv.add_line( line_ss.str() );

	}

};