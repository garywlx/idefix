#ifndef IDEFIX_PAIRS_H
#define IDEFIX_PAIRS_H

#include <string>
#include <map>

namespace IDEFIX {
	/*!
	 * Get counter pair of symbol. If no symbol is found, returns symbol param value
	 * @param const std::string symbol
	 * @return std::string
	 */
	inline std::string getCounterPair(const std::string symbol) {

		typedef std::pair<std::string, std::string> pair;

		std::map<std::string, std::string> counterPairs;
		counterPairs.insert( pair("EUR/GBP", "GBP/USD") );
		counterPairs.insert( pair("EUR/JPY", "USD/JPY") );
		counterPairs.insert( pair("EUR/CHF", "USD/CHF") );
		counterPairs.insert( pair("EUR/AUD", "AUD/USD") );
		counterPairs.insert( pair("EUR/CAD", "USD/CAD") );
		counterPairs.insert( pair("EUR/NZD", "NZD/USD") );
		counterPairs.insert( pair("AUD/CAD", "USD/CAD") );
		counterPairs.insert( pair("AUD/CHF", "USD/CHF") );
		counterPairs.insert( pair("AUD/JPY", "USD/JPY") );
		counterPairs.insert( pair("AUD/NZD", "NZD/USD") );
		counterPairs.insert( pair("CAD/CHF", "USD/CHF") );
		counterPairs.insert( pair("CAD/JPY", "USD/JPY") );
		counterPairs.insert( pair("CHF/JPY", "USD/JPY") );
		counterPairs.insert( pair("GBP/AUD", "AUD/USD") );
		counterPairs.insert( pair("GBP/CAD", "USD/CAD") );
		counterPairs.insert( pair("GBP/CHF", "USD/CHF") );
		counterPairs.insert( pair("GBP/JPY", "USD/JPY") );
		counterPairs.insert( pair("GBP/NZD", "NZD/USD") );
		counterPairs.insert( pair("NZD/CAD", "USD/CAD") );
		counterPairs.insert( pair("NZD/CHF", "USD/CHF") );
		counterPairs.insert( pair("NZD/JPY", "USD/JPY") );

		auto iterator = counterPairs.find( symbol );
		if ( iterator != counterPairs.end() ) {
			return iterator->second;
		}

		return std::move( symbol );
	}
};

#endif