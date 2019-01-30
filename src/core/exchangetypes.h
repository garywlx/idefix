#pragma once

#include <string>
#include <unordered_map>

namespace idefix {

typedef std::unordered_map<std::string, std::string> ExchangeSettingsMap;
typedef std::unordered_map<std::string, std::string> ExchangeCollateralSettingsMap;

struct ExchangePositionReport {
	std::string account_id;
	std::string symbol;
	std::string position_id;
	std::string pos_open_time;
};

struct ExchangeTick {
	std::string symbol;
	std::string timestamp;
	double bid;
	double ask;
	double session_high;
	double session_low;
};

enum ExchangeOrderEvent {
    MARKET_ORDER_SET_SL,
    MARKET_ORDER_SET_TP,
    MARKET_ORDER_CANCELED,
    MARKET_ORDER_REJECT,
    MARKET_ORDER_NEW,
    MARKET_ORDER_TP_HIT,
    MARKET_ORDER_SL_HIT
};

};