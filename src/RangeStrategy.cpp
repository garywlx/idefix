#include "RangeStrategy.h"
#include "indicator/RenkoIndicator.h"
#include <utility>
#include "Math.h"
#include <stdexcept>

namespace IDEFIX {
	RangeStrategy::RangeStrategy() {

	}

	RangeStrategy::~RangeStrategy() {}

	std::string RangeStrategy::getIdentifier() const {
		return "RangeStrategy";
	}

	void RangeStrategy::onInit(FIXManager& manager) {
		manager.console()->info( "[{}:onInit]", getIdentifier() );

		// subscribe to markets, AUD/USD, EUR/USD, GBP/USD, NZD/USD, USD/CAD, USD/CHF, USD/JPY
		std::vector<std::string> symbols {"EUR/USD"};

		for ( auto& symbol : symbols ) {
			// init renko indicator and callback for moving average
			initIndicator( manager, symbol );
			// subscribe to market data
			manager.subscribeMarketData( symbol );
		}
	}

	void RangeStrategy::onTick(FIXManager& manager, const MarketSnapshot& snapshot) {
		if ( manager.isExiting() ) {
			return;
		}
	}

	void RangeStrategy::onAccountChange(FIXManager& manager, Account& account) {
		if ( manager.isExiting() ) {
			return;
		}

		manager.console()->info( "[{}:onAccountChange] {}", getIdentifier(), account.toString() );
	}

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

	void RangeStrategy::onExit(FIXManager& manager) {
		manager.console()->info( "[{}:onExit]", getIdentifier() );
	}

	void RangeStrategy::onError(FIXManager& manager, std::string origin, std::string message) {
		manager.console()->error( "[{}:onError] origin: {} message: {}", getIdentifier(), origin, message );
	}

	void RangeStrategy::onRequestAck(FIXManager& manager, std::string request_ident, std::string text) {
		manager.console()->warn( "[{}:onRequestAck] ident: {} message: {}", getIdentifier(), request_ident, text );
	}

/*!
 * Add moving average value to list and renko close price to list
 * 
 * @param const RenkoBrick& brick Reference to last renko brick
 */
void RangeStrategy::addMovingAverageValue(const RenkoBrick& brick) {
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
		auto it1 = m_renko_close_prices.find( brick.symbol );
		if ( it1 != m_renko_close_prices.end() ) {
  			// found list, add value
			it1->second.push_back( brick.close_price );

			// calculate moving average
			ma_value = Math::get_moving_average( it1->second, avg_period, avg_offset );
		} else {
  			// list not found for symbol, add list
			std::vector<double> list;
			list.push_back( brick.close_price );

  			// add list
			m_renko_close_prices.insert( std::pair<std::string, std::vector<double> >( brick.symbol, list ) );
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
 * @param const std::string  symbol 
 * @return double
 */
double RangeStrategy::getLastRenkoPrice(const std::string symbol) {
	FIX::Locker lock( m_mutex );
	auto it = m_renko_close_prices.find( symbol );
	if ( it != m_renko_close_prices.end() ) {
	// found average list
		return it->second.back();
	}

	return 0;
}

// Init range indicators per symbol and callbacks
void RangeStrategy::initIndicator(FIXManager& manager, const std::string symbol) {
	// add range indicator for 3 pip range on bid price
	double brick_size = 0.5;
	#ifdef STRATEGY_RENKO_BRICK_SIZE
		brick_size = STRATEGY_RENKO_BRICK_SIZE;
	#endif
	manager.addIndicator( symbol, new RenkoIndicator( brick_size, RenkoIndicator::PriceType::BID_PRICE, 
		[&](const RenkoBrick& brick){
			// stop if manager is exiting
			if ( manager.isExiting() ) {
				return;
			}

			// calculate moving average for brick
			addMovingAverageValue( brick );

			// inform for debug
			// - double last_renko_price = getLastRenkoPrice( brick.symbol );
			double last_ma = getLastMovingAverage( brick.symbol );
			// log
			// manager.console()->warn( "[{} BRICK] {} close {:f} moving average {:f}", 
			// 	brick.symbol,
			// 	brick.status == 1 ? "UP" : "DOWN",
			// 	brick.close_price,
			// 	last_ma
			// );
			std::cout << brick.close_price << "," << last_ma << std::endl;
			// debug eol
		} )
	);
}

};