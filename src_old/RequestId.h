#ifndef IDEFIX_REQUESTID_H
#define IDEFIX_REQUESTID_H

#include <string>
#include <quickfix/Mutex.h>

// Creates unique IDs for orders and market data subscriptions
// Synchronized.
namespace IDEFIX {
class RequestId {
public:
	RequestId();
	~RequestId();
	std::string nextOrderID();
	std::string currentOrderID() const;
	std::string nextRequestID();

private:
	void store();
	int restore();
	RequestId(const RequestId& rhs);
	RequestId& operator=(const RequestId& rhs);

	mutable FIX::Mutex m_mutex;
	int m_request_id;
	int m_order_id;

}; // class 
}; // NS idefix

#endif
