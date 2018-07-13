#ifndef IDEFIX_TRADEMATH_H
#define IDEFIX_TRADEMATH_H

#include <cmath>
#include <string>
#include "string_extensions.h"

namespace IDEFIX {
struct TradeMath {
	/*!
	 * Return decimal multiplicator for price calculations
	 * @param const unsigned int precision 
	 * @return 0 if precision > 7
	 */
	static unsigned int decimalMultiplicator(const unsigned int precision){
		const unsigned int decimals[] = {0, 1, 10, 100, 1000, 10000, 100000};
		
		if( precision > 7 ){
			return 0;
		}

		return decimals[precision];
	}

	/*!
	 * Return profit/loss value
	 * https://www.oanda.com/lang/de/forex-trading/analysis/profit-calculator/
	 * https://www.oanda.com/lang/de/forex-trading/analysis/profit-calculator/how
	 * @param const double lastPx or currentPx
	 * @param const double entryPx
	 * @param const double qty
	 * @param const std::string& symbol
	 * @return double in USD
	 */
	static double getPnL(const double lastPx, const double entryPx, const double qty, const std::string& symbol){

		double mulPx = lastPx;

		// if quote currency is not USD, convert pnl back to usd
		std::string splitStr = std::move( symbol );
		auto symEx = str::explode(splitStr, '/');
		if( symEx.size() > 0 && symEx[1] != "USD" ) {
			mulPx = 1 / mulPx;
		}

		// Formula
		// (Schlusskurs - Eröffnungskurs) * ({Angebotswährung}/{Standardwährung}) * Units
		double pnl = (lastPx - entryPx) * mulPx * qty;		

		return pnl;
	}
};
};
#endif