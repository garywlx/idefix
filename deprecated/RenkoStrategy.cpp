#include "RenkoStrategy.h"
#include "indicator/Renko.h"

namespace IDEFIX {
	// Constructor
	RenkoStrategy::RenkoStrategy(): m_period( 1 ) {
		// subscribe to markets, AUD/USD, EUR/USD, GBP/USD, NZD/USD, USD/CAD, USD/CHF
		// EUR/USD
		MarketWatcher eurusd = {"EUR/USD"};
		m_market_watcher.push_back( eurusd );
	}

	// return the strategy identifier, this musst be unique
	std::string RenkoStrategy::getName() const {
		return "renko_strategy";
	}

	// Is called when FIX Manager is ready to initialize the strategy.
	void RenkoStrategy::onInit(FIXManager& manager) {
		manager.console()->info( "[{}:onInit]", getName() );

		for ( auto& mw : m_market_watcher ) {
			
			// subscribe to market data
			manager.subscribeMarketData( mw.symbol );
		}
	}

	// Is called when a new market snapshot is available.
	void RenkoStrategy::onTick(FIXManager& manager, const MarketSnapshot& snapshot) {
		
	}

	// Is called when the account changed
	void RenkoStrategy::onAccountChange(FIXManager& manager, Account& account) {

	}

	// Is called when a position changed
	void RenkoStrategy::onPositionChange(FIXManager& manager, MarketOrder& position, MarketOrder::Status status) {

	}

	// Is called when the FIX Manager session exits.
	void RenkoStrategy::onExit(FIXManager& manager) {
		// unsubscribe markets
		for ( auto& mw : m_market_watcher ) {
			manager.unsubscribeMarketData( mw.symbol );
		}
	}

	// Is called if an error happens.
	void RenkoStrategy::onError(FIXManager& manager, std::string origin, std::string message) {

	}

	// Is called if an api requests has no data.
	void RenkoStrategy::onRequestAck(FIXManager& manager, std::string request_ident, std::string text) {

	}

};