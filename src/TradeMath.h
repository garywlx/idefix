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
	 * Return unsigned profit/loss value
	 * Formula from Andre Schulte
	 * 
	 * @param const double currentPx  The current market last price (for SELL use Bid, for BUY use ASK)
	 * @param const double entryPx    The position entry price.
	 * @param const double point_size The symbol point size.
	 * @param const double qty        The qty in lot.
	 * @param const double pip_value  The value of one pip for the traded currency
	 * @param const Side   side       The side of the position. Defaults to TradeMath::Side::BUY
	 * @return double
	 */
	static double getProfitLoss(const double currentPx, const double entryPx, const double point_size, const double qty, const double pip_value, const TradeMath::Side side = TradeMath::Side::BUY) {		
		double pnlV = 0;
		double pipDiff = 0;

		if ( side == TradeMath::Side::BUY ) {
			pipDiff = ( currentPx - entryPx );
		} else if ( side == TradeMath::Side::SELL ) {
			pipDiff = ( entryPx - currentPx );
		}

		pnlV = ( pipDiff * pip_value * 100 ) * ( qty / 100000 );

		return pnlV;
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
		double pipValue = ( point_size * lot_size ) / symbol_price;
		return pipValue;
	}
};
};
#endif