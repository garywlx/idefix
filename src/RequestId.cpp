#include "RequestId.h"

#include <iostream>
#include <sstream>
#include <fstream>

#define REQUESTID_STORE_FILENAME "orderid.txt"

namespace IDEFIX {
RequestId::RequestId(): m_mutex(), m_request_id(1), m_order_id(restore()) {

}

RequestId::~RequestId(){
	store();
}

// Save order id to file
void RequestId::store(){
	FIX::Locker lock(m_mutex);
	std::ofstream file(REQUESTID_STORE_FILENAME, std::ios::trunc);
	file << m_order_id << '\n';
}

// Restore order id from file
int RequestId::restore(){
	FIX::Locker lock(m_mutex);
	std::ifstream file(REQUESTID_STORE_FILENAME);
	std::string s;
	if(!file.fail()){
		std::getline(file, s);	
	}
	
	return s.empty() ? 1 : std::stoi(s);
}

// Get next order id
std::string RequestId::nextOrderID() {
	FIX::Locker lock(m_mutex);
	m_order_id++;
	std::stringstream s;
	s << m_order_id;
	return s.str();
}

// Get current order id
std::string RequestId::currentOrderID() const {
	FIX::Locker lock(m_mutex);
	std::stringstream s;
	s << m_order_id;
	return s.str();
}

// Get next request id
// Will be reset to 0 on deconstruct
std::string RequestId::nextRequestID() {
	FIX::Locker lock(m_mutex);
	m_request_id++;
	std::stringstream s;
	s << m_request_id;
	return s.str();
}

}; // NS idefix