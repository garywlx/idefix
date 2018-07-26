#ifndef IDEFIX_PAIRS_H
#define IDEFIX_PAIRS_H

#include <string>
#include <map>
#include "String.h"

namespace IDEFIX {
	/*!
	 * Get counter pair of symbol. If no symbol is found, returns symbol param value
	 * @param const std::string symbol
	 * @param const std::string account_currency
	 * @return std::string
	 */
	inline std::string getCounterPair(const std::string symbol, const std::string account_currency) {

		// get symbol parts
		auto split = str::explode( symbol, '/' );

		if ( account_currency == "EUR" ) {
			// if quote is USD, return EUR/USD
			if ( split[1] == "USD" ) {
				return "EUR/USD";
			}
			// else return counter pair
			else {
				if ( symbol == "USD/CAD" ) {
					return "EUR/CAD";
				} else if ( symbol == "USD/CHF" ) {
					return "EUR/CHF";
				} else if ( symbol == "USD/JPY" ) {
					return "EUR/JPY";
				}
			}
		} 

		// as default
		return "EUR/USD";
	}
};

#endif