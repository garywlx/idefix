#include "sim.h"
#include "core/logger.h"

#include <quickfix/Session.h>
#include <quickfix/fix44/ExecutionReport.h>
#include <quickfix/fix44/Logon.h>
#include <quickfix/fix44/SecurityList.h>

#include <algorithm>
#include <iostream>

namespace idefix {
	void Sim::onCreate( const FIX::SessionID& sessionID ) {
		SPDLOG_INFO("onCreate");
	}
	void Sim::onLogon( const FIX::SessionID& sessionID ) {
		SPDLOG_INFO("onLogon");
		m_sessions.push_back( sessionID ); 
	}
	void Sim::onLogout( const FIX::SessionID& sessionID ) {
		SPDLOG_INFO("onLogout");
		auto it = std::find_if( m_sessions.begin(), m_sessions.end(), [&](const FIX::SessionID& lh) {
			return lh == sessionID;
		});
		if ( it == m_sessions.end() ) return;

		m_sessions.erase( it );
	}
	void Sim::toAdmin( FIX::Message& message,
		const FIX::SessionID& sessionID ) {
		SPDLOG_INFO("OUT ADMIN: {}", message.toString() );
	}
	void Sim::toApp( FIX::Message& message,
		const FIX::SessionID& sessionID )
	throw( FIX::DoNotSend ) {
		SPDLOG_INFO("OUT APP: {}", message.toString());
	}

	void Sim::fromAdmin( const FIX::Message& message,
		const FIX::SessionID& sessionID )
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon ) {
		SPDLOG_INFO("IN ADMIN: {}", message.toString() );
	}

	void Sim::fromApp( const FIX::Message& message,
		const FIX::SessionID& sessionID )
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
	{ 
		SPDLOG_INFO("IN APP: {}", message.toString()  );
		crack( message, sessionID ); 
	}

	void Sim::onMessage( const FIX44::NewOrderSingle& message,
		const FIX::SessionID& sessionID )
	{
		FIX::Symbol symbol;
		FIX::Side side;
		FIX::OrdType ordType;
		FIX::OrderQty orderQty;
		FIX::Price price;
		FIX::ClOrdID clOrdID;
		FIX::Account account;

		message.get( ordType );

		if ( ordType != FIX::OrdType_LIMIT )
			throw FIX::IncorrectTagValue( ordType.getField() );

		message.get( symbol );
		message.get( side );
		message.get( orderQty );
		message.get( price );
		message.get( clOrdID );

		FIX44::ExecutionReport executionReport = FIX44::ExecutionReport
		( FIX::OrderID( genOrderID() ),
			FIX::ExecID( genExecID() ),
			FIX::ExecType( FIX::ExecType_TRADE ),
			FIX::OrdStatus( FIX::OrdStatus_FILLED ),
			side,
			FIX::LeavesQty( 0 ),
			FIX::CumQty( orderQty ),
			FIX::AvgPx( price ) );

		executionReport.set( clOrdID );
		executionReport.set( symbol );
		executionReport.set( orderQty );
		executionReport.set( FIX::LastQty( orderQty ) );
		executionReport.set( FIX::LastPx( price ) );

		if( message.isSet(account) )
			executionReport.setField( message.get(account) );

		try
		{
			FIX::Session::sendToTarget( executionReport, sessionID );
		}
		catch ( FIX::SessionNotFound& ) {}
	}

	void Sim::onMessage( const FIX44::BusinessMessageReject& msg, const FIX::SessionID& sessionID) {
		std::string text = msg.getField( FIX::FIELD::Text );
		std::string refmsgtype = msg.getField( FIX::FIELD::RefMsgType );
		int reason = FIX::IntConvertor::convert( msg.getField( FIX::FIELD::BusinessRejectReason ) );
		std::string reasontext;

		switch( reason ) {
			case 0: reasontext = "Other"; break;
			case 1: reasontext = "Unknown ID"; break;
			case 2: reasontext = "Unknown Security"; break;
			case 3: reasontext = "Unsupported Message Type"; break;
			case 4: reasontext = "Application not available"; break;
			case 5: reasontext = "Conditionally Required Field Missing"; break;
			case 6: reasontext = "Not authorized"; break;
			case 7: reasontext = "DeliverTo firm not available at this time."; break;
		}

		SPDLOG_WARN("BusinessMessageReject: {} RefMsgType: {} Reason: {}", text, refmsgtype, reasontext );
	}

	void Sim::onMessage( const FIX44::MarketDataRequest& msg, const FIX::SessionID& sessionID) {
		int symbol_count = FIX::IntConvertor::convert( msg.getField( FIX::FIELD::NoRelatedSym ) );

		SPDLOG_INFO("MarketDataRequest {:d}", symbol_count);

		for ( int i = 1; i < symbol_count; i++ ) {
			FIX44::SecurityList::NoRelatedSym symbols_group;
			msg.getGroup( i, symbols_group );

			std::string symbol = symbols_group.getField( FIX::FIELD::Symbol );
			SPDLOG_INFO("Subscribe to {}", symbol);
		}
	}
};