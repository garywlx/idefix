#include "Account.h"

namespace IDEFIX {
	Account::Account(): m_balance(0), m_margin_used(0), m_hedging(false), m_contract_size(0) {}
	Account::~Account() {}

	void Account::setAccountID(const std::string& account_id) {
		if ( m_account_id != account_id ) {
			m_account_id = account_id;
		}
	}
	std::string Account::getAccountID() const {
		return m_account_id;
	}
	
	void Account::setSecuritiesAccountID(const std::string& account_id) {
		if ( m_securities_account_id != account_id ) {
			m_securities_account_id = account_id;
		}
	}

	std::string Account::getSecuritiesAccountID() const {
		return m_securities_account_id;
	}

	void Account::setPerson(const std::string& person) {
		if ( m_person != person ) {
			m_person = person;
		}
	}

	std::string Account::getPerson() const {
		return m_person;
	}

	void Account::setBalance(const double balance) {
		if ( m_balance != balance ) {
			m_balance = balance;
		}
	}
	
	double Account::getBalance() const {
		return m_balance;
	}
	
	void Account::addBalance(const double amount) {
		m_balance += amount;
	}
	
	void Account::subBalance(const double amount) {
		m_balance -= amount;
	}
	
	void Account::setMarginUsed(const double margin_used) {
		if ( m_margin_used != margin_used ) {
			m_margin_used = margin_used;
		}
	}
	
	double Account::getMarginUsed() const {
		return m_margin_used;
	}
	
	void Account::setCurrency(const std::string& currency) {
		if ( m_currency != currency ) {
			m_currency = currency;
		}
	}
	std::string Account::getCurrency() const {
		return m_currency;
	}
	
	void Account::setHedging(const bool hedging) {
		if ( m_hedging != hedging ) {
			m_hedging = hedging;
		}
	}
	
	bool Account::isHedging() const {
		return m_hedging;
	}
	
	void Account::setContractSize(const double contract_size) {
		if ( m_contract_size != contract_size ) {
			m_contract_size = contract_size;
		}
	}
	
	double Account::getContractSize() const {
		return m_contract_size;
	}

	void Account::setMarginRatio(const double ratio) {
		if ( m_margin_ratio != ratio ) {
			m_margin_ratio = ratio;
		}
	}

	/*!
	 * The amount reflected in this filed indicates the margin required to open a one lot position.
	 * @return double
	 */
	double Account::getMarginRatio() const {
		return m_margin_ratio;
	}

	/*!
	 * MMR = ContractSize * MarginRatio
	 * @return [description]
	 */
	double Account::getMMR() const {
		return getMarginRatio() * getContractSize();
	}

	/*!
	 * The minimum trade size allowed in this account. Tag 53 in CollateralReport
	 * 
	 * @param const double min
	 */
	void Account::setMinTradeSize(const double min) {
		if ( m_min_trade_size != min ) {
			m_min_trade_size = min;
		}
	}

	double Account::getMinTradeSize() const {
		return m_min_trade_size;
	}
};