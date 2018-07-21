/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */

#include <iostream>
#include <string>
#include <ostream>
#include <iomanip>
#include <cmath>

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
 * 
 * @param const Symbol      snapshot         The current market snapshot.
 * @param const double      pos_qty          The current position qty.
 * @param const std::string account_currency The account currency in capital letters.
 * @return double
 */
double get_pip_value(const Symbol snapshot, const double pos_qty, const std::string account_currency) {
	// PipValue = (Pip in decimal places * Trade Size) / Market Price
	double pip_v = ( snapshot.point_size * pos_qty );

	// if account is denominated in EUR, devide by ask price
	// OR if symbol base is USD
	if ( account_currency == "EUR" || ( snapshot.base_currency == "USD" && account_currency == "USD" ) ) {
		pip_v /= snapshot.ask;
	}

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
	// other pairs
	else {
		profit_loss = ( pip_diff_decimal * pos.qty ); // USD	

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
	Account account = {"EUR", 1000, 0};
	cout << account << endl;

	// this symbol is traded
	Symbol snapshot_eurusd = {"EUR/USD", "EUR", "USD", 1.1721, 1.17211, 0.0001, 5, 100000};
	Symbol snapshot_eurjpy = {"EUR/JPY", "EUR", "JPY", 130.624, 130.625, 0.01, 4, 100000};
	Symbol snapshot = {"USD/JPY", "USD", "JPY", 111.459, 111.460, 0.01, 3, 100000};
	//Symbol snapshot = snapshot_eurusd;
	cout << snapshot << endl;

	// Position
	//Position pos = {Side::BUY, 100000, 1.1719, 0, 0};
	Position pos = {Side::BUY, 100000, 111.458, 0, 0};
	cout << pos << endl;

	// PipValue
	double pip_value = 0;
	if ( account.currency == "EUR" && snapshot.quote_currency == "JPY" ) {
		pip_value = get_pip_value( snapshot_eurjpy, pos.qty, account.currency );
	} else {
		pip_value = get_pip_value( snapshot, pos.qty, account.currency );	
	}
	cout << " pip_v         " << pip_value << " " << account.currency << endl;
	
	// Profit/Loss Value
	const double pnl = get_profit_loss( snapshot, pos, account.currency, ( account.currency == "EUR" && snapshot.quote_currency == "JPY" ? snapshot_eurjpy.bid : 0 ) );
	cout << std::setprecision(2) << fixed;
	cout << " profit_loss   " << pnl << " " << account.currency << endl;

	cout << endl;

	return EXIT_SUCCESS;
}

