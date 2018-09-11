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
#define STRATEGY_AVG_OFFSET 0
// Renko Brick Size, default is 0.5
#define STRATEGY_RENKO_BRICK_SIZE 1.5

namespace IDEFIX {
// Class
class RangeStrategy: public Strategy {
private:
	FIX::Mutex m_mutex;
	// hold all close prices per symbol
	std::map<std::string, std::vector<RenkoBrick> > m_renko_bricks;
	// hold moving averages for symbols ma[symbol][] = double
	std::map<std::string, std::vector<double> > m_moving_average;

	// hold all symbols to trade
	std::vector<std::string> m_symbols;

	int m_open_sell;
	int m_open_buy;
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
	void addListValues(const RenkoBrick& brick);
	double getLastMovingAverage(const std::string symbol);
	double getLastRenkoPrice(const std::string symbol);
	double getMovingAverage(const std::string symbol, const int period);
	double getRenkoPrice(const std::string symbol, const int period);
	void getRenkoBrick(const std::string symbol, const int period, RenkoBrick& brick);
	void initIndicator(FIXManager& manager, const std::string symbol);
	int getRenkoBrickCount(const std::string symbol);
};
};

#endif