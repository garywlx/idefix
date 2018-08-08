#include "LogaStrategy.h"

namespace IDEFIX {

	LogaStrategy::LogaStrategy() {}

	LogaStrategy::~LogaStrategy() {}

	std::string LogaStrategy::getIdentifier() const {
		return "LogaStrategy";
	}
	
	/*!
	 * Subscribe to MarketData and do initialization stuff
	 * 
	 * @param FIXManager& manager The reference to FIXManager
	 */
	void LogaStrategy::onInit(FIXManager& manager) {
 		cout << "[onInit]" << endl;
 		// subscribe market data
 		manager.subscribeMarketData( "EUR/USD" );

 		// subscribe candle period 30 seconds
 		manager.addCandlePeriod( "EUR/USD", 10 );
	}
	
	/*!
	 * Is called on every tick
	 * 
	 * @param FIXManager&           manager   The reference to FIXManager
	 * @param const MarketSnapshot& snapshot  The snapshot
	 */
	void LogaStrategy::onTick(FIXManager& manager, const MarketSnapshot &snapshot) {
		/*cout << "[onTick]" << endl;
		cout << snapshot << endl; */
	}

	void LogaStrategy::onAccountChange(FIXManager& manager, Account& account) {
		cout << "[onAccountChange]" << endl;
		cout << account << endl;
	}

	void LogaStrategy::onPositionChange(FIXManager& manager, MarketOrder& position, MarketOrder::Status status) {
		cout << "[onPositionChange] ";

		switch( status ) {
			case MarketOrder::Status::NEW:
				cout << " NEW " << position.getPosID() << endl;
				break;
			case MarketOrder::Status::FILLED:
				cout << " FILLED " << position.getPosID() << endl;
				break;
			case MarketOrder::Status::STOPPED:
				cout << " STOPPED " << position.getPosID() << endl;
				break;
			case MarketOrder::Status::REMOVED:
				cout << " REMOVED " << position.getPosID() << endl;
				break;
			case MarketOrder::Status::UPDATE:
				cout << " UPDATE " << position.getPosID() << endl;
				break;
		}
	}

	void LogaStrategy::onExit(FIXManager& manager) {
		cout << "[onExit]" << endl;
	}

	void LogaStrategy::onError(FIXManager& manager, std::string origin, std::string message) {
		cout << "[onError:" << origin << "] " << message << endl;
	}

	void LogaStrategy::onRequestAck(FIXManager& manager, std::string request_ident, std::string text) {
		cout << "[" << request_ident << "] " << text << endl;
	}

};