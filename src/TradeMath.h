#ifndef IDEFIX_TRADEMATH_H
#define IDEFIX_TRADEMATH_H

#include <cmath>

namespace IDEFIX {
struct TradeMath {
	/*!
	 * Return profit/loss value (FINAL)
	 * https://www.kagels-trading.de/pip-wert-berechnung-im-forex-handel/
	 * 
	 * @param const double lastPx     The current market last price (for SELL use Bid, for BUY use ASK)
	 * @param const double entryPx    The position entry price.
	 * @param const double point_size The symbol point size.
	 * @param const double qty        The qty in lot.
	 * @return double
	 */
	static double getProfitLoss(const double lastPx, const double entryPx, const double point_size, const double qty) {
		double price_diff = ( lastPx - entryPx ) * ( 1 / point_size ); 
		double pip_value = qty * point_size;
		double pnl = price_diff * pip_value;

		return pnl;
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
		// https://www.xm.com/de/forex-calculators/pip-value
		// https://www.kagels-trading.de/pip-wert-berechnung-im-forex-handel/
		double pipValue = ( point_size / symbol_price ) * lot_size;
		return pipValue;
	}
};
};
#endif