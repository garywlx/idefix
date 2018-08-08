#include "RangeStrategy.h"

IDEFIX::RangeStrategy::RangeStrategy() {

}

IDEFIX::RangeStrategy::~RangeStrategy() {

}

std::string IDEFIX::RangeStrategy::getIdentifier() const {
	return "RangeStrategy";
}

void IDEFIX::RangeStrategy::onInit(FIXManager& manager) {
	manager.console()->info( "[Strategy:onInit] {}", getIdentifier() );
	// subscribe market data
	manager.subscribeMarketData( "AUD/USD" );
	manager.subscribeMarketData( "EUR/USD" );
	// manager.subscribeMarketData( "GBP/USD" );
	// manager.subscribeMarketData( "NZD/USD" );
	// manager.subscribeMarketData( "USD/CAD" );
	// manager.subscribeMarketData( "USD/CHF" );
	// manager.subscribeMarketData( "USD/JPY" );
}

void IDEFIX::RangeStrategy::onTick(FIXManager& manager, const MarketSnapshot& snapshot) {
	if ( manager.isExiting() ) {
		return;
	}

	manager.console()->info( "[Strategy:onTick] {}", snapshot.toString() );
}

void IDEFIX::RangeStrategy::onAccountChange(FIXManager& manager, Account& account) {
	if ( manager.isExiting() ) {
		return;
	}

	manager.console()->info( "[Strategy:onAccountChange] {}", account.toString() );
}

void IDEFIX::RangeStrategy::onPositionChange(FIXManager& manager, MarketOrder& position, MarketOrder::Status status) {
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

void IDEFIX::RangeStrategy::onExit(FIXManager& manager) {
	manager.console()->info( "[Strategy:onExit]" );
}

void IDEFIX::RangeStrategy::onError(FIXManager& manager, std::string origin, std::string message) {
	manager.console()->error( "[Strategy:onError] origin: {} message: {}", origin, message );
}

void IDEFIX::RangeStrategy::onRequestAck(FIXManager& manager, std::string request_ident, std::string text) {
	manager.console()->warn( "[Strategy:onRequestAck] ident: {} message: {}", request_ident, text );
}
