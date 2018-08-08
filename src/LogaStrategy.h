#ifndef IDEFIX_LOGASTRATEGY_H
#define IDEFIX_LOGASTRATEGY_H

#include "Strategy.h"

namespace IDEFIX {
	class LogaStrategy: public Strategy {
	public:
		LogaStrategy();
		~LogaStrategy();

		std::string getIdentifier() const;
		void onInit(FIXManager& manager);
		void onTick(FIXManager& manager, const MarketSnapshot& snapshot);
		void onAccountChange(FIXManager& manager, Account& account);
		void onPositionChange(FIXManager& manager, MarketOrder& position, MarketOrder::Status status);
		void onExit(FIXManager& manager);
		void onError(FIXManager& manager, std::string origin, std::string message);
		void onRequestAck(FIXManager& manager, std::string request_ident, std::string text);

	};
};

#endif