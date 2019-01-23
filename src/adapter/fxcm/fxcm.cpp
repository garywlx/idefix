#include "fxcm.h"
#include "../../core/logger.h"
#include "../../core/utility.h"

#include <quickfix/Utility.h>
#include <quickfix/Dictionary.h>
#include <quickfix/fix44/TradingSessionStatusRequest.h>
#include <quickfix/fix44/CollateralInquiry.h>
#include <quickfix/fix44/RequestForPositions.h>
#include <quickfix/fix44/MarketDataRequest.h>

#include <string>
#include <exception>

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
  			SPDLOG_INFO( "MarketData Session created." );
  		} else {
  			// this is the order session
  			m_sessions[ "order" ] = sessionID;
  			SPDLOG_INFO( "Order Session created." );
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
		sendTradingStatusRequest();
	}

	/**
	 * Notifies you when an FIX session is no longer online. This could happen during a normal logout
	 * exchange or because of a forced termination or a loss of network connection
     *
	 * @param const SessionID& sessionID
	 */
	void FXCM::onLogout(const SessionID &sessionID) {
		if ( sessionID.toString().find( "MD_" ) != string::npos ) {
			SPDLOG_INFO( "MarketSession logout." );
		} else {
			SPDLOG_INFO( "OrderSession logout." );
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

    		SPDLOG_WARN( "[fromAdmin:Reject] tagID {} msgType {}: {}", tagID, msgType, text );
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

		    SPDLOG_WARN( "[fromApp:Reject] tagID {} msgType {}: {}", tagID, msgType, text );
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
			trad_status = "open";
		} else if ( trad_status == "3" ) {
			trad_status = "closed";
		}

		SPDLOG_INFO( "TradingSessionStatus -> TradSesStatus - {}", trad_status );
		
		// Within the TradingSessionStatus message is an embeded SecurityList. From SecurityList we can see
		// the list of available trading securities and information relevant to each; e.g., point sizes,
		// minimum and maximum order quantities by security, etc. 
		SPDLOG_INFO( "SecurityList via TradingSessionStatus ->" );
		int symbols_count = IntConvertor::convert( tss.getField( FIELD::NoRelatedSym ) );
		for ( int i = 1; i <= symbols_count; i++ ) {
			// Get the NoRelatedSym group and for each, print out the Symbol value
			FIX44::SecurityList::NoRelatedSym symbols_group;
			tss.getGroup( i, symbols_group );
			string symbol = symbols_group.getField( FIELD::Symbol );
			
			SPDLOG_INFO( "  Symbol -> {}", symbol );
		}
		

		// Also within TradingSessionStatus are FXCM system parameters. This includes important information
		// such as account base currency, server time zone, the time at which the trading day ends, and more.
		SPDLOG_INFO( "System Parameters via TradingSessionStatus -> " );
		
		// Read field FXCMNoParam (9016) which shows us how many system parameters are 
		// in the message
		int params_count = IntConvertor::convert( tss.getField( FXCM_NO_PARAMS ) ); // FXCMNoParam (9016)
		for ( int i = 1; i < params_count; i++ ) {
			// For each paramater, print out both the name of the paramater and the value of the 
			// paramater. FXCMParamName (9017) is the name of the paramater and FXCMParamValue(9018)
			// is of course the paramater value
			FIX::FieldMap field_map = tss.getGroupRef( i, FXCM_NO_PARAMS );
			SPDLOG_INFO( "  Param {} = {}", field_map.getField( FXCM_PARAM_NAME ), field_map.getField( FXCM_PARAM_VALUE ) );
		}
		
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
		SPDLOG_INFO( "CollateralReport -> " );
		string accountID = cr.getField( FIELD::Account );
		// Get account balance, which is the cash balance in the account, not including any profit
		// or losses on open trades
		string balance = cr.getField( FIELD::CashOutstanding );
		SPDLOG_INFO( "  AccountID {}", accountID );
		SPDLOG_INFO( "  Balance {}", balance );

		// The CollateralReport NoPartyIDs group can be inspected for additional account information
		// such as AccountName or HedgingStatus
		FIX44::CollateralReport::NoPartyIDs group;
		cr.getGroup( 1, group); // CollateralReport will only have 1 NoPartyIDs group

		SPDLOG_INFO( "Parties ->" );
		// Get the number of NoPartySubIDs repeating groups
		int number_subID = IntConvertor::convert( group.getField( FIELD::NoPartySubIDs ) );
		// For each group, print out both the PartySubIDType and the PartySubID (the value)
		for ( int u = 1; u <= number_subID; u++ ) {
			FIX44::CollateralReport::NoPartyIDs::NoPartySubIDs sub_group;
			group.getGroup( u, sub_group );

			string sub_type = sub_group.getField( FIELD::PartySubIDType );
			string sub_value = sub_group.getField( FIELD::PartySubID );
			
			SPDLOG_INFO( "  {} -> {}", sub_type, sub_value );
		}

		// Add the accountID to our vector<string> being used to track all
		// accounts under our login
		recordAccount( accountID );
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
		string accountID     = pr.getField( FIELD::Account );
		string symbol        = pr.getField( FIELD::Symbol );
		string positionID    = pr.getField( FXCM_POS_ID );
		string pos_open_time = pr.getField( FXCM_POS_OPEN_TIME );
		SPDLOG_INFO( "PositionReport" );
		SPDLOG_INFO( "   Account -> {}", accountID );
		SPDLOG_INFO( "   Symbol -> {}", symbol );
		SPDLOG_INFO( "   PositionID -> {}", positionID );
		SPDLOG_INFO( "   Open Time -> {}", pos_open_time );
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
		SPDLOG_INFO( "MarketDataRequestReject -> " );
		
		if ( mdr.isSetField( FIELD::Text ) ) {
			SPDLOG_INFO( " Text -> {}", mdr.getField( FIELD::Text ) );
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
		string symbol = mds.getField( FIELD::Symbol );
		// Declare variables for both the bid and ask prices. We will read the MarketDataSnapshotFullRefresh
		// message for tthese values
		double bid_price = 0;
		double ask_price = 0;
		// For each MDEntry in the message, inspect the NoMDEntries group for
		// the presence of either the Bid or Ask (Offer) type 
		int entry_count = IntConvertor::convert( mds.getField( FIELD::NoMDEntries ) );
		for ( int i = 1; i < entry_count; i++ ) {
			FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
			mds.getGroup( i, group );
			string entry_type = group.getField( FIELD::MDEntryType );
			if ( entry_type == "0" ) { // Bid
				bid_price = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx ) );
			} else if ( entry_type == "1" ) { // Ask (Offer)
				ask_price = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx ) );
			}
		}

		SPDLOG_INFO( "MarketDataSnapshot {} Bid {.5f} Ask {.5f}", symbol, bid_price, ask_price );
	}

	/**
	 * If an order is executed this message pops up
	 * 
	 * @param const FIX44::ExecutionReport& er        
	 * @param const SessionID&              sessionID 
	 */
	void FXCM::onMessage(const FIX44::ExecutionReport& er, const SessionID& sessionID) {
		SPDLOG_INFO( "ExecutionReport" );
		SPDLOG_INFO( "  ClOrdID -> {}", er.getField( FIELD::ClOrdID ) );
		SPDLOG_INFO( "  Account -> {}", er.getField( FIELD::Account ) );
		SPDLOG_INFO( "  OrderID -> {}", er.getField( FIELD::OrderID ) );
		SPDLOG_INFO( "  LastQty -> {}", er.getField( FIELD::LastQty ) );
		SPDLOG_INFO( "  CumQty -> {}", er.getField( FIELD::CumQty ) );
		SPDLOG_INFO( "  ExecType -> {}", er.getField( FIELD::ExecType ) );
		SPDLOG_INFO( "  OrdStatus -> {}", er.getField( FIELD::OrdStatus ) );

		// ** Note on order status. ** 
		// In order to determine the status of an order, and also how much an order is filled, we must
		// use the OrdStatus and CumQty fields. There are 3 possible final values for OrdStatus: Filled (2),
		// Rejected (8), and Cancelled (4). When the OrdStatus field is set to one of these values, you know
		// the execution is completed. At this time the CumQty (14) can be inspected to determine if and how
		// much of an order was filled.
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
		m_initiator_ptr->stop();
		delete m_initiator_ptr;
		delete m_settings_ptr;
		delete m_store_factory_ptr;
		delete m_log_factory_ptr;
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