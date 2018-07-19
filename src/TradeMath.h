#ifndef IDEFIX_TRADEMATH_H
#define IDEFIX_TRADEMATH_H

#include <iostream>
#include <cmath>

using namespace std;

namespace IDEFIX {
struct TradeMath {

	enum Side {
		BUY,
		SELL
	};

	/*!
	 * Return unsigned profit/loss value (FINAL)
	 * https://www.kagels-trading.de/pip-wert-berechnung-im-forex-handel/
	 * https://www.ifcmarkets.de/introduction/profit-loss-calculation
	 * 
	 * @param const double currentPx  The current market last price (for SELL use Bid, for BUY use ASK)
	 * @param const double entryPx    The position entry price.
	 * @param const double point_size The symbol point size.
	 * @param const double qty        The qty in lot.
	 * @param const Side   side       The side of the position. Defaults to TradeMath::Side::BUY
	 * @return double
	 */
	static double getProfitLoss(const double currentPx, const double entryPx, const double point_size, const double qty, const TradeMath::Side side = TradeMath::Side::BUY) {
		double pip_diff = TradeMath::getPipDiff(currentPx, entryPx, point_size);
		double quote_pnl = ( pip_diff * ( qty * point_size ) );

		if ( ( side == TradeMath::Side::BUY && currentPx < entryPx ) || ( side == TradeMath::Side::SELL && currentPx > entryPx ) ) {
        	// loss
        	quote_pnl *= -1;
      	}

		return quote_pnl;
	}

	/*!
	 * Return value for one pip
	 * 
	 * @param const double point_size   The point size of the symbol, e.g. 0.0001.
	 * @param const double symbol_price The price for the symbol.
	 * @param const double lot_size     The size of one lot. e.g. 100000.
	 * @return double
	 */
	static double getPipValue(const double point_size, const double symbol_price, const double lot_size) {
		// Pip-Wert = (Ein Pip / Umrechnungskurs) * Lot-Größe
		// https://www.kagels-trading.de/pip-wert-berechnung-im-forex-handel/
		// https://www.day-trading.de/forex/grundlagen/kursdarstellung.html
		double pipValue = ( point_size / symbol_price ) * lot_size;
		return pipValue;
	}

	/*!
	 * Return pip difference between two prices
	 * 
	 * @param const double leftPx
	 * @param const doulbe rightPx
	 * @param const double point_size Defaults to 0.0001.
	 * @return double
	 */
	static double getPipDiff(const double leftPx, const double rightPx, const double point_size = 0.0001) {
		return std::abs( leftPx - rightPx ) * ( 1 / point_size );
	}
};
};
#endif