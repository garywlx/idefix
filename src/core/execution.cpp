#include "execution.h"

namespace idefix {

Execution::Execution() {
	
}

// Gets the unique ID of this execution.
std::string Execution::getId() const {
	return m_id;
}

// Gets the pips (points) gained/lost
double Execution::getPips() const {
	return 0;
}

// Gets the profit or loss associated with this trade.
double Execution::getPnL() const {
	return 0;
}

// Gets the entry price.
double Execution::getEntryPrice() const {
	return m_entry_price;
}

// Gets the type of execution (Entry, Exit or SAR).
enums::ExecutionType Execution::getType() const {
	return m_type;
}

// Set the execution id
void Execution::setId(const std::string id) {
	m_id = id;
}

// Sets the execution entry price
void Execution::setEntryPrice(const double price) {
	m_entry_price = price;
}

// Sets the execution type
void Execution::setType(const enums::ExecutionType type) {
	m_type = type;
}

};