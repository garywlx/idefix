#ifndef IDEFIX_ACCOUNT_H
#define IDEFIX_ACCOUNT_H

#include <string>
#include <iomanip>
#include <ostream>

namespace IDEFIX {
class Account {
private:
	std::string m_account_id;
	std::string m_securities_account_id;
	std::string m_person;
	double m_balance;
	double m_margin_used;
	double m_free_margin;
	double m_equity;
	std::string m_currency;
	bool m_hedging;
	double m_contract_size;
	double m_margin_ratio;
	// This is the base unit size for FX pairs.
    // If it’s 1000 it means quantities for FX orders must be a multiple of 1000.
	double m_min_trade_size; // tag 53 in CollateralReport, 
	

public:
	Account();
	~Account();

	void setAccountID(const std::string& account_id);
	std::string getAccountID() const;

	void setSecuritiesAccountID(const std::string& account_id);
	std::string getSecuritiesAccountID() const;

	void setPerson(const std::string& person);
	std::string getPerson() const;

	void setBalance(const double balance);
	double getBalance() const;
	void addBalance(const double amount);
	void subBalance(const double amount);

	void setMarginUsed(const double margin_used);
	double getMarginUsed() const;

	void setFreeMargin(const double free_margin);
	double getFreeMargin() const;

	void setEquity(const double equity);
	double getEquity() const;

	void setCurrency(const std::string& currency);
	std::string getCurrency() const;

	void setHedging(const bool hedging);
	bool isHedging() const;

	void setContractSize(const double contract_size);
	double getContractSize() const;

	void setMarginRatio(const double ratio);
	double getMarginRatio() const;

	double getMMR() const;

	void setMinTradeSize(const double min);
	double getMinTradeSize() const;

	bool operator==(const Account& other) {
		return m_account_id == other.getAccountID();
	}

	bool operator!=(const Account& other) {
		return m_account_id != other.getAccountID();
	}

	double getConversionPrice(const std::string& symbol);

	std::string toString() const;
};

inline std::ostream& operator<<(std::ostream& out, const Account& acc) {
	out << acc.toString();
	return out;
}

};

#endif