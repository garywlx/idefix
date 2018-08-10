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

		// add range indicator for 3 pip range on bid price
		manager.addIndicator( "EUR/USD", new RenkoIndicator( 0.5, RenkoIndicator::PriceType::BID_PRICE, 
			[&](const RenkoBrick& brick){

				// inform for debug
				manager.console()->warn( "[{} BRICK] {} open {:f} close {:f} ticks {:d} close_time {}", 
					brick.symbol,
					brick.status == 1 ? "UP" : "DOWN",
					brick.open_price,
					brick.close_price,
					brick.tick_volume,
					brick.close_time
				);
				// debug eol

				addMovingAverageValue( brick );

				double last_renko_price = getLastRenkoPrice( brick.symbol );
				double last_ma = getLastMovingAverage( brick.symbol );

				manager.console()->info("[{} MovingAverage] {:f} last close {:f}", brick.symbol, last_ma, last_renko_price );
			} )
		);

	// subscribe market data
	//manager.subscribeMarketData( "AUD/USD" );
		manager.subscribeMarketData( "EUR/USD" );
	// manager.subscribeMarketData( "GBP/USD" );
	// manager.subscribeMarketData( "NZD/USD" );
	// manager.subscribeMarketData( "USD/CAD" );
	// manager.subscribeMarketData( "USD/CHF" );
	// manager.subscribeMarketData( "USD/JPY" );
	}

	void RangeStrategy::onTick(FIXManager& manager, const MarketSnapshot& snapshot) {
		if ( manager.isExiting() ) {
			return;
		}

	//manager.console()->info( "[Strategy:onTick] {} bid@{:.5f} ask@{:.5f} spread@{:.2f}", snapshot.getSymbol(), snapshot.getBid(), snapshot.getAsk(), snapshot.getSpread() );

	// add tick to range
	// auto market = manager.getMarket( snapshot.getSymbol() );
	// if ( ! market.isValid() ) {
	// 	return;
	// }
	}

	void RangeStrategy::onAccountChange(FIXManager& manager, Account& account) {
		if ( manager.isExiting() ) {
			return;
		}

		manager.console()->info( "[Strategy:onAccountChange] {}", account.toString() );
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
			manager.console()->warn( "[Strategy:onPositionChange] unknown status" );
			break;
		}

		if ( ! oss.str().empty() ) {
			manager.console()->info( "[Strategy:onPositionChange] {}", oss.str().c_str() );
		}
	}

	void RangeStrategy::onExit(FIXManager& manager) {
		manager.console()->info( "[Strategy:onExit]" );
	}

	void RangeStrategy::onError(FIXManager& manager, std::string origin, std::string message) {
		manager.console()->error( "[Strategy:onError] origin: {} message: {}", origin, message );
	}

	void RangeStrategy::onRequestAck(FIXManager& manager, std::string request_ident, std::string text) {
		manager.console()->warn( "[Strategy:onRequestAck] ident: {} message: {}", request_ident, text );
	}

/*!
 * Add moving average value to list and renko close price to list
 * 
 * @param const RenkoBrick& brick Reference to last renko brick
 */
void RangeStrategy::addMovingAverageValue(const RenkoBrick& brick) {
	try {
		FIX::Locker lock( m_mutex );

		// the price list
		std::vector<double> price_list;

		// add close price to list
		auto it1 = m_renko_close_prices.find( brick.symbol );
		if ( it1 != m_renko_close_prices.end() ) {
  			// found list, add value
			it1->second.push_back( brick.close_price );

			price_list = it1->second;
		} else {
  			// list not found for symbol, add list
			std::vector<double> list{brick.close_price};
  			// add list
			m_renko_close_prices.insert( std::pair<std::string, std::vector<double> >( brick.symbol, list ) );

			price_list = list;
		}

		// calculate moving average for close price
		auto ma_value = Math::get_moving_average( price_list, 3 );

		auto it2 = m_moving_average.find( brick.symbol );
		if ( it2 != m_moving_average.end() ) {
  			// found moving average list for symbol
  			// add value to list
			it2->second.push_back( ma_value );
		} else {
  			// no list found, create one
			std::vector<double> list{ma_value};
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


};