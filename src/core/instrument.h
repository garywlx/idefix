#pragma once

#include <string>
#include <utility>
#include <vector>

#include "exchangetypes.h"
#include "enums.h"

namespace idefix {
class Instrument {
public:

	Instrument();
	Instrument(const std::string& symbol);
	Instrument(const std::string& symbol, const double ask, const double bid, const double point);

	// Formats the given value as a string.
	std::string format(const double value);
	// Gets the current ask price for this instrument.
	double getAskPrice();
	// Gets the current bid price for this instrument.
	double getBidPrice();
	// Gets the current high price for this instrument
	double getHighPrice();
	// Gets the current low price for this instrument
	double getLowPrice();
	// Gets the size of a 'point'.
	double getPointSize() const;
	// Gets the difference between the bid and ask price for this instrument.
	double getSpread();
	// Gets the difference between the bid and ask price in points.
	double getSpreadPoints();
	double getFactor() const;
	double getContractMultiplier() const;
	double getRoundLot() const;
	double getInterestBuy() const;
	double getInterestSell() const;
	double getCondDistStop() const;
	double getCondDistLimit() const;
	double getCondDistEntryStop() const;
	double getCondDistEntryLimit() const;
	double getMinQuantity() const;
	double getMaxQuantity() const;
	enums::InstrumentType getProduct() const;
	int getSymID() const;
	int getPrecision() const;
	int getSortOrder() const;
	int getFieldProductID() const;
	// Get the volume
	int getVolume();
	// Gets the symbol for this instrument.
	std::string getSymbol() const;
	// Gets the currency string // tag 15, EUR
	std::string getCurrency() const;
	// FXCM tag 9076
	std::string getSubscriptionStatus() const;
	// FXCM tag 9096
	std::string getTradingStatus() const;

	void setCurrency(const std::string value);
	void setSubscriptionStatus(const std::string value);
	void setTradingStatus(const std::string value);
	void setPointSize(const double value);
	void setFactor(const double value);
	void setContractMultiplier(const double value);
	void setRoundLot(const double value);
	void setInterestBuy(const double value);
	void setInterestSell(const double value);
	void setCondDistStop(const double value);
	void setCondDistLimit(const double value);
	void setCondDistEntryStop(const double value);
	void setCondDistEntryLimit(const double value);
	void setMinQuantity(const double value);
	void setMaxQuantity(const double value);
	void setProduct(const enums::InstrumentType type);
	void setProduct(const int value);
	void setSymID(const int value);
	void setPrecision(const int value);
	void setSortOrder(const int value);
	void setFieldProductID(const int value);

	// Rounds the given value to the minimum tick size.
	double round(const double value);
	// Add tick
	void addTick(const ExchangeTick tick);

	// relational operator
	inline bool operator==(Instrument& x) {
		return ( x.getSymbol() == getSymbol() );
	}

	inline bool operator<(const Instrument& x) const {
		return false;
	}

private:
	
	std::string m_symbol; // tag 55, EUR/USD
	std::string m_currency; // tag 15, EUR
	std::string m_subscription_status; // FXCM tag 9076
	std::string m_trading_status; // FXCM tag 9096

	double m_ask_price;
	double m_bid_price;
	double m_high_price;
	double m_low_price;

	// tag 228, For Derivatives: Contract Value Factor by which price must be adjusted to determine the true nominal 
	// value of one futures/options contract. (Qty * Price) * Factor = Nominal Value.
	double m_factor; // tag 228
	double m_contract_multiplier; // tag 231
	double m_round_lot; // tag 561, The trading lot size of a security.
	double m_sym_point_size; // FXCM tag 9002
	double m_sym_interest_buy; // FXCM tag 9003
	double m_sym_interest_sell; // FXCM tag 9004
	double m_sym_cond_dist_stop; // FXCM tag 9090
	double m_sym_cond_dist_limit; // FXCM tag 9091
	double m_sym_cond_dist_entry_stop; // FXCM tag 9092
	double m_sym_cond_dist_entry_limit; // FXCM tag 9093
	double m_max_quantity; // FXCM tag 9094
	double m_min_quantity; // FXCM tag 9095

	int m_product; // tag 460, 4 = CURRENCY, 7 = INDEX, 2 = COMMODITY
	int m_sym_id; // FXCM FXCM tag 9000, FXCM_SYM_ID
	int m_sym_precision; // FXCM tag 9001
	int m_sym_order; // FXCM tag 9005
	int m_sym_field_product_id; // FXCM tag 9080

	std::vector<ExchangeTick> m_ticks;

};
};