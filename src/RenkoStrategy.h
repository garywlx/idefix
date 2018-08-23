#ifndef IDEFIX_RENKO_STRATEGY_H
#define IDEFIX_RENKO_STRATEGY_H

#include "Strategy.h"

namespace IDEFIX {
	struct MarketWatcher {
		std::string symbol;
	};

	class RenkoStrategy: public Strategy {
	private:
		int m_period;
		std::vector<MarketWatcher> m_market_watcher;

	public:
		RenkoStrategy();

		// return the strategy identifier, this musst be unique
		std::string getName() const;
		// Is called when FIX Manager is ready to initialize the strategy.
		void onInit(FIXManager& manager);
		// Is called when a new market snapshot is available.
		void onTick(FIXManager& manager, const MarketSnapshot& snapshot);
		// Is called when the account changed
		void onAccountChange(FIXManager& manager, Account& account);
		// Is called when a position changed
		void onPositionChange(FIXManager& manager, MarketOrder& position, MarketOrder::Status status);
		// Is called when the FIX Manager session exits.
		void onExit(FIXManager& manager);
		// Is called if an error happens.
		void onError(FIXManager& manager, std::string origin, std::string message);
		// Is called if an api requests has no data.
		void onRequestAck(FIXManager& manager, std::string request_ident, std::string text);
	};
};

#endif