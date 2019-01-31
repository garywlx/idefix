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

};