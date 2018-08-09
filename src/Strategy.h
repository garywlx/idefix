#ifndef IDEFIX_STRATEGY_H
#define IDEFIX_STRATEGY_H

#include <string>
#include "FIXManager.h"

namespace IDEFIX {
	class Strategy {
	public:
		// return the strategy identifier, this musst be unique
		virtual std::string getIdentifier() const = 0;
		// Is called when FIX Manager is ready to initialize the strategy.
		virtual void onInit(FIXManager& manager) = 0;
		// Is called when a new market snapshot is available.
		virtual void onTick(FIXManager& manager, const MarketSnapshot& snapshot) = 0;
		// Is called when the account changed
		virtual void onAccountChange(FIXManager& manager, Account& account) = 0;
		// Is called when a position changed
		virtual void onPositionChange(FIXManager& manager, MarketOrder& position, MarketOrder::Status status) = 0;
		// Is called when the FIX Manager session exits.
		virtual void onExit(FIXManager& manager) = 0;
		// Is called if an error happens.
		virtual void onError(FIXManager& manager, std::string origin, std::string message) = 0;
		// Is called if an api requests has no data.
		virtual void onRequestAck(FIXManager& manager, std::string request_ident, std::string text) = 0;
	};
};

#endif