#include "RangeStrategy.h"
#include "indicator/RenkoIndicator.h"
#include <utility>
#include "Math.h"
#include <stdexcept>

namespace IDEFIX {
	/*!
	 * Constructor
	 */
	RangeStrategy::RangeStrategy(): m_open_sell(0), m_open_buy(0) {
		// subscribe to markets, AUD/USD, EUR/USD, GBP/USD, NZD/USD, USD/CAD, USD/CHF, USD/JPY
		// m_symbols.push_back( "AUD/USD" );
		// m_symbols.push_back( "EUR/USD" );
		m_symbols.push_back( "GBP/USD" );
		// m_symbols.push_back( "NZD/USD" );
		// m_symbols.push_back( "USD/CAD" );
		// m_symbols.push_back( "USD/CHF" );
		// m_symbols.push_back( "USD/JPY" );
	}

	/*!
	 * Deconstructor
	 */
	RangeStrategy::~RangeStrategy() {}

	/*!
	 * Get the identifier of this Strategy
	 * 
	 * @return const std::string
	 */
	std::string RangeStrategy::getIdentifier() const {
		return "RangeStrategy";
	}

	/*!
	 * onInit Callback
	 * 
	 * @param FIXManager& manager Reference to FIXManager
	 */
	void RangeStrategy::onInit(FIXManager& manager) {
		manager.console()->info( "[{}:onInit]", getIdentifier() );		

		for ( auto& symbol : m_symbols ) {
			// init renko indicator and callback for moving average
			initIndicator( manager, symbol );
			// subscribe to market data
			manager.subscribeMarketData( symbol );
		}
	}

	/*!
	 * onTick Callback
	 * 
	 * @param FIXManager&           manager  Reference to FIXManager
	 * @param const MarketSnapshot& snapshot Reference to the Snapshot
	 */
	void RangeStrategy::onTick(FIXManager& manager, const MarketSnapshot& snapshot) {}

	/*!
	 * onAccountChange Callback
	 * 
	 * @param FIXManager& manager Reference to FIXManager
	 * @param Account&    account Reference to Account
	 */
	void RangeStrategy::onAccountChange(FIXManager& manager, Account& account) {
		if ( manager.isExiting() ) {
			return;
		}

		manager.console()->info( "[{}:onAccountChange] {}", getIdentifier(), account.toString() );
	}

	/*!
	 * onPositionChange Callback
	 * 
	 * @param FIXManager&         manager  Reference to FIXManager
	 * @param MarketOrder&        position Reference to MarketOrder
	 * @param MarketOrder::Status status   The status
	 */
	void RangeStrategy::onPositionChange(FIXManager& manager, MarketOrder& position, MarketOrder::Status status) {
		std::ostringstream oss;

		switch( status ) {
			case MarketOrder::Status::NEW:
			oss << " NEW " << position.getPosID();
			break;
			case MarketOrder::Status::FILLED:
			oss << " FILLED " << position.getPosID();
			break;
			case MarketOrder::Status::STOPPED:
			oss << " STOPPED " << position.getPosID();
			break;
			case MarketOrder::Status::REMOVED:
			oss << " REMOVED " << position.getPosID();
			break;
			case MarketOrder::Status::UPDATE:
			oss << " UPDATE " << position.getPosID();
			break;
			default:
			manager.console()->warn( "[{}:onPositionChange] unknown status", getIdentifier() );
			break;
		}

		if ( ! oss.str().empty() ) {
			manager.console()->info( "[{}:onPositionChange] {}", getIdentifier(), oss.str().c_str() );
		}
	}

	/*!
	 * onExit Callback
	 * 
	 * @param FIXManager& manager Reference to FIXManager
	 */
	void RangeStrategy::onExit(FIXManager& manager) {
		manager.console()->info( "[{}:onExit]", getIdentifier() );

		for ( auto& symbol : m_symbols ) {
			// close all positions for symbols
			manager.closeAllPositions( symbol );
		}
	}

	/*!
	 * onError Callback
	 * 
	 * @param FIXManager& manager Reference to FIXManager
	 * @param std::string origin  The origin which sends the message
	 * @param std::string message The error message
	 */
	void RangeStrategy::onError(FIXManager& manager, std::string origin, std::string message) {
		manager.console()->error( "[{}:onError] origin: {} message: {}", getIdentifier(), origin, message );
	}

	/*!
	 * onRequestAck Callback
	 * 
	 * @param FIXManager& manager Reference to FIXManager
	 * @param std::string request_ident The ident which sends the message
	 * @param std::string text          The message text
	 */
	void RangeStrategy::onRequestAck(FIXManager& manager, std::string request_ident, std::string text) {
		manager.console()->warn( "[{}:onRequestAck] ident: {} message: {}", getIdentifier(), request_ident, text );
	}

	/*!
	 * Add moving average value to list and renko close price to list
	 * 
	 * @param const RenkoBrick& brick Reference to last renko brick
	 */
	void RangeStrategy::addListValues(const RenkoBrick& brick) {
		try {
			FIX::Locker lock( m_mutex );

			// calculate moving average for close price
			unsigned int avg_period = 3;
			#ifdef STRATEGY_AVG_PERIOD
				avg_period = STRATEGY_AVG_PERIOD;
			#endif
			int avg_offset = 0;
			#ifdef STRATEGY_AVG_OFFSET
				avg_offset = STRATEGY_AVG_OFFSET;
			#endif

			// the moving average value
			double ma_value = 0;
			// add close price to list
			auto it1 = m_renko_bricks.find( brick.symbol );
			if ( it1 != m_renko_bricks.end() ) {
	  			// found list, add value
				it1->second.push_back( brick );

				std::vector<double> temp_prices;
				for ( auto b : it1->second ) {
					temp_prices.push_back( b.close_price );
				}

				// calculate moving average
				ma_value = Math::get_moving_average( temp_prices, avg_period, avg_offset );
			} else {
	  			// list not found for symbol, add list
				std::vector<RenkoBrick> list;
				list.push_back( brick );

	  			// add list
				m_renko_bricks.insert( std::pair<std::string, std::vector<RenkoBrick> >( brick.symbol, list ) );
			}

			// add moving average to list
			auto it2 = m_moving_average.find( brick.symbol );
			if ( it2 != m_moving_average.end() ) {
	  			// found moving average list for symbol
	  			// add value to list
				it2->second.push_back( ma_value );
			} else {
	  			// no list found, create one
				std::vector<double> list;
				list.push_back( ma_value );
			    // add list to stack
				m_moving_average.insert( std::pair<std::string, std::vector<double> >( brick.symbol, list ) );
			}
		} catch( const std::length_error& error ){
			// catch this error, it works after first insert.
		}
	}

	/*!
	 * Get last moving average value for symbol
	 * 
	 * @param const std::string  symbol
	 * @return double
	 */
	double RangeStrategy::getLastMovingAverage(const std::string symbol) {
		FIX::Locker lock( m_mutex );
		auto it = m_moving_average.find( symbol );
		if ( it != m_moving_average.end() ) {
			// found average list
			return it->second.back();
		}

		return 0;
	}

	/*!
	 * Get last renko close price
	 * 
	 * @param const std::string  symbol 
	 * @return double
	 */
	double RangeStrategy::getLastRenkoPrice(const std::string symbol) {
		FIX::Locker lock( m_mutex );
		auto it = m_renko_bricks.find( symbol );
		if ( it != m_renko_bricks.end() ) {
			// found average list
			auto brick = it->second.back();
			return brick.close_price;
		}

		return 0;
	}

	/*!
	 * Get moving average for period
	 *
	 * @param const std::string symbol
	 * @param const int period
	 * @return double
	 */
	double RangeStrategy::getMovingAverage(const std::string symbol, const int period) {
		FIX::Locker lock( m_mutex );
		auto it = m_moving_average.find( symbol );
		if ( it != m_moving_average.end() ) {
			if ( period <= it->second.size() ) {
				auto ma_it = it->second.rbegin();
				ma_it += period;

				return *ma_it;
			}
		}

		return 0;
	}

	/*!
	 * Get renko close price for period
	 *
	 * @param const std::string symbol
	 * @param const int period
	 * @return double
	 */
	double RangeStrategy::getRenkoPrice(const std::string symbol, const int period) {
		FIX::Locker lock( m_mutex );
		auto it = m_renko_bricks.find( symbol );
		if ( it != m_renko_bricks.end() ) {
			if ( period <= it->second.size() ) {
				auto re_it = it->second.rbegin();
				re_it += period;

				return (*re_it).close_price;
			}
		}

		return 0;
	}

	/*!
	 * Get renko brick for period
	 * 
	 * @param const std::string symbol
	 * @param const int         period
	 * @param RenkoBrick&       brick Is getting set by method or left
	 */
	void RangeStrategy::getRenkoBrick(const std::string symbol, const int period, RenkoBrick& brick) {
		FIX::Locker lock( m_mutex );
		auto it = m_renko_bricks.find( symbol );
		if ( it != m_renko_bricks.end() ) {
			int size = (int) it->second.size();
			auto rit = it->second.rbegin();
			if ( size >= period ) {
				rit += period;

				brick = *rit;
			}
		}
	}

	/*!
	 * Initialize Indicators and the Strategy Logic in the Renko Callback
	 * 
	 * @param FIXManager&       manager Reference to FIXManager
	 * @param const std::string symbol  The symbol to initialize
	 */
	void RangeStrategy::initIndicator(FIXManager& manager, const std::string symbol) {
		// add range indicator for 3 pip range on bid price
		double brick_size = 0.5;
		#ifdef STRATEGY_RENKO_BRICK_SIZE
			brick_size = STRATEGY_RENKO_BRICK_SIZE;
		#endif

		manager.console()->info("[initIndicator] RenkoBrick Size {:.2f}", brick_size);

		// callback for renko indicator on every new brick
		manager.addIndicator( symbol, new RenkoIndicator( brick_size, RenkoIndicator::PriceType::BID_PRICE, 
			[&](const RenkoBrick& brick){
				// stop if manager is exiting
				if ( manager.isExiting() ) {
					return;
				}

				// calculate moving average for brick
				addListValues( brick );

				// proceed only if we have at least 3 renko bricks
				const int brick_count = getRenkoBrickCount( brick.symbol );
				if ( brick_count < 3 ) {
					manager.console()->info("Not enough bricks: {} {:d}/3", brick.symbol, brick_count);
					return;
				}

				// - double last_renko_price = getLastRenkoPrice( brick.symbol );
				double last_ma = getLastMovingAverage( brick.symbol );
				// if moving average is 0, return until it has some value
				if ( last_ma == 0 ) {
					return;
				}

				// - get last snapshot
				auto last_snapshot = manager.getLatestSnapshot( brick.symbol );
				// - spread
				const double last_spread = last_snapshot.getSpread();
				// - previous renko brick
				RenkoBrick previous_brick;
				getRenkoBrick( brick.symbol, 1, previous_brick);

				// - has open positions for symbol
				// bool has_open_positions = m_open_positions > 0; // manager.hasOpenPositions( brick.symbol );
				bool has_open_buy_pos = m_open_buy > 0;
				bool has_open_sell_pos = m_open_sell > 0;

				manager.console()->info( "BRICK {} {} OP@{:.5f} CP@{:.5f} MA@{:.5f} SP@{:.2f}", brick.symbol, brick.status == 1 ? "UP" : "DOWN", brick.open_price, brick.close_price, last_ma, last_spread );

				// entry conditions buy
				if ( ! has_open_buy_pos && last_spread < 1 ) {
					// entry signal long
					if ( previous_brick.close_price < brick.close_price && brick.close_price > last_ma && brick.open_price > last_ma ) {
						manager.console()->warn("BUY {} PREV {:.5f} < BP {:.5f} and BP {:.5f} > MA {:.5f}", 
							brick.symbol, previous_brick.close_price, brick.close_price, brick.close_price, last_ma );
						m_open_buy = 1;
					} 
				}
				// exit conditions buy
				else if ( has_open_buy_pos ) {
					// exit signal long
					if ( previous_brick.close_price > brick.close_price && brick.close_price < last_ma ) {
						manager.console()->warn("EXIT BUY {}: PREV {:.5f} > BP {:.5f} and BP {:.5f} < MA {:.5f}", 
							brick.symbol, previous_brick.close_price, brick.close_price, brick.close_price, last_ma );
						m_open_buy = 0;
					}
				}

				// entry conditions sell
				if( ! has_open_sell_pos && last_spread < 1 ) {
					// entry signal short
					if ( previous_brick.close_price > brick.close_price && brick.close_price < last_ma && brick.open_price < last_ma ) {
						manager.console()->warn("SELL {} PREV {:.5f} > BP {:.5f} and BP {:.5f} < MA {:.5f}", 
							brick.symbol, previous_brick.close_price, brick.close_price, brick.close_price, last_ma );
						m_open_sell = 1;
					}
				}
				// exit conditions sell
				else if ( has_open_sell_pos ) {
					// exit singal short
					if ( previous_brick.close_price < brick.close_price && brick.close_price > last_ma ) {
						manager.console()->warn("EXIT SHORT {}: PREV {:.5f} < BP {:.5f} and BP {:.5f} > MA {:.5f}", 
							brick.symbol, previous_brick.close_price, brick.close_price, brick.close_price, last_ma );
						m_open_sell = 0;
					}
				}
			
			} ) // - end callback
		);
	}

	/*!
	 * Get count of renko bricks for symbol
	 * 
	 * @param const std::string  symbol
	 * @return int
	 */
	int RangeStrategy::getRenkoBrickCount(const std::string symbol) {
		if ( m_renko_bricks.empty() ) return 0;

		auto it = m_renko_bricks.find( symbol );
		if ( it != m_renko_bricks.end() ) {
			return (int)it->second.size();
		}

		return 0;
	}
};