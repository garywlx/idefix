/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */

#include <iostream>
#include <string>
#include <ostream>
#include <iomanip>
#include <cmath>
#include <map>

using namespace std;

enum Side {
	BUY, SELL
};

struct Symbol {
	std::string symbol;
	std::string base_currency;
	std::string quote_currency;
	double bid;
	double ask;
	double point_size;
	int precision;
	double contract_size;
};

struct Position {
	Side side;
	double qty;
	double entry_price;
	double stop_price;
	double limit_price;
};

struct Account {
	std::string currency;
	double equity;
	double margin;
};

std::ostream& operator<<(std::ostream& out, const Symbol& s) {
	double spread = s.ask - s.bid;
	out << "[Symbol] " << endl
		<< " symbol        " << s.symbol << endl
		<< std::setprecision(s.precision) << fixed
		<< " bid           " << s.bid << endl
		<< " ask           " << s.ask << endl
		<< " base      " << s.base_currency << " " << s.bid << endl
		<< " quote     " << s.quote_currency << " " << ( 1 / s.bid ) << endl
		<< " spread        " << ( spread / s.point_size ) << " ( " << spread << " ) " << endl
		<< " point_size    " << s.point_size << endl
		<< " precision     " << s.precision << endl
		<< " contract_size " << s.contract_size << endl;

	return out;
}

std::ostream& operator<<(std::ostream& out, const Position& p) {
	out << "[Position]" << endl
	    << " side          " << ( p.side == Side::BUY ? "BUY" : "SELL" ) << endl
	    << " qty           " << p.qty << endl
	    << " entry_price   " << p.entry_price << endl;

    return out;
}

std::ostream& operator<<(std::ostream& out, const Account& a) {
	out << "[Account]" << endl
		<< " currency      " << a.currency << endl
		<< std::setprecision(2) << fixed
		<< " equity        " << a.equity << " " << a.currency << endl
		<< " margin        " << a.margin << " " << a.currency << endl;

	return out;
}

/*!
 * Pip Value calculation for quote currency
 * https://www.myfxbook.com/forex-calculators/pip-calculator
 * https://www.thebalance.com/calculating-pip-value-in-forex-pairs-1031022
 * 
 * @param const Symbol      snapshot         The current market snapshot.
 * @param const double      pos_qty          The current position qty.
 * @param const std::string account_currency The account currency in capital letters. Default is USD
 * @param const double      conversion_price The conversion_price for snapshot if it does not contain account_currency. eg. ACC=USD, Snapshot=EUR/GBP => USD/GBP
 * @return double
 */
double get_pip_value(const Symbol snapshot, const double pos_qty, const std::string account_currency = "USD", const double conversion_price = 0) {
	
	// Whatever currency the account is, when that currency is listed second in a pair the pip values are fixed. 
	// For example, if you have a Canadian dollar (CAD) account, any pair that is XXX/CAD, such as the USD/CAD 
	// will have a fixed pip value. A standard lot is CAD$10, a mini lot is CAD$1, and a micro lot is CAD$0.10.
	double pip_v = ( snapshot.point_size * pos_qty );

	// if the account currency is USD
	if ( account_currency == "USD" ) {
		// if the USD isn't listed second,
		// divide the pip value by the USD/xxx rate
		if ( snapshot.base_currency == "USD" ) {
			pip_v /= snapshot.ask;	
		}

		// if the snapshot does not contain account currency
		// e.g. Account USD && Symbol EUR/CHF
		if ( snapshot.base_currency != account_currency && snapshot.quote_currency != account_currency ) {
			// convert with conversion price
			if ( conversion_price > 0 ) {
				pip_v /= conversion_price;
			}
		}
	}
	// account currency is EUR
	else if ( account_currency == "EUR" ) {
		// if symbol base equals account currency: EUR/... in EUR account
		if ( snapshot.base_currency == account_currency && snapshot.quote_currency != "USD" ) {
			pip_v /= snapshot.ask;
		}

		// if symbol base equals usd in euro account: USD/... in EUR account
		if ( snapshot.base_currency == "USD" && account_currency == "EUR" && conversion_price > 0 ) {
			pip_v /= conversion_price;
		}
	}

	// if account is denominated in EUR, devide by ask price
	// OR if symbol base is USD and account is USD
	// if ( account_currency == "EUR" || ( snapshot.base_currency == "USD" && account_currency == "USD" ) ) {
	// 	pip_v /= snapshot.ask;
	// }

	return pip_v;
}

/*!
 * Profit/Loss calculation
 * 
 * @param const Symbol      snapshot         The current market snapshot.
 * @param const Position    pos              The current position to calculate.
 * @param const std::string account_currency The account currency in capital letters.
 * @paran const double      conversion_price The price to convert the pnl to. e.g. USD/JPY && account EUR => EURJPY.bid
 * @return double
 */
double get_profit_loss(const Symbol snapshot, const Position pos, const std::string account_currency, const double conversion_price = 0) {

	double pip_diff_decimal = 0;
	double profit_loss = 0;

	// Get price difference in pips in decimal
	if ( pos.side == Side::BUY ) {
		pip_diff_decimal = snapshot.bid - pos.entry_price;
	} else {
		pip_diff_decimal = pos.entry_price - snapshot.bid;
	}
	cout << " pip_diff      " << pip_diff_decimal << endl;

	// Profit&Loss
	// Formula: Profit in Account Currency = ((close price – open price) * Position size / (or *) Currency rate) ± (swap in account currency value *period)
	
	// for USD/JPY
	if ( snapshot.symbol == "USD/JPY" ) {
		profit_loss = ( ( pip_diff_decimal / ( account_currency == "EUR" ? conversion_price : snapshot.bid ) ) * pos.qty );
	} 
	// Account = EUR && GBP/USD
	/*else if ( account_currency == "EUR" && snapshot.quote_currency == "USD" && snapshot.base_currency != "EUR" ) {
		profit_loss = ( ( pip_diff_decimal / conversion_price ) * pos.qty ) / snapshot.bid;
	} */
	// other pairs with USD/... || .../USD
	else {

		// check for quote .../USD
		if ( snapshot.quote_currency == "USD" && snapshot.base_currency != "EUR" ) {
			profit_loss = ( ( pip_diff_decimal / conversion_price ) * pos.qty );
			cout << ".../USD" << endl;
		} 
		// USD/...
		else if ( snapshot.quote_currency != "USD" && snapshot.base_currency == "USD" ) {
			profit_loss = ( pip_diff_decimal * pos.qty );
			cout << "USD/..." << endl;
			cout << pip_diff_decimal << " " << pos.qty << endl;
		}
		else {
			profit_loss = ( pip_diff_decimal * pos.qty );
		}

		// if account is denominated in EUR, divide by currency_rate
		if ( account_currency == "EUR" ) {
			profit_loss /= snapshot.bid;
		}
	}
	
	return profit_loss;
}

// --------------------------------------------------------------------------------

int main(int argc, char** argv){

	cout << endl;

	// The account
	Account account = {"USD", 1000, 0};

	// set account_currency by argument
	if ( argc == 2 ) {
		account.currency = argv[1];
	}

	cout << account << endl;

	
	// Exchange Symbols for EUR account
	Symbol eurjpy = {"EUR/JPY", "EUR", "JPY", 130.624, 130.624, 0.01, 4, 100000};
	Symbol eurgbp = {"EUR/GBP", "EUR", "GBP", 0.89254, 0.89254, 0.0001, 5, 100000};
	Symbol eurchf = {"EUR/CHF", "EUR", "CHF", 1.16319, 1.16319, 0.0001, 5, 100000};
	Symbol eurcad = {"EUR/CAD", "EUR", "CAD", 1.53933, 1.53933, 0.0001, 5, 100000};

	// MAJORS
	Symbol audusd = {"AUD/USD", "AUD", "USD", 0.74202, 0.74212, 0.0001, 5, 100000};
	Symbol eurusd = {"EUR/USD", "EUR", "USD", 1.17206, 1.17216, 0.0001, 5, 100000};
	Symbol gbpusd = {"GBP/USD", "GBP", "USD", 1.31307, 1.31317, 0.0001, 5, 100000};
	Symbol nzdusd = {"NZD/USD", "NZD", "USD", 0.68085, 0.68095, 0.0001, 5, 100000};
	Symbol usdcad = {"USD/CAD", "USD", "CAD", 1.31325, 1.31335, 0.0001, 5, 100000};
	Symbol usdchf = {"USD/CHF", "USD", "CHF", 0.99242, 0.99252, 0.0001, 5, 100000};
	Symbol usdjpy = {"USD/JPY", "USD", "JPY", 111.445, 111.446, 0.01, 3, 100000};

	typedef std::pair<std::string, Symbol> major_pair;
	std::map<std::string, Symbol> majors;

	majors.insert( major_pair(eurusd.symbol, eurusd) );
	majors.insert( major_pair(usdjpy.symbol, usdjpy) );
	majors.insert( major_pair(gbpusd.symbol, gbpusd) );
	majors.insert( major_pair(usdchf.symbol, usdchf) );
	majors.insert( major_pair(audusd.symbol, audusd) );
	majors.insert( major_pair(usdcad.symbol, usdcad) );
	majors.insert( major_pair(nzdusd.symbol, nzdusd) );

	// Traded symbol
	Symbol snapshot = eurusd;
	cout << snapshot << endl;

	// Position
	Position position = {Side::BUY, 100000, snapshot.bid - snapshot.point_size, 0, 0};
	cout << position << endl;

	// PipValue
	double pip_value = 0;
	double conversion_price = 0;

	// IMPORTANT FOR EUR ACCOUNT
	if ( account.currency == "EUR" ) {
		for ( auto it = majors.begin(); it != majors.end(); ++it ) {
			// AUD/USD, EUR/USD, GBP/USD, NZD/USD, ...
			if ( it->second.quote_currency == "USD" ) {
				pip_value = get_pip_value( it->second, position.qty, account.currency );
			} 
			// USD/CAD, USD/CHF, USD/JPY, ...
			else if ( it->second.base_currency == "USD" ) {

				// get conversion rate
				conversion_price = 1 / eurusd.ask;

				if ( it->second.quote_currency == "CAD" ) {
					conversion_price = eurcad.ask;
				} else if ( it->second.quote_currency == "CHF" ) {
					conversion_price = eurchf.ask;
				} else if ( it->second.quote_currency == "JPY" ) {
					conversion_price = eurjpy.ask;
				}

				pip_value = get_pip_value( it->second, position.qty, account.currency, conversion_price );
			}

			cout << it->first << " pip_v " << pip_value << " " << account.currency << endl;
		}
	} 
	// USD ACCOUNT
	else {
		for ( auto it = majors.begin(); it != majors.end(); ++it ) {
			pip_value = get_pip_value( it->second, position.qty, account.currency );
			cout << it->first << " pip_v " << pip_value << " " << account.currency << endl;
		}
		
	}
	//cout << " pip_v         " << pip_value << " " << account.currency << endl;
	//cout << " conversion_p  " << conversion_price << endl;

	// Profit/Loss Value
	const double pnl = get_profit_loss( snapshot, position, account.currency, conversion_price );
	cout << std::setprecision(5) << fixed;
	cout << " profit_loss   " << pnl << " " << account.currency << endl;

	cout << endl;

	return EXIT_SUCCESS;
}

