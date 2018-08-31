#ifndef IDEFIX_MATH_H
#define IDEFIX_MATH_H

#include "MarketSnapshot.h"
#include "MarketOrder.h"
#include <quickfix/Field.h>
#include <cmath>
#include <iterator>
#include <vector>

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
		 * @param const char				side 			 The side of the current position.
		 * @return double
		 */
		inline double get_pip_value(const MarketSnapshot snapshot, const double pos_qty, const std::string account_currency = "USD", const double conversion_price = 0, const char side = FIX::Side_BUY) {
			
			// Whatever currency the account is, when that currency is listed second in a pair the pip values are fixed. 
			// For example, if you have a Canadian dollar (CAD) account, any pair that is XXX/CAD, such as the USD/CAD 
			// will have a fixed pip value. A standard lot is CAD$10, a mini lot is CAD$1, and a micro lot is CAD$0.10.
			double pip_v = ( snapshot.getPointSize() * pos_qty );

			// if the account currency is USD
			if ( account_currency == "USD" ) {
				// if the USD isn't listed second,
				// divide the pip value by the USD/xxx rate
				if ( snapshot.getBaseCurrency() == "USD" ) {
					pip_v /= ( side == FIX::Side_BUY ? snapshot.getAsk() : snapshot.getBid() );
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
		 * @param const double            pip_value (DEPRECATED) The pip value for this symbol
		 * @param const& MarketSnapshot   snapshot  The current price data for the position
		 * @param const& MarketOrder      position  The position to calculate
		 * @return double
		 */
		inline double get_profit_loss(const double pip_value, const MarketSnapshot& snapshot, const MarketOrder& position) {
			double open  = position.getPrice();
			double close = 0;
			int side     = ( position.getSide() == FIX::Side_BUY ? 1 : 0 );
			if ( side == 1 ) {
				close = snapshot.getBid(); // because we want to sell for closing
			} else {
				close = snapshot.getAsk(); // because we want to buy for closing
			}

			// calculate profit/loss in points
			double pips_moved   = std::abs( close - open ) / snapshot.getPointSize();
			// calculate point profit
			double point_profit = position.getQty() * snapshot.getPointSize();
			// calculate profit/loss
			double pnl          = pips_moved * point_profit;

			// check if this is a loss
			if ( ( side == 1 && close < open ) || ( side == 0 && close > open ) ) {
				pnl *= -1;
			}
			
			return pnl;
		} // END get_profit_loss

		/*!
		 * Calculate Equity
		 * 
		 * @param const double balance    The account balance.
		 * @param const double profitloss The current profit loss of a position
		 * @return double
		 */
		inline double get_equity(const double balance, const double profitloss) {
			if ( profitloss == 0 ) {
				return balance;
			}

			double equity = balance + profitloss;
			
			return equity;
		}

		/*!
		 * Calculate Free Margin
		 * 
		 * @param const double balance         The account balance.
		 * @param const double equity          The calculated equity (get_equity).
		 * @param const double required_margin The required margin actually in use.
		 * @return double
		 */
		inline double get_free_margin(const double balance, const double equity, const double required_margin) {
			// no open positions, balance value is free margin
			if ( equity == 0 && required_margin == 0 ) {
				return balance;
			}

			double free_margin = equity - required_margin;

			return free_margin;
		}

		/*!
		 * Calculate Margin Ratio
		 * 
		 * @param const double equity The account equity.
		 * @param const double margin The required margin actually in use.
		 * @return double
		 */
		inline double get_margin_ratio(const double equity, const double margin) {
			// check if margin is 0
			if ( margin == 0 ) {
				return 0;
			}

			double margin_ratio = ( equity / margin ) * 100;

			return margin_ratio;
		}

		/*!
		 * Calculate unit size based on pip distance, account currency and risk in percent of free margin
		 * 
		 * @param const double  free_margin      The free margin.
		 * @param const double  risk             Risk in percent of free margin. 
		 * @param const double  pip_risk         Risk in pips e.g. the distance between entry and stoploss.
		 * @param const double  conversion_price The conversion price for non USD accounts. Defaults to 0.
		 * @param const double  point_size       The point size of the currency pair. Defaults to 0.0001
		 * @param const double  contract_size    The contract size of the asset. Defaults to 100000
		 * @return double
		 */
		inline double get_unit_size(const double free_margin, const double risk, const double pip_risk, const double conversion_price = 0, const double point_size = 0.0001, const double contract_size = 100000) {
			if ( free_margin <= 0 || risk <= 0 || pip_risk <= 0 ) {
				return 0;
			}

			double result = 0;

			// Using his account balance and the percentage amount he wants to risk, we can calculate the dollar amount risked.
			double risk_in_currency = free_margin * ( risk / 100 );

			// if conversion price is available, convert the risk in currency by the conversion
			if ( conversion_price > 0 ) {				
				risk_in_currency = conversion_price * risk_in_currency;
			}

			// Next, we divide the amount risked by the stop to find the value per pip.
			const double value_per_pip = risk_in_currency / pip_risk;
			// Lastly, we multiply the value per pip by a known unit/pip value ratio of currency pair. 
			// In this case, with 100k units (or one lot), each pip move is worth USD 10.
			result = value_per_pip * ( contract_size / ( contract_size * point_size ) );

			return result;
		}

		/*!
		 * Get amount at risk in account currency
		 * 
		 * @param const double  free_margin The current free margin.
		 * @param const double  risk        The risk in percent.
		 * @return double
		 */
		inline double get_amount_at_risk(const double free_margin, const double risk) {
			const double result = free_margin * ( risk / 100 );
			return result;
		}

		/*!
		 * Get conversion price based on account currency and symbol
		 * 
		 * @param const double         price                            The current price bid or ask
		 * @param const std::string    account_currency                 The account currency
		 * @param const std:string     conversion_symbol_quote_currency The quote currency of the conversion price
		 * @return double
		 */
		inline double get_conversion_price(const double price, const std::string account_currency, const std::string conversion_symbol_quote_currency) {
			double result = 0;
			
			if ( account_currency == "EUR" || ( account_currency == "USD" && conversion_symbol_quote_currency != "USD" ) ) {
				result = price / 1;
			}

			return result;
		}

		/*!
		 * Check if the timestamp hits the period
		 * 
		 * @param const std::string&  timestamp Timestamp in format yyyymmdd-H:i:s.u
		 * @param const int           period    Period in seconds, e.g. 60 = 1 Minute
		 * @return bool
		 */
		inline bool is_period_hit(const std::string& timestamp, const int period) {
			// convert string to UtcTimeStamp
			FIX::UtcTimeStamp nowTS = FIX::UtcTimeStampConvertor::convert( timestamp );

			// seconds 0 - 59
			if ( period < 60 && ( nowTS.getSecond() % period == 0 ) ) {
				return true;
			}
			// minutes 0 - 59
			else if ( ( period > 60 && period < 3540 ) && nowTS.getSecond() == 0 && ( nowTS.getMinute() % ( period / 60 ) == 0 ) ) {
				return true;
			} 
			// hours 0 - 23
			else if ( ( period > 3540 && period < 82800 ) && nowTS.getMinute() == 0 && nowTS.getSecond() == 0 && ( nowTS.getHour() % ( period / 3600 ) == 0 ) ) {
				return true;
			}

			return false;
		}

		/*!
		 * Calculate the spread between two prices
		 * 
		 * @param const double bid_price  
		 * @param const double ask_price  
		 * @param const double point_size 
		 * @return double returns -1 on error
		 */
		inline double get_spread(const double bid_price, const double ask_price, const double point_size) {
			if ( point_size == 0 ) return -1;
			return std::abs( ( ask_price - bid_price ) * ( 1 / point_size ) );
		}

		/*!
		 * Calculate moving average value
		 * 
		 * @param const std::vector<double>  value_list  The values list
		 * @param const unsigned int         period     How many entries to calculate?
		 * @param const unsigned int         offset     Begin by offset entry
		 * @return double
		 */
		// inline double get_moving_average(const std::vector<double> value_list, const unsigned int period, const unsigned int offset = 0) {
		// 	// check bounds
		// 	if ( value_list.empty() || ( period + offset ) > value_list.size() ) {
		// 		return 0;
		// 	}
		// 	// the sum
		// 	double sum;
		// 	// get start iterator from offset
		// 	auto it = std::next( value_list.rbegin(), offset );
		// 	// cycle through periods
		// 	for ( int i = 0; i < period; i++ ) {
		// 		// forward iterator with period starting from offset
		// 		auto vi = std::next( it, i );
		// 		// add value to sum
		// 		sum += *vi;
		// 	}
		// 	// calculate moving average
		// 	double avg = sum / period;			
		// 	// return value
		// 	return avg;
		// }
	}; // END NS MATH
}; // END NS IDEFIX

#endif