#ifndef IDEFIX_MATH_H
#define IDEFIX_MATH_H

#include "MarketSnapshot.h"
#include "MarketOrder.h"

namespace IDEFIX {
	namespace Math {
		/*!
		 * Pip Value calculation for quote currency
		 * https://www.myfxbook.com/forex-calculators/pip-calculator
		 * https://www.thebalance.com/calculating-pip-value-in-forex-pairs-1031022
		 * 
		 * @param const MarketSnapshot      snapshot         The current market snapshot.
		 * @param const double              pos_qty          The current position qty.
		 * @param const std::string         account_currency The account currency in capital letters. Default is USD
		 * @param const double              conversion_price The conversion_price for snapshot if it does not contain account_currency. eg. ACC=USD, Snapshot=EUR/GBP => USD/GBP
		 * @return double
		 */
		inline double get_pip_value(const MarketSnapshot snapshot, const double pos_qty, const std::string account_currency = "USD", const double conversion_price = 0) {
			
			// Whatever currency the account is, when that currency is listed second in a pair the pip values are fixed. 
			// For example, if you have a Canadian dollar (CAD) account, any pair that is XXX/CAD, such as the USD/CAD 
			// will have a fixed pip value. A standard lot is CAD$10, a mini lot is CAD$1, and a micro lot is CAD$0.10.
			double pip_v = ( snapshot.getPointSize() * pos_qty );

			// if the account currency is USD
			if ( account_currency == "USD" ) {
				// if the USD isn't listed second,
				// divide the pip value by the USD/xxx rate
				if ( snapshot.getBaseCurrency() == "USD" ) {
					pip_v /= snapshot.getAsk();	
				}

				// if the snapshot does not contain account currency
				// e.g. Account USD && Symbol EUR/CHF
				if ( snapshot.getBaseCurrency() != account_currency && snapshot.getQuoteCurrency() != account_currency ) {
					// convert with conversion price
					if ( conversion_price > 0 ) {
						pip_v /= conversion_price;
					}
				}
			}
			// account currency is EUR
			else if ( account_currency == "EUR" ) {
				// if symbol base equals account currency: EUR/... in EUR account
				if ( snapshot.getBaseCurrency() == account_currency ) {
					pip_v /= snapshot.getAsk();
				}
				// if ( base or quote == USD ) && conversion_price > 0
				// USD/... or .../USD
				else if ( ( snapshot.getBaseCurrency() == "USD" || snapshot.getQuoteCurrency() == "USD" ) && conversion_price > 0 ) {
					pip_v /= conversion_price;
				}
			}

			return pip_v;
		} // END get_pip_value

		/*!
		 * Calculate Profit / Loss based on pip_value
		 * 
		 * @param const double           pip_value The pip value for this symbol
		 * @param const MarketSnapshot   snapshot  The current price data for the position
		 * @param const MarketOrder      position  The position to calculate
		 * @return double
		 */
		inline double get_profit_loss(const double pip_value, const MarketSnapshot snapshot, const MarketOrder position) {
			double pip_diff_decimal = 0;
			double pnl = 0;

			if ( position.getSide() == FIX::Side_BUY ) {
				pip_diff_decimal = snapshot.getBid() - position.getPrice();
			} else {
				pip_diff_decimal = position.getPrice() - snapshot.getBid();
			}

			double pips = pip_diff_decimal / snapshot.getPointSize();

			pnl = ( ( pips * ( position.getQty() / snapshot.getContractSize() ) ) * pip_value );

			return pnl;
		} // END get_profit_loss
	};
};

#endif