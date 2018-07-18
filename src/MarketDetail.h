#ifndef IDEFIX_MARKETDETAIL_H
#define IDEFIX_MARKETDETAIL_H

#include <string>
#include <ostream>
#include <iomanip>

namespace IDEFIX {
class MarketDetail {
private:
	// tag 55, EUR/USD
	std::string m_symbol;
	// tag 15, EUR
	std::string m_currency;
	// tag 228, For Derivatives: Contract Value Factor by which price must be adjusted to determine the true nominal value of one futures/options contract.
	// (Qty * Price) * Factor = Nominal Value.
	double m_factor;
	// tag 231
	double m_contractmultiplier;
	// tag 460, 4 = CURRENCY, 7 = INDEX, 2 = COMMODITY
	int m_product; 
	// tag 561, The trading lot size of a security.
	double m_roundlot;
	// tag 9000, FXCM_SYM_ID
	int m_sym_id;
	// tag 9001
	int m_sym_precision;
	// tag 9002
	double m_sym_point_size;
	// tag 9003
	double m_sym_interest_buy;
	// tag 9004
	double m_sym_interest_sell;
	// tag 9005
	int m_sym_sort_order;
	// tag 9076
	std::string m_subscription_status;
	// tag 9080
	int m_field_product_id;
	// tag 9090
	double m_cond_dist_stop;
	// tag 9091
	double m_cond_dist_limit;
	// tag 9092
	double m_cond_dist_entry_stop;
	// tag 9093
	double m_cond_dist_entry_limit;
	// tag 9094
	double m_max_quantity;
	// tag 9095
	double m_min_quantity;
	// tag 9096
	std::string m_trading_status;
public:
	explicit MarketDetail() {}
	inline ~MarketDetail() {}

	inline std::string getSymbol() const { return m_symbol; }
	inline void setSymbol(const std::string& symbol){
		if ( m_symbol != symbol ) {
			m_symbol = symbol;
		}
	}

	inline std::string getCurrency() const { return m_currency; }
	inline void setCurrency(const std::string& currency) {
		if ( m_currency != currency ) {
			m_currency = currency;
		}
	}

	inline double getFactor() const { return m_factor; }
	inline void setFactor(const double factor) {
		if ( m_factor != factor ) {
			m_factor = factor;
		}
	}

	inline double getContractMultiplier() const { return m_contractmultiplier; }
	inline void setContractMultiplier(const double multiplier) {
		if ( m_contractmultiplier != multiplier ) {
			m_contractmultiplier = multiplier;
		}
	}

	inline int getProduct() const { return m_product; }
	inline void setProduct(const int product) {
		if ( m_product != product ) {
			m_product = product;
		}
	}

	inline double getRoundlot() const { return m_roundlot; }
	inline void setRoundlot(const double roundlot) {
		if ( m_roundlot != roundlot ) {
			m_roundlot = roundlot;
		}
	}

	inline int getSymID() const { return m_sym_id; }
	inline void setSymID(const int symid) {
		if ( m_sym_id != symid ) {
			m_sym_id = symid;
		}
	}

	inline int getSymPrecision() const { return m_sym_precision; }
	inline void setSymPrecision(const int precision) {
		if ( m_sym_precision != precision ) {
			m_sym_precision = precision;
		}
	}

	inline double getSymPointsize() const { return m_sym_point_size; }
	inline void setSymPointsize(const double pointsize) {
		if ( m_sym_point_size != pointsize ) {
			m_sym_point_size = pointsize;
		}
	}

	inline double getSymInterestBuy() const { return m_sym_interest_buy; }
	inline void setSymInterestBuy(const double interestbuy) {
		if ( m_sym_interest_buy != interestbuy ) {
			m_sym_interest_buy = interestbuy;
		}
	}

	inline double getSymInterestSell() const { return m_sym_interest_sell; }
	inline void setSymInterestSell(const double interestsell) {
		if ( m_sym_interest_sell != interestsell ) {
			m_sym_interest_sell = interestsell;
		}
	}

	inline int getSymSortOrder() const { return m_sym_sort_order; }
	inline void setSymSortOrder(const int sortorder) {
		if ( m_sym_sort_order != sortorder ) {
			m_sym_sort_order = sortorder;
		}
	}

	inline std::string getSubscriptionStatus() const { return m_subscription_status; }
	inline void setSubscriptionStatus(const std::string& status) {
		if ( m_subscription_status != status ) {
			m_subscription_status = status;
		}
	}

	inline int getFieldProductID() const { return m_field_product_id; }
	inline void setFieldProductID(const int id) {
		if ( m_field_product_id != id ) {
			m_field_product_id = id;
		}
	}

	inline double getCondDistStop() const { return m_cond_dist_stop; }
	inline void setCondDistStop(const double value) {
		if ( m_cond_dist_stop != value ) {
			m_cond_dist_stop = value;
		}
	}

	inline double getCondDistLimit() const { return m_cond_dist_limit; }
	inline void setCondDistLimit(const double value) {
		if ( m_cond_dist_limit != value ) {
			m_cond_dist_limit = value;
		}
	}

	inline double getCondDistEntryStop() const { return m_cond_dist_entry_stop; }
	inline void setCondDistEntryStop(const double value) {
		if ( m_cond_dist_entry_stop != value ) {
			m_cond_dist_entry_stop = value;
		}
	}

	inline double getCondDistEntryLimit() const { return m_cond_dist_entry_limit; }
	inline void setCondDistEntryLimit(const double value) {
		if ( m_cond_dist_entry_limit != value ) {
			m_cond_dist_entry_limit = value;
		}
	}

	inline double getMaxQuantity() const { return m_max_quantity; }
	inline void setMaxQuantity(const double qty) {
		if ( m_max_quantity != qty ) {
			m_max_quantity = qty;
		}
	}

	inline double getMinQuantity() const { return m_min_quantity; }
	inline void setMinQuantity(const double qty) {
		if ( m_min_quantity != qty ) {
			m_min_quantity = qty;
		}
	}

	inline std::string getTradingStatus() const { return m_trading_status; }
	inline void setTradingStatus(const std::string& status) {
		if ( m_trading_status != status ) {
			m_trading_status = status;
		}
	}
};

inline std::ostream& operator<<(std::ostream& out, const IDEFIX::MarketDetail& md) {
	out << "[MarketDetail] {" << std::setprecision(md.getSymPrecision()) << std::endl
	    << " - Symbol             " <<  md.getSymbol() << std::endl
	    << " - Currency           " << md.getCurrency() << std::endl
	    << " - Factor             " << std::fixed << md.getFactor() << std::endl
	    << " - ContractMultiplier " << std::fixed << md.getContractMultiplier() << std::endl
	    << " - Product            " << md.getProduct() << std::endl
	    << " - RoundLot           " << std::fixed << md.getRoundlot() << std::endl
	    << " - SymID              " << md.getSymID() << std::endl
	    << " - SymPrecision       " << std::fixed << md.getSymPrecision() << std::endl
	    << " - SymPointsize       " << std::fixed << md.getSymPointsize() << std::endl
	    << " - SymInterestBuy     " << std::fixed << md.getSymInterestBuy() << std::endl
	    << " - SymInterestSell    " << std::fixed << md.getSymInterestSell() << std::endl
	    << " - SymSortOrder       " << md.getSymSortOrder() << std::endl
	    << " - SubscriptionStatus " << md.getSubscriptionStatus() << std::endl
	    << " - FieldProductID     " << md.getFieldProductID() << std::endl
	    << " - CondDistStop       " << std::fixed << md.getCondDistStop() << std::endl
	    << " - CondDistLimit      " << std::fixed << md.getCondDistLimit() << std::endl
	    << " - CondDistEntryStop  " << std::fixed << md.getCondDistEntryStop() << std::endl
	    << " - CondDistEntryLimit " << std::fixed << md.getCondDistEntryLimit() << std::endl
	    << " - TradingStatus      " << md.getTradingStatus() << std::endl
	    << "}" << std::endl;
	return out;
}

};
#endif