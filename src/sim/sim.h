#pragma once

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"
#include "quickfix/Utility.h"
#include "quickfix/Mutex.h"

#include "quickfix/fix44/NewOrderSingle.h"
#include "quickfix/fix44/BusinessMessageReject.h"
#include "quickfix/fix44/MarketDataRequest.h"

namespace idefix {
class Sim: public FIX::Application, public FIX::MessageCracker {
public:
	Sim(): m_orderID(0), m_execID(0) {};

	// Application overloads
	void onCreate( const FIX::SessionID& );
	void onLogon( const FIX::SessionID& sessionID );
	void onLogout( const FIX::SessionID& sessionID );
	void toAdmin( FIX::Message&, const FIX::SessionID& );
	void toApp( FIX::Message&, const FIX::SessionID& )
	throw( FIX::DoNotSend );
	void fromAdmin( const FIX::Message&, const FIX::SessionID& )
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon );
	void fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType );

  	// MessageCracker overloads
	void onMessage( const FIX44::NewOrderSingle&, const FIX::SessionID& );
	void onMessage( const FIX44::BusinessMessageReject&, const FIX::SessionID& );
	void onMessage( const FIX44::MarketDataRequest&, const FIX::SessionID& );

	std::string genOrderID() {
		std::stringstream stream;
		stream << ++m_orderID;
		return stream.str();
	}
	std::string genExecID() {
		std::stringstream stream;
		stream << ++m_execID;
		return stream.str();
	}
private:
	int m_orderID, m_execID;
	std::vector<FIX::SessionID> m_sessions;
};
};