#ifndef IDEFIX_RANGESTRATEGY_H
#define IDEFIX_RANGESTRATEGY_H

#include "Strategy.h"
#include "indicator/RenkoBrick.h"
#include <map>
#include <vector>
#include <quickfix/Mutex.h>

// Moving Average Period, default is 3
#define STRATEGY_AVG_PERIOD 3
// Moving Average Offset, default is 0
#define STRATEGY_AVG_OFFSET 1
// Renko Brick Size, default is 0.5
#define STRATEGY_RENKO_BRICK_SZE 0.5

namespace IDEFIX {
// Class
class RangeStrategy: public Strategy {
private:
	FIX::Mutex m_mutex;
	// hold all close prices per symbol
	std::map<std::string, std::vector<double> > m_renko_close_prices;
	// hold moving averages for symbols ma[symbol][] = double
	std::map<std::string, std::vector<double> > m_moving_average;

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

private:
	void addMovingAverageValue(const RenkoBrick& brick);
	double getLastMovingAverage(const std::string symbol);
	double getLastRenkoPrice(const std::string symbol);
	void initIndicator(FIXManager& manager, const std::string symbol);
};
};

#endif