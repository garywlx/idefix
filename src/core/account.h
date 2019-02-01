#pragma once

#include <string>

namespace idefix {
class Account {
public:
	Account();

	std::string getAccountID() const;
	void setAccountID(const std::string id);

	double getBalance() const;
	void setBalance(const double value);

private:
	std::string m_account_id;
	double m_balance;
};
};