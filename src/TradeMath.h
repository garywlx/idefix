#ifndef IDEFIX_TRADEMATH_H
#define IDEFIX_TRADEMATH_H

#include <cmath>
#include <string>

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
	 * Return int precision value for given symbol
	 * @param  symbol [description]
	 * @return        [description]
	 */
	static unsigned int getPrecision(const std::string symbol){
		if( symbol == "EUR/USD" ){
			return 5;
		}

		return 0;
	}
};
};
#endif