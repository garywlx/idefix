#include "fxcm.h"
#include "core/logger.h"
#include "core/utility.h"
#include "core/order.h"

#include <quickfix/Utility.h>
#include <quickfix/Dictionary.h>
#include <quickfix/fix44/TradingSessionStatusRequest.h>
#include <quickfix/fix44/CollateralInquiry.h>
#include <quickfix/fix44/RequestForPositions.h>
#include <quickfix/fix44/MarketDataRequest.h>

#include <string>
#include <exception>
#include <vector>

namespace idefix {
	FXCM::FXCM() {
		// Initialize unsigned int requestID to 1. We will use this as a 
		// counter for making request IDs
		m_request_id = 1;
	}

	// ----------------------------------------------------------------------------------
	// FIX METHODS
	// ----------------------------------------------------------------------------------

	/**
	 * Gets called when quickfix creates a new session. A session comes into and remains in existence
	 * for the life of the application
	 * 
	 * @param const SessionID& sessionID
	 */
	void FXCM::onCreate(const SessionID &sessionID) {
		SPDLOG_INFO( "[onCreate] send Logon(A) message." );

		// FIX Session created. We must now logon. Quickfix will automatically send
		// the Logon(A) message.
  		if ( sessionID.toString().find( "MD_" ) != string::npos ) {
  			// this is the market data session
  			m_sessions[ "market" ] = sessionID;
  			// API Callback
  			onExchangeSessionCreated( "market" );
  		} else {
  			// this is the order session
  			m_sessions[ "order" ] = sessionID;
  			// API Callback
  			onExchangeSessionCreated( "order" );
  		}

	}

	/**
	 * Notifies you when a valid logon has been established with FXCM
	 * 
	 * @param const SessionID& sessionID
	 */
	void FXCM::onLogon(const SessionID &sessionID) {
		// Session logon successful. Now we request TradingSessionStatus which is
		// used to determine market status (open or closed), to get a list of securities,
		// and to obtain important FXCM system parameters		
		if ( getSessID( "market" ) == sessionID ) {
			// API Callback
			onExchangeLogon( "market" );
		}

		if ( getSessID( "order" ) == sessionID ) {
			// API Callback
			onExchangeLogon( "order" );
			// Send Trading Status Request
			sendTradingStatusRequest();
		}
	}

	/**
	 * Notifies you when an FIX session is no longer online. This could happen during a normal logout
	 * exchange or because of a forced termination or a loss of network connection
     *
	 * @param const SessionID& sessionID
	 */
	void FXCM::onLogout(const SessionID &sessionID) {
		if ( sessionID.toString().find( "MD_" ) != string::npos ) {
			// API Callback
			onExchangeLogout( "market" );

			// remove market session id
			m_sessions.erase( "market" );
		} else {
			// API Callback
			onExchangeLogout( "order" );

			// remove order session id
			m_sessions.erase( "order" );
		}
	}

	/**
	 * Provides you with a peak at the administrative messages that are being sent from your FIX engine
	 * to FXCM
	 * 
	 * @param Message&         message   
	 * @param const SessionID& sessionID
	 */
	void FXCM::toAdmin(Message &message, const SessionID &sessionID) {
		// If the Admin message being sent to FXCM is of type Logon(A), we want
		// to set the Username and Password fields. We want to catch this message as it
		// is going out.
		string msg_type = message.getHeader().getField( FIELD::MsgType ); 
		if ( msg_type == "A" ) {
		    // Get both username and password from our settings file. Then set these
		    // respektive fields
			string str_username = m_settings_ptr->get().getString( "Username" );
			string str_password = m_settings_ptr->get().getString( "Password" );
			message.setField( Username( str_username ) );
			message.setField( Password( str_password ) );
		}
		// All messages sent to FXCM must contain the TargetSubID field (both Administrative and
		// Application messages). Here we set this.
		string sub_ID = m_settings_ptr->get().getString( "TargetSubID" );
		message.getHeader().setField( TargetSubID( sub_ID ) );
	}

	/**
	 * A callback for application messages that you are being sent to a counterparty
	 * 
	 * @param Message&         message   
	 * @param const SessionID& sessionID
	 * @throws FIX::DoNotSend
	 */
	void FXCM::toApp(Message &message, const SessionID &sessionID)
	throw( FIX::DoNotSend ) {
		// All messages sent to FXCM must contain the TargetSubID field (both Administrative and
		// Application messages).
		string sub_ID = m_settings_ptr->get().getString( "TargetSubID" );
		message.getHeader().setField( TargetSubID( sub_ID ) );
	}

	/**
	 * Notifies you when an administrative message is sent from FXCM to your FIX engine.
	 * 
	 * @param const Message&   message  
	 * @param const SessionID& sessionID
	 * @throws FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon
	 */
	void FXCM::fromAdmin(const Message &message, const SessionID &sessionID)
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon ) {

		string msgtype = message.getHeader().getField( FIELD::MsgType );
		if ( MsgType_Reject == msgtype ) {
			string text = message.getField( FIELD::Text );
		    string tagID = message.getField( FIELD::RefTagID );
		    string msgType = message.getField( FIELD::RefMsgType );

		    // API Callback
		    std::string msg = fmt::format( "[fromAdmin:Reject] tagID {} msgType {}: {}", tagID, msgType, text );
    		onExchangeWarning( msg );
		}

		// Call MessageCracker.crack method to handle the message by one of our
		// overloaded onMessage methods below
		crack( message, sessionID );
	}

	/**
	 * One of the core entry points for your FIX application. Every application level request will come through here
	 * 
	 * @param const Message& message  
	 * @param const SessionID& sessionID
	 * @throws FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType
	 */
	void FXCM::fromApp(const Message &message, const SessionID &sessionID)
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType ) {
		// Call MessageCracker.crack method to handle the message by one of our
		// overloaded onMessage methods below
		string msgtype = message.getHeader().getField( FIELD::MsgType );
		if ( MsgType_Reject == msgtype ) {
			string text = message.getField( FIELD::Text );
		    string tagID = message.getField( FIELD::RefTagID );
		    string msgType = message.getField( FIELD::RefMsgType );

		    // API Callback
		    std::string msg = fmt::format( "[fromApp:Reject] tagID {} msgType {}: {}", tagID, msgType, text );
    		onExchangeWarning( msg );
		}

		crack( message, sessionID );
	}

	/**
	 * The TradingSessionStatus message is used to provide an update on the status of the market. Furthermore, 
	 * this message contains useful system parameters as well as information about each trading security (embedded SecurityList).
	 * TradingSessionStatus should be requested upon successful Logon and subscribed to. The contents of the
	 * TradingSessionStatus message, specifically the SecurityList and system parameters, should dictate how fields
	 * are set when sending messages to FXCM.
	 * 
	 * @param const FIX44::TradingSessionStatus& tss
	 * @param const SessionID&                   sessionID 
	 */
	void FXCM::onMessage(const FIX44::TradingSessionStatus& tss, const SessionID& sessionID) {
		// Check TradSesStatus field to see if the trading desk is open or closed
		// 2 = Open; 3 = Closed
		string trad_status = tss.getField( FIELD::TradSesStatus );
		
		if ( trad_status == "2" ) {
			// API Callback - open trading desk
			onExchangeTradingDeskChange( true );
			
		} else if ( trad_status == "3" ) {
			// API Callback - closed trading desk
			onExchangeTradingDeskChange( false );
		}
		
		// Within the TradingSessionStatus message is an embeded SecurityList. From SecurityList we can see
		// the list of available trading securities and information relevant to each; e.g., point sizes,
		// minimum and maximum order quantities by security, etc. 
		std::vector<Instrument> instruments = {};
		int symbols_count = IntConvertor::convert( tss.getField( FIELD::NoRelatedSym ) );
		for ( int i = 1; i <= symbols_count; i++ ) {
			// Get the NoRelatedSym group and for each, print out the Symbol value
			FIX44::SecurityList::NoRelatedSym symbols_group;
			tss.getGroup( i, symbols_group );
			
			// add instrument to list
			Instrument instr( symbols_group.getField( FIELD::Symbol ) );

		    instr.setCurrency( symbols_group.getField( FIELD::Currency ) );
		    instr.setFactor( DoubleConvertor::convert( symbols_group.getField( FIELD::Factor ) ) );
		    instr.setContractMultiplier( DoubleConvertor::convert( symbols_group.getField( FIELD::ContractMultiplier ) ) );
		    instr.setProduct( IntConvertor::convert( symbols_group.getField( FIELD::Product ) ) );
		    instr.setRoundLot( DoubleConvertor::convert( symbols_group.getField( FIELD::RoundLot ) ) );

		    // FXCM related
		    instr.setSymID( IntConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_ID ) ) );
		    instr.setPrecision( IntConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_PRECISION ) ) );
		    instr.setPointSize( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_POINT_SIZE ) ) );
		    instr.setInterestBuy( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_INTEREST_BUY ) ) );
		    instr.setInterestSell( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_INTEREST_SELL ) ) );
		    instr.setSortOrder( IntConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_SORT_ORDER ) ) );
		    instr.setSubscriptionStatus( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SUBSCRIPTION_STATUS ) );
		    instr.setFieldProductID( IntConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_FIELD_PRODUCT_ID ) ) );
		    instr.setCondDistStop( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_COND_DIST_STOP ) ) );
		    instr.setCondDistLimit( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_COND_DIST_LIMIT ) ) );
		    instr.setCondDistEntryStop( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_COND_DIST_ENTRY_STOP ) ) );
		    instr.setCondDistEntryLimit( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_COND_DIST_ENTRY_LIMIT ) ) );
		    instr.setTradingStatus( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_TRADING_STATUS ) );
		    instr.setMinQuantity( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_MIN_QUANTITY ) ) );
		    instr.setMaxQuantity( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_MAX_QUANTITY ) ) );

			instruments.push_back( instr );
		}

		// API Callback
		onExchangeInstrumentList( instruments );
		
		// Also within TradingSessionStatus are FXCM system parameters. This includes important information
		// such as account base currency, server time zone, the time at which the trading day ends, and more.
		
		// Read field FXCMNoParam (9016) which shows us how many system parameters are 
		// in the message
		int params_count = IntConvertor::convert( tss.getField( FXCM_NO_PARAMS ) ); // FXCMNoParam (9016)
		ExchangeSettingsMap settings_map;
		for ( int i = 1; i < params_count; i++ ) {
			// For each paramater, print out both the name of the paramater and the value of the 
			// paramater. FXCMParamName (9017) is the name of the paramater and FXCMParamValue(9018)
			// is of course the paramater value
			FIX::FieldMap field_map = tss.getGroupRef( i, FXCM_NO_PARAMS );

			auto param_name  = field_map.getField( FXCM_PARAM_NAME );
			auto param_value = field_map.getField( FXCM_PARAM_VALUE );

			settings_map.emplace( param_name, param_value );
		}
		
		// API Callback
		onExchangeSettings( settings_map );

		// Request accounts under our login
		sendAccountsRequest();

		// ** Note on Text(58) ** 
		// You will notice that Text(58) field is always set to "Market is closed. Any trading
		// functionality is not available." This field is always set to this value; therefore, do not 
		// use this field value to determine if the trading desk is open. As stated above, use TradSesStatus for this purpose
	}

	/**
	 * Leave empty
	 * @param const FIX44::CollateralInquiryAck& ack        
	 * @param const SessionID&                   sessionID 
	 */
	void FXCM::onMessage(const FIX44::CollateralInquiryAck& ack, const SessionID& sessionID) {

	}

	/**
	 * CollateralReport is a message containing important information for each account under the login. It is returned
	 * as a response to CollateralInquiry. You will receive a CollateralReport for each account under your login.
	 * Notable fields include Account(1) which is the AccountID and CashOutstanding(901) which is the account balance
	 * 
	 * @param cr         
	 * @param sessionID 
	 */
	void FXCM::onMessage(const FIX44::CollateralReport& cr, const SessionID& sessionID) {
		//SPDLOG_INFO( "CollateralReport -> " );
		string accountID = cr.getField( FIELD::Account );
		// Get account balance, which is the cash balance in the account, not including any profit
		// or losses on open trades
		string balance = cr.getField( FIELD::CashOutstanding );

		// API Callback
		onExchangeAccountID( accountID );
		double balance_value = DoubleConvertor::convert( balance );
		onExchangeBalanceChange( accountID, balance_value );

		// The CollateralReport NoPartyIDs group can be inspected for additional account information
		// such as AccountName or HedgingStatus
		FIX44::CollateralReport::NoPartyIDs group;
		cr.getGroup( 1, group); // CollateralReport will only have 1 NoPartyIDs group

		// Get the number of NoPartySubIDs repeating groups
		int number_subID = IntConvertor::convert( group.getField( FIELD::NoPartySubIDs ) );
		ExchangeCollateralSettingsMap settingsMap;

		// For each group, print out both the PartySubIDType and the PartySubID (the value)
		for ( int u = 1; u <= number_subID; u++ ) {
			FIX44::CollateralReport::NoPartyIDs::NoPartySubIDs sub_group;
			group.getGroup( u, sub_group );

			string sub_type = sub_group.getField( FIELD::PartySubIDType );
			string sub_value = sub_group.getField( FIELD::PartySubID );
		
			if ( sub_type == "4000" ) {
				sub_type = "SUPPORTS_HEDGING";
			} else if ( sub_type == "2" ) {
				sub_type = "USERNAME";
			} else if ( sub_type == "22" ) {
				sub_type = "ACCOUNT_LASTNAME";
			} else if ( sub_type == "10" ) {
				sub_type = "ACCOUNT_ID";
			}

			settingsMap.emplace( sub_type, sub_value );
		}

		// API Callback
		onExchangeCollateralSettings( settingsMap );

		// Add the accountID to our vector<string> being used to track all
		// accounts under our login
		recordAccount( accountID );

		// API Callback
		onExchangeReady();
	}

	/**
	 * If a PositionReport is requested and no positions exist for that request, this message pops up
	 * 
	 * @param const FIX44::RequestForPositionsAck& ack       
	 * @param const SessionID&                     sessionID 
	 */
	void FXCM::onMessage(const FIX44::RequestForPositionsAck& ack, const SessionID& sessionID) {
		string pos_reqID = ack.getField( FIELD::PosReqID );
		SPDLOG_INFO( "RequestForPositionsAck -> PosReqID - {}", pos_reqID );

		// If a PositionReport is requested and no positions exist for that request, the Text field will
		// indicate that no positions mathced the requested criteria 
		if ( ack.isSetField( FIELD::Text ) ) {
			SPDLOG_INFO( "RequestForPositionsAck -> Text - {}", ack.getField( FIELD::Text ) );
		}
	}

	/**
	 * Get a detailed position report for open positions
	 * 
	 * @param const FIX44::PositionReport& pr        
	 * @param const SessionID&             sessionID 
	 */
	void FXCM::onMessage(const FIX44::PositionReport& pr, const SessionID& sessionID) {
		// Print out important position related information such as accountID and symbol 
		idefix::ExchangePositionReport position_report;
		position_report.account_id    = pr.getField( FIELD::Account );
		position_report.symbol        = pr.getField( FIELD::Symbol );
		position_report.position_id   = pr.getField( FXCM_POS_ID );
		position_report.pos_open_time = pr.getField( FXCM_POS_OPEN_TIME );

		// API Callback
		onExchangePositionReport( position_report );
	}

	/**
	 * If MarketDataRequestReject is returned as the result of a MarketDataRequest message,
	 * print out the contents of the Text field but first check that it is set
	 * 
	 * @param const FIX44::MarketDataRequestReject& mdr       
	 * @param const SessionID&                      sessionID 
	 */
	void FXCM::onMessage(const FIX44::MarketDataRequestReject& mdr, const SessionID& sessionID) {
		// If MarketDataRequestReject is returned as the result of a MarketDataRequest message,
		// print out the contents of the Text field but first check that it is set		
		if ( mdr.isSetField( FIELD::Text ) ) {
			// API Callback
			onExchangeMarketDataReject( mdr.getField( FIELD::Text ) );
		}
	}

	/**
	 * Get a MarketDataSnapshotFullRefresh Message (Tick)
	 * 
	 * @param const FIX44::MarketDataSnapshotFullRefresh& mds       
	 * @param const SessionID&                            sessionID 
	 */
	void FXCM::onMessage(const FIX44::MarketDataSnapshotFullRefresh& mds, const SessionID& sessionID) {
		// Get symbol name of the snapshot; e.g., EUR/USD. Our example only subscribes to EUR/USD so 
		// this is the only possible value
		idefix::ExchangeTick tick;
		tick.symbol    = mds.getField( FIELD::Symbol );
		tick.timestamp = mds.getHeader().getField( FIELD::SendingTime );

		// For each MDEntry in the message, inspect the NoMDEntries group for
		// the presence of either the Bid or Ask (Offer) type 
		int entry_count = IntConvertor::convert( mds.getField( FIELD::NoMDEntries ) );
		for ( int i = 1; i < entry_count; i++ ) {
			FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
		    mds.getGroup( i, group );
		    string entry_type = group.getField( FIELD::MDEntryType );
		    // 0 Bid
		    if(entry_type == MDEntryType( MDEntryType_BID ).getString() ){
				tick.bid = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx ) );
		    } 
		    // 1 Ask
		    else if(entry_type == MDEntryType( MDEntryType_OFFER ).getString() ) {
				tick.ask = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx ) );
		    } 
		    // 7 Session High
		    else if(entry_type == MDEntryType( MDEntryType_TRADING_SESSION_HIGH_PRICE ).getString() ) {
				tick.session_high = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx) );
		    }
		    // 8 Session Low
		    else if(entry_type == MDEntryType( MDEntryType_TRADING_SESSION_LOW_PRICE ).getString() ) {
				tick.session_low = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx) );
		    }
		}

		// API Callback
		onExchangeTick( tick );
	}

	/**
	 * If an order is executed this message pops up
	 * 
	 * @param const FIX44::ExecutionReport& er        
	 * @param const SessionID&              sessionID 
	 */
	void FXCM::onMessage(const FIX44::ExecutionReport& er, const SessionID& sessionID) {
		// SPDLOG_INFO( "ExecutionReport" );
		// SPDLOG_INFO( "  ClOrdID -> {}", er.getField( FIELD::ClOrdID ) );
		// SPDLOG_INFO( "  Account -> {}", er.getField( FIELD::Account ) );
		// SPDLOG_INFO( "  OrderID -> {}", er.getField( FIELD::OrderID ) );
		// SPDLOG_INFO( "  LastQty -> {}", er.getField( FIELD::LastQty ) );
		// SPDLOG_INFO( "  CumQty -> {}", er.getField( FIELD::CumQty ) );
		// SPDLOG_INFO( "  ExecType -> {}", er.getField( FIELD::ExecType ) );
		// SPDLOG_INFO( "  OrdStatus -> {}", er.getField( FIELD::OrdStatus ) );

		// ** Note on order status. ** 
		// In order to determine the status of an order, and also how much an order is filled, we must
		// use the OrdStatus and CumQty fields. There are 3 possible final values for OrdStatus: Filled (2),
		// Rejected (8), and Cancelled (4). When the OrdStatus field is set to one of these values, you know
		// the execution is completed. At this time the CumQty (14) can be inspected to determine if and how
		// much of an order was filled.

		std::string fxcm_pos_id = er.getField( FXCM_FIX_FIELDS::FXCM_POS_ID );
		if ( fxcm_pos_id.empty() ) {
			onExchangeError( "FIX44::ExecutionReport: FXCM PosID is empty!" );
			return;
		}

		std::string sending_time = er.getHeader().getField( FIELD::SendingTime );
		std::string order_status = er.getField( FIELD::OrdStatus );
		std::string order_type   = er.getField( FIELD::OrdType );
		std::string cl_ord_id    = er.getField( FIELD::ClOrdID );
		std::string last_qty     = er.getField( FIELD::LastQty );
		std::string last_price   = er.getField( FIELD::LastPx );
		std::string symbol       = er.getField( FIELD::Symbol );
		std::string account_id   = er.getField( FIELD::Account );
		std::string side         = er.getField( FIELD::Side );
		std::string cum_qty      = er.getField( FIELD::CumQty );
		std::string exec_type    = er.getField( FIELD::ExecType );

		if ( exec_type == FIX::ExecType_PARTIAL_FILL || exec_type == FIX::ExecType_FILL ) {
			// order has been partial filled (1)
			// order has been filled (2)
			
			auto exec = std::make_shared<Execution>();
			exec->setId( fxcm_pos_id );
			exec->setOrderID( cl_ord_id );
			exec->setFilled( IntConvertor::convert( cum_qty ) );
			exec->setLastFillPrice( DoubleConvertor::convert( last_price ) );
			exec->setLastFillTime( IntConvertor::convert( sending_time ) );
			exec->setSymbol( symbol );
			exec->setAction( side == '1' ? enums::OrderAction::BUY : enums::OrderAction::SELL );
			exec->setAccountID( account_id );
			
			if ( order_type == FIX::OrdType_MARKET ) {
				exec->Order::setType( enums::OrderType::MARKET );
				exec->setType( enums::ExecutionType::ENTRY );
			} else if ( order_type == FIX::OrdType_LIMIT ) {
				exec->Order::setType( enums::OrderType::LIMIT );
				exec->setType( enums::ExecutionType::EXIT );
			} else if ( order_type == FIX::OrdType_STOP ) {
				exec->Order::setType( enums::OrderType::STOP );
				exec->setType( enums::ExecutionType::EXIT );
			}
			
			// API Callback
			onExchangeOrderFilled( std::move( exec ) );

		} else if ( exec_type == FIX::ExecType_REJECTED ) {
			// order has been rejected (8)
			
		} else if ( exec_type == FIX::ExecType_CANCELED ) {
			// order has been cancelled (4)
			
		} else if ( exec_type == FIX::ExecType_NEW ) {
			// order has been received (0)
			

		}

		// FIX::ExecType execType;
		// FIX::OrdStatus ordStatus;
		// FIX::OrdType ordType;
		// FIX::ClOrdID clOrdID;
		// FIX::LastQty lastQty;
		// FIX::LastPx lastPx;
		// FIX::Side side;
		// FIX::Symbol symbol;
		// FIX::Account account;
		// FIX::CumQty cumQty;

		// er.get( execType );
		// er.get( ordStatus );
		// er.get( ordType );
		// er.get( clOrdID );
		// er.get( cumQty );
		// er.get( lastQty );
		// er.get( lastPx );
		// er.get( side );
		// er.get( symbol );
		// er.get( account );


		// If we have a new market order with fxcm position id.
		// if ( ! fxcm_pos_id.empty() ) {

		// // create market order.
		// 	Order marketOrder;
		// 	marketOrder.setPosID( fxcm_pos_id );
		// 	marketOrder.setSymbol( symbol.getValue() );
		// 	marketOrder.setSide( side.getValue() );
		// 	marketOrder.setPrice( lastPx.getValue() );
		// 	marketOrder.setQty( lastQty.getValue() );
		// 	marketOrder.setAccountID( account.getValue() );
		// 	marketOrder.setClOrdID( clOrdID.getValue() );
		// 	marketOrder.setSendingTime( sendingTime );
		// 	marketOrder.setStopPrice( 0 );
		// 	marketOrder.setTakePrice( 0 );
		// 	marketOrder.setClosePrice( 0 );

		// 	// MassOrderStatus
		// 	if ( execType == FIX::ExecType_NEW ) { // FIX::ExecType_ORDER_STATUS
		// 	  	// get specific fields for OrderStatus
		// 		FIX::OrderQty orderQty;
		// 		er.get( orderQty );

		// 	  	// MarketOrder with ExecType I = OrderStatus && OrdStatus 0 = New && OrdType 2 = Limit
		// 	  	// update order in list, set take profit
		// 		if ( ordStatus == FIX::OrdStatus_NEW && ordType == FIX::OrdType_LIMIT && orderQty.getValue() > 0 ) {
		// 	    	// console output
		// 			// console()->info( "{} OrderStatus->updateMarketOrder (LIMIT) {} {} ", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
		// 	    	// set take profit price.
		// 			marketOrder.setTakePrice( marketOrder.getPrice() );
		// 	    	// set OrderQty
		// 			marketOrder.setQty( orderQty.getValue() );
		// 	    	// update order in list.
		// 			// updateMarketOrder( marketOrder, true );
		// 	    	// API Callback
		// 			onExchangeOrder( idefix::enums::ExchangeOrderEvent::MARKET_ORDER_SET_TP, marketOrder );
		// 		}
		// 	  	// MarketOrder with ExecType I = OrderStatus && OrdStatus 0 = New && OrdType 3 = Stop
		// 	  	// update order in list, set stop loss
		// 		else if ( ordStatus == FIX::OrdStatus_NEW && ordType == FIX::OrdType_STOP && orderQty.getValue() > 0 ) {
		// 	    	// console output
		// 			// console()->info( "{} OrderStatus->updateMarketOrder (STOP) {} {}", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
		// 	    	// set stop price.
		// 			marketOrder.setStopPrice( marketOrder.getPrice() );
		// 	    	// set OrderQty
		// 			marketOrder.setQty( orderQty.getValue() );
		// 	    	// update order in list.
		// 			// updateMarketOrder( marketOrder, true );
		// 	    	// API Callback
		// 			onExchangeOrder( idefix::enums::ExchangeOrderEvent::MARKET_ORDER_SET_SL, marketOrder );
		// 		}
		// 	}
		// 	// MarketOrder with ExecTyp F = Trade && OrdStatus 2 = Filled && OrdType 1 = Market
		// 	// add new order in list
		// 	else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_MARKET ) {
		// 	  	// console output
		// 		// console()->info( "{} addMarketOrder {} {} fill @ {:.5f} {} qty {:.2f}", prefix, marketOrder.getSymbol(), marketOrder.getPosID(), marketOrder.getPrice(), marketOrder.getSideStr(), marketOrder.getQty() );
		// 	  	// add order  
		// 		// addMarketOrder( marketOrder );
		// 	  	// trade log
		// 		// tradelog( marketOrder );
		// 	  	// API Callback
		// 		onExchangeOrder( idefix::enums::ExchangeOrderEvent::MARKET_ORDER_NEW, marketOrder );
		// 	}
		// 	// MarketOrder with ExecType F = Trade && OrdStatus 2 = Filled && OrdType 2 = Limit
		// 	// take profit filled -> remove order from list
		// 	else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_LIMIT ) {
		// 	  	// console output
		// 		// console()->info( "{} removeMarketOrder (LIMIT) {} {}", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
		// 	  	// trade log
		// 		// tradelog( marketOrder );
		// 	  	// remove market order
		// 		// removeMarketOrder( marketOrder.getPosID() );
		// 	  	// API Callback
		// 		onExchangeOrder( idefix::enums::ExchangeOrderEvent::MARKET_ORDER_TP_HIT, marketOrder );
		// 	}
		// 	// MarketOrder with ExecType F = Trade && OrdStatus 2 = Filled && OrdType 3 = Stop
		// 	// stop loss filled -> remove order form list
		// 	else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_STOP ) { 
		// 	  	// console output
		// 		// console()->info( "{} removeMarketOrder (STOP) {} {}", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
		// 	  	// trade log
		// 		// tradelog( marketOrder );
		// 	  	// remove market order
		// 		// removeMarketOrder( marketOrder.getPosID() );
		// 	  	// API Callback
		// 		onExchangeOrder( idefix::enums::ExchangeOrderEvent::MARKET_ORDER_SL_HIT, marketOrder );
		// 	}
		// 	// MarketOrder with ExecType 4 = Canceled && OrdStatus 4 = Canceled 
		// 	// remove order from list
		// 	else if ( execType == FIX::ExecType_CANCELED && ordStatus == FIX::OrdStatus_CANCELED ) {
		// 	  	// console output
		// 		// console()->info( "{} removeMarketOrder (CANCELED) {} {}", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
		// 	  	// remove market order.
		// 		// removeMarketOrder( marketOrder.getPosID() );
		// 	  	// API Callback
		// 		onExchangeOrder( idefix::enums::ExchangeOrderEvent::MARKET_ORDER_CANCELED, marketOrder );
		// 	}
		// }
	}

	// ----------------------------------------------------------------------------------
	// PUBLIC METHODS
	// ----------------------------------------------------------------------------------

	/**
	 * Starts the FIX session. Throws FIX::ConfigError exception if our configuration settings
	 * do not pass validation required to construct SessionSettings 
	 */
	void FXCM::connect() noexcept {
		try {
			m_settings_ptr = new SessionSettings( getConfigFile() );
			m_store_factory_ptr = new FileStoreFactory( *m_settings_ptr );
			m_log_factory_ptr = new FileLogFactory( *m_settings_ptr );
			m_initiator_ptr = new SocketInitiator( *this, *m_store_factory_ptr, *m_settings_ptr, *m_log_factory_ptr );
			m_initiator_ptr->start();
		} catch( ConfigError error ) {
			SPDLOG_ERROR( error.what() );
		}
	}

	/**
	 * Logout and end session
	 */
	void FXCM::disconnect() noexcept {
		try {
			m_initiator_ptr->stop();

			while ( isConnected() ) {
				// blocking until stopped.
			}

			// clear accounts
			m_account_vec.clear();

			if ( m_initiator_ptr != nullptr ) delete m_initiator_ptr;
			if ( m_settings_ptr != nullptr ) delete m_settings_ptr;
			if ( m_store_factory_ptr != nullptr ) delete m_store_factory_ptr;
			if ( m_log_factory_ptr != nullptr ) delete m_log_factory_ptr;	
		} catch( ... ) {
			SPDLOG_ERROR( "Unknown Exception." );
		}
	}

	/**
	 * Check if there is an open connection
	 * 
	 * @return bool
	 */
	bool FXCM::isConnected() {
		if ( m_initiator_ptr != nullptr ) {
			auto order_session = getSessID( "order" );
			auto market_session = getSessID( "market" );

			return ( ! order_session.toString().empty() && 
				     ! market_session.toString().empty() && 
				     ! m_initiator_ptr->isStopped() && 
				     m_initiator_ptr->isLoggedOn() );
		}

		return false;
	}

	/**
	 * NetworkAdapter Interface -> subscribeMarketData
	 * 
	 * @param const std::string& symbol
	 */
	void FXCM::subscribeMarketData(const std::string& symbol) {
		sendMarketDataSubscription( symbol );
	}

	/**
	 * NetworkAdapter Interface -> unsubscribeMarketData
	 * 
	 * @param const std::string& symbol
	 */
	void FXCM::unsubscribeMarketData(const std::string& symbol) {
		sendMarketDataUnsubscription( symbol );
	}

	// ----------------------------------------------------------------------------------
	// PRIVATE METHODS
	// ----------------------------------------------------------------------------------

	/**
	 * Get SessionID for market or order
	 * 
	 * @param const std::string& name either market or order
	 * @return SessionID
	 */
	SessionID FXCM::getSessID(const std::string& name) const {
		return FindInMap( m_sessions, name );
	}

	/**
	 * Sends TradingSessionStatusRequest message in order to receive as a response the
	 * TradingSessionStatus message
	 */
	void FXCM::sendTradingStatusRequest() {
		FIX44::TradingSessionStatusRequest request;
		request.setField( TradSesReqID( nextRequestID() ) );
		request.setField( TradingSessionID( "FXCM" ) );
		request.setField( SubscriptionRequestType( SubscriptionRequestType_SNAPSHOT ) );
		
		Session::sendToTarget( request, getSessID( "order" ) );
	}

	/**
	 * Sends the CollateralInquiry message in order to receive as a response the
	 * CollateralReport message.
	 */
	void FXCM::sendAccountsRequest() {
		// Request CollateralReport message. We will receive a CollateralReport for each
		// account under our login
		FIX44::CollateralInquiry request;
		request.setField( CollInquiryID( nextRequestID() ) );
		request.setField( TradingSessionID( "FXCM" ) );
		request.setField( SubscriptionRequestType( SubscriptionRequestType_SNAPSHOT ) );
		
		Session::sendToTarget( request, getSessID( "order" ) );
	}

	/**
	 * Sends RequestForPositions which will return PositionReport messages if positions
	 * matching the requested criteria exist; otherwise, a RequestForPositionsAck will be
	 * sent with the acknowledgement that no positions exist. In our example, we request
	 * positions for all accounts under our login
	 */
	void FXCM::sendPositionRequest() {
		// Here we will get positions for each account under our login. To do this,
		// we will send a RequestForPositions message that contains the accountID 
		// associated with our request. For each account in our list, we send
		// RequestForPositions. 
		int total_accounts = (int) m_account_vec.size();
		for ( int i = 0; i < total_accounts; i++ ) {
			string accountID =  m_account_vec.at( i );
			// Set default fields
			FIX44::RequestForPositions request;
			request.setField( PosReqID( nextRequestID() ) );
			request.setField( PosReqType( PosReqType_POSITIONS ) );
			// AccountID for the request. This must be set for routing purposes. We must
			// also set the Parties AccountID field in the NoPartySubIDs group
			request.setField( Account( accountID ) );
			request.setField( SubscriptionRequestType( SubscriptionRequestType_SNAPSHOT ) );
			request.setField( AccountType( AccountType_ACCOUNT_IS_CARRIED_ON_NON_CUSTOMER_SIDE_OF_BOOKS_AND_IS_CROSS_MARGINED ) );
			request.setField( TransactTime() );
			request.setField( ClearingBusinessDate() );
			request.setField( TradingSessionID( "FXCM" ) );
			// Set NoPartyIDs group. These values are always as seen below
			request.setField( NoPartyIDs( 1 ) );
			FIX44::RequestForPositions::NoPartyIDs parties_group;
			parties_group.setField( PartyID( "FXCM ID" ) );
			parties_group.setField( PartyIDSource( 'D' ) );
			parties_group.setField( PartyRole( 3 ) );
			parties_group.setField( NoPartySubIDs( 1 ) );
			// Set NoPartySubIDs group
			FIX44::RequestForPositions::NoPartyIDs::NoPartySubIDs sub_parties;
			sub_parties.setField( PartySubIDType( PartySubIDType_SECURITIES_ACCOUNT_NUMBER ) );
			// Set Parties AccountID
			sub_parties.setField( PartySubID( accountID ) );
			// Add NoPartySubIds group
			parties_group.addGroup( sub_parties );
			// Add NoPartyIDs group
			request.addGroup( parties_group );
			// Send request
			Session::sendToTarget( request, getSessID( "order" ) );
		}
	}

	/**
	 * Subscribes to a trading security
	 * 
	 * @param const std::string& symbol 
	 */
	void FXCM::sendMarketDataSubscription(const std::string& symbol) {
		// Subscribe to market data for EUR/USD
		string request_ID = symbol + "_Request_";
		FIX44::MarketDataRequest request;
		request.setField( MDReqID( request_ID ) );
		request.setField( SubscriptionRequestType( SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES ) );
		request.setField( MarketDepth( 0 ) );
		request.setField( NoRelatedSym( 1 ) );

		// Add the NoRelatedSym group to the request with Symbol
		// field set to EUR/USD
		FIX44::MarketDataRequest::NoRelatedSym symbols_group;
		symbols_group.setField( Symbol( symbol ) );
		request.addGroup( symbols_group );

		// Add the NoMDEntryTypes group to the request for each MDEntryType
		// that we are subscribing to. This includes Bid, Offer, High, and Low
		FIX44::MarketDataRequest::NoMDEntryTypes entry_types;
		entry_types.setField( MDEntryType( MDEntryType_BID ) );
		request.addGroup( entry_types );
		entry_types.setField( MDEntryType( MDEntryType_OFFER ) );
		request.addGroup( entry_types );
		entry_types.setField( MDEntryType( MDEntryType_TRADING_SESSION_HIGH_PRICE ) );
		request.addGroup( entry_types );
		entry_types.setField( MDEntryType( MDEntryType_TRADING_SESSION_LOW_PRICE ) );
		request.addGroup( entry_types );

		Session::sendToTarget( request, getSessID( "market" ) );
	}

	/**
	 * Unsubscribe from a trading security
	 * 
	 * @param const std::string& symbol 
	 */
	void FXCM::sendMarketDataUnsubscription(const std::string& symbol) {
		// Unsubscribe from EUR/USD. Note that our request_ID is the exact same
		// that was sent for our request to subscribe. This is necessary to 
		// unsubscribe. This request below is identical to our request to subscribe
		// with the exception that SubscriptionRequestType is set to
		// "SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT_PLUS_UPDATE_REQUEST"
		string request_ID = symbol + "_Request_";
		FIX44::MarketDataRequest request;
		request.setField( MDReqID(request_ID ) );
		request.setField( SubscriptionRequestType ( SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT_PLUS_UPDATE_REQUEST ) );
		request.setField( MarketDepth( 0 ) );
		request.setField( NoRelatedSym( 1 ) );

		// Add the NoRelatedSym group to the request with Symbol
		// field set to EUR/USD
		FIX44::MarketDataRequest::NoRelatedSym symbols_group;
		symbols_group.setField( Symbol( symbol ) );
		request.addGroup( symbols_group );

		// Add the NoMDEntryTypes group to the request for each MDEntryType
		// that we are subscribing to. This includes Bid, Offer, High, and Low
		FIX44::MarketDataRequest::NoMDEntryTypes entry_types;
		entry_types.setField( MDEntryType( MDEntryType_BID ) );
		request.addGroup( entry_types );
		entry_types.setField( MDEntryType( MDEntryType_OFFER ) );
		request.addGroup( entry_types );
		entry_types.setField( MDEntryType( MDEntryType_TRADING_SESSION_HIGH_PRICE ) );
		request.addGroup( entry_types );
		entry_types.setField( MDEntryType( MDEntryType_TRADING_SESSION_LOW_PRICE ) );
		request.addGroup( entry_types );

		Session::sendToTarget( request, getSessID( "market" ) );
	}

	/**
	 * Record an account id
	 * 
	 * @param std::string accountID 
	 */
	void FXCM::recordAccount(std::string accountID) {
		int size = (int) m_account_vec.size();
		if ( size == 0 ) {
			m_account_vec.push_back( accountID );
		} else {
			for ( int i = 0; i < size; i++ ) {
				if ( m_account_vec.at( i ) == accountID ) break;
					
				if ( i == size - 1 ) {
					m_account_vec.push_back( accountID );
				}
			}
		}
	}

	/**
	 * Generate string value used to populate the fields in each message
	 * which are used as a custom identifier
	 * 
	 * @return std::string
	 */
	std::string FXCM::nextRequestID() {
		if( m_request_id == 65535 )
			m_request_id = 1;

		m_request_id++;
		std::string next_ID = IntConvertor::convert( m_request_id );
		return next_ID;
	}
};

extern "C" {
	idefix::Adapter* create() {
		return new idefix::FXCM{};
	}
}