#include "account.h"

namespace idefix {

Account::Account() {

}

std::string Account::getAccountID() const {
	return m_account_id;
}

void Account::setAccountID(const std::string id) {
	m_account_id = id;
}

double Account::getBalance() const {
	return m_balance;
}

void Account::setBalance(const double value) {
	m_balance = value;
}
};