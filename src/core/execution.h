#pragma once

/// Represents an executed order.

#include "core/order.h"

namespace idefix {
class Execution: public Order {
public:
	Execution();
	
	// Gets the unique ID of this execution.
	std::string getId() const;
	// Gets the pips (points) gained/lost
	double getPips() const; 
	// Gets the profit or loss associated with this trade.
	double getPnL() const;
	// Gets the entry price.
	double getEntryPrice() const;
	// Gets the type of execution (Entry, Exit or SAR).
	enums::ExecutionType getType() const;

	void setId(const std::string id);
	void setEntryPrice(const double price);
	void setType(const enums::ExecutionType type);

private:
	std::string m_id;
	enums::ExecutionType m_type;
	double m_entry_price;
};
}; 
