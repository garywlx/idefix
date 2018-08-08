#ifndef IDEFIX_RANGESTRATEGY_H
#define IDEFIX_RANGESTRATEGY_H

#include "Strategy.h"

namespace IDEFIX {
class RangeStrategy: public Strategy {
private:

public:
	RangeStrategy();
	~RangeStrategy();
	
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