#include "RangeStrategy.h"
#include "indicator/RenkoIndicator.h"
#include "indicator/MovingAverage.h"
#include <utility>
#include <cmath>

namespace IDEFIX {
// ---------------------------------------------------------------------------------------
// RangeStrategy
// ---------------------------------------------------------------------------------------
RangeStrategy::RangeStrategy() {

}

RangeStrategy::~RangeStrategy() {}

std::string RangeStrategy::getIdentifier() const {
	return "RangeStrategy";
}

void RangeStrategy::onInit(FIXManager& manager) {
	manager.console()->info( "[Strategy:onInit] {}", getIdentifier() );
	// add range indicator for 3 pip range on bid price
	manager.addIndicator( "EUR/USD", new RenkoIndicator( 1.5, RenkoIndicator::PriceType::BID_PRICE ) );
	// add moving average for close price on renko
	manager.addIndicator( "EUR/USD", new MovingAverage() );

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
	auto market = manager.getMarket( snapshot.getSymbol() );
	if ( ! market.isValid() ) {
		return;
	}

	// check if range is big enough
	// if ( market.getSize() > 2 ) {
	// 	// get last 3 elements
	// 	FIX::Locker lock( m_mutex );
	// 	auto ticks = market.getRange( -3 );
	// 	if ( ! ticks.empty() ) {
	// 		for ( int i = 0; i < 3; i++ ) {
	// 			manager.console()->warn( 
	// 				"[Strategy:onTick] {} #{:d} bid@{:.5f} ask@{:.5f} spread@{:.2f}", 
	// 				ticks.at( i ).getSymbol(),
	// 				i,
	// 				ticks.at( i ).getBid(),
	// 				ticks.at( i ).getAsk(),
	// 				ticks.at( i ).getSpread()
	// 			);
	// 		}
	// 	}
	// } // if market.getSize() > 2
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

};