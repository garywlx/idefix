/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */
#include "FIXManager.h"
#include "Strategy.h"
#include <cmath>
#include <string>
#include <exception>

namespace IDEFIX {
/*!
 * Constructs FIXManager
 */
FIXManager::FIXManager(): m_debug_toggle_snapshot_output(false), m_debug_toggle_update_prices_output(false) {}

/*!
 * Constructs FIXManager and starts a FIX Session from settings file
 *
 * @param const std::string settingsFile The FIX settings file
 * @param Strategy*         strategy     The strategy to use
 */
FIXManager::FIXManager(const string settingsFile, Strategy* strategy): m_debug_toggle_snapshot_output(false), m_debug_toggle_update_prices_output(false) {
  startSession(settingsFile);
  setStrategy( strategy );
}

// Gets called when quickfix creates a new session. A session comes into and remains in existence
// for the life of the application
void FIXManager::onCreate(const SessionID &sessionID) {
  cout << "[onCreate] send Logon(A) message." << endl;
  // FIX Session created. We must now logon. Quickfix will automatically send
  // the Logon(A) message.
}

// Notifies you when a valid logon has been established with FXCM
void FIXManager::onLogon(const SessionID &sessionID) {
  // Session logon successful. Now we request TradingSessionStatus which is
  // used to determine market status (open or closed), to get a list of securities,
  // and to obtain important FXCM system parameters
  if( isMarketDataSession(sessionID) ){
    setMarketSessionID(sessionID);
    cout << "[onLogon] " << getMarketSessionID() << " (MarketDataSession)" << endl;

    queryTradingStatus();
  }

  if( isOrderSession(sessionID) ){
    setOrderSessionID(sessionID);
    cout << "[onLogon] " << getOrderSessionID() << " (OrderSession)" << endl;
  }
}

// Notifies you when an FIX session is no longer online. This could happen during a normal logout
// exchange or because of a forced termination or a loss of network connection
void FIXManager::onLogout(const SessionID &session_ID) {
  // Session logout
  if( isMarketDataSession(session_ID) ){
    cout << "[onLogout] MarketDataSession" << endl;
  }

  if( isOrderSession(session_ID) ){
    cout << "[onLogout] OrderSession" << endl;
  }
}

// Provides you with a peak at the administrative messages that are being sent from your FIX engine
// to FXCM
void FIXManager::toAdmin(Message &message, const SessionID &session_ID) {
  // If the Admin message being sent to FXCM is of type Logon(A), we want
  // to set the Username and Password fields. We want to catch this message as it
  // is going out.
  string msg_type = message.getHeader().getField(FIELD::MsgType);
  if(msg_type == "A"){
    // Get both username and password from our settings file. Then set these
    // respektive fields
    string str_username = m_psettings->get().getString("Username");
    string str_password = m_psettings->get().getString("Password");
    message.setField(Username(str_username));
    message.setField(Password(str_password));
  }
  // All messages sent to FXCM must contain the TargetSubID field (both Administrative and
  // Application messages). Here we set this.
  string sub_ID = m_psettings->get().getString("TargetSubID");
  message.getHeader().setField(TargetSubID(sub_ID));
}

// A callback for application messages that you are being sent to a counterparty
void FIXManager::toApp(Message &message, const SessionID &session_ID)
  throw( FIX::DoNotSend ) {
  // All messages sent to FXCM must contain the TargetSubID field (both Administrative and
  // Application messages).
  string sub_ID = m_psettings->get().getString("TargetSubID");
  message.getHeader().setField(TargetSubID(sub_ID));
}

// Notifies you when an administrative message is sent from FXCM to your FIX engine.
void FIXManager::fromAdmin(const Message &message, const SessionID &session_ID)
  throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon ) {
  
  // cout << "[fromAdmin] " << message << endl;
  string msgtype = message.getHeader().getField(FIELD::MsgType);
  if(MsgType_Reject == msgtype){
    string text = message.getField(FIELD::Text);
    string tagID = message.getField(371); // RefTagID
    string msgType = message.getField(372); // RefMsgType
    cout << "[fromAdmin:Reject] tagID " << tagID << " msgType " << msgType << ": " << text << endl;
  }

  // Call MessageCracker.crack method to handle the message by one of our
  // overloaded onMessage methods below
  crack(message, session_ID);
}

// One of the core entry points for your FIX application. Every application level request will come through here
void FIXManager::fromApp(const Message &message, const SessionID &session_ID)
  throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType ) {
  // Call MessageCracker.crack method to handle the message by one of our
  // overloaded onMessage methods below
  // cout << "[fromApp] " << message << endl;
  // 
  string msgtype = message.getHeader().getField(FIELD::MsgType);
  if(MsgType_Reject == msgtype){
    string text = message.getField(FIELD::Text);
    string tagID = message.getField(371); // RefTagID
    string msgType = message.getField(372); // RefMsgType
    cout << "[fromApp:Reject] tagID " << tagID << " msgType " << msgType << ": " << text << endl;
  }
  crack(message, session_ID);
}

// The TradingSessionStatus message is used to provide an update on the status of the market.
// Furthermore, this message contains useful system parameters as well as information about each
// trading security (embedded SecurityList). TradingSessionStatus should be requested upon successful Logon
// and subscribed to. The contents of the TradingSessionStatus message, specifically the SecurityList and
// system parameters, should dictate how fields are set when sending messages to FXCM.
void FIXManager::onMessage(const FIX44::TradingSessionStatus& tss, const SessionID& session_ID){

  // Check TradSesStatus field to see if the trading desk is open or closed
  // 2 = open; 3 = closed
  string trad_status = tss.getField(FIELD::TradSesStatus);
  cout << "[onMessage:TradingSessionStatus] Markets are " << (trad_status == "2" ? "open" : "closed") << endl;

  // Within the TradingSessionStatus message is an embeded SecurityList. From SecurityList we can see
  // the list of available trading securities and information relevant to each; e.g., point sizes,
  // minimum and maximum order quantities by security, etc.
  // cout << " SecurityList via TradingSessionStatus -> " << endl;
  int symbols_count = IntConvertor::convert( tss.getField( FIELD::NoRelatedSym ) );
  for(int i = 1; i <= symbols_count; i++){
    // Get the NoRelatedSym group and for each, print out the Symbol value
    FIX44::SecurityList::NoRelatedSym symbols_group;
    tss.getGroup(i, symbols_group);

    MarketDetail marketDetail;
    marketDetail.setSymbol( symbols_group.getField( FIELD::Symbol ) );
    marketDetail.setCurrency( symbols_group.getField( FIELD::Currency ) );
    marketDetail.setFactor( DoubleConvertor::convert( symbols_group.getField( FIELD::Factor ) ) );
    marketDetail.setContractMultiplier( DoubleConvertor::convert( symbols_group.getField( FIELD::ContractMultiplier ) ) );
    marketDetail.setProduct( IntConvertor::convert( symbols_group.getField( FIELD::Product ) ) );
    marketDetail.setRoundlot( DoubleConvertor::convert( symbols_group.getField( FIELD::RoundLot ) ) );

    // FXCM related
    marketDetail.setSymID( IntConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_ID ) ) );
    marketDetail.setSymPrecision( IntConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_PRECISION ) ) );
    marketDetail.setSymPointsize( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_POINT_SIZE ) ) );
    marketDetail.setSymInterestBuy( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_INTEREST_BUY ) ) );
    marketDetail.setSymInterestSell( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_INTEREST_SELL ) ) );
    marketDetail.setSymSortOrder( IntConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SYM_SORT_ORDER ) ) );
    marketDetail.setSubscriptionStatus( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_SUBSCRIPTION_STATUS ) );
    marketDetail.setFieldProductID( IntConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_FIELD_PRODUCT_ID ) ) );
    marketDetail.setCondDistStop( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_COND_DIST_STOP ) ) );
    marketDetail.setCondDistLimit( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_COND_DIST_LIMIT ) ) );
    marketDetail.setCondDistEntryStop( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_COND_DIST_ENTRY_STOP ) ) );
    marketDetail.setCondDistEntryLimit( DoubleConvertor::convert( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_COND_DIST_ENTRY_LIMIT ) ) );
    marketDetail.setTradingStatus( symbols_group.getField( FXCM_FIX_FIELDS::FXCM_TRADING_STATUS ) );

    addMarketDetail( marketDetail );
  }

  // Also within TradingSessionStatus are FXCM system parameters. This includes important information
  // such as account base currency, server time zone, the time at which the trading day ends, and more.
  //cout << "  System Parameters via TradingSessionStatus -> " << endl;
  // Read field FXCMNoParam (9016) which shows us how many system parameters are in the message
  int params_count = IntConvertor::convert( tss.getField( FXCM_NO_PARAMS ) ); // FXCMNoParam (9016)
  for( int i = 1; i < params_count; i++ ) {
    // For each parameter, print out both the name of the parameter and the value of the parameter.
    // FXCMParamName (9017) is the name of the parameter and FXCMParamValue(9018) is of course the value
    FIX::FieldMap field_map = tss.getGroupRef( i, FXCM_NO_PARAMS );
    addSysParam( field_map.getField( FXCM_PARAM_NAME ), field_map.getField( FXCM_PARAM_VALUE ) );

    // cout << field_map.getField(FXCM_PARAM_NAME) << " = " << field_map.getField(FXCM_PARAM_VALUE) << endl;
  }
 
  // Request accounts under our login
  queryAccounts();

  // ** Note on Text(58)
  // You will notice that Text(58) field is always set to "Market is closed. Any trading functionality
  // is not available." This field is always set to this value; therefore, do not use this field
  // to determine if the trading desk is open. As stated above, use TradSesStatus for this purpose
 }

// Used to respond to a Collateral Inquiry in the following situations:
// - When the CollateralInquiry will result in an out of band response (such as a file transfer).
// - When the inquiry is otherwise valid but no collateral is found to match the criteria specified on the Collateral Inquiry message.
// - When the Collateral Inquiry is invalid based upon the business rules of the counterparty.
void FIXManager::onMessage(const FIX44::CollateralInquiryAck& ack, const SessionID& session_ID){
  if( ack.isSetField( FIELD::Text ) ){
    // Call strategy onRequestAck
    if ( m_pstrategy != NULL ) {
      auto text = ack.getField( FIELD::Text );
      m_pstrategy->onRequestAck( *this, "CollateralInquiryAck", text );
    }
  }

  if( ack.isSetField( FXCM_FIX_FIELDS::FXCM_REQUEST_REJECT_REASON ) ) {
    // Call strategy onRequestAck
    if ( m_pstrategy != NULL ) {
      auto text = ack.getField( FXCM_FIX_FIELDS::FXCM_REQUEST_REJECT_REASON );
      m_pstrategy->onRequestAck( *this, "CollateralInquiryAck", text );
    }
  }
}

// CollateralReport is a message containing important information for each account under the login.
// It is returned as a response to CollateralInquiry. You will receive a CollateralReport for each account
// under your login. Notable fields include Account(1) which is the AccountID and CashOutstanding(901) which
// is the account balance
void FIXManager::onMessage(const FIX44::CollateralReport &cr, const SessionID &session_ID) {
  cout << "[onMessage:CollateralReport] " << endl;

  IDEFIX::Account account;
  account.setAccountID( cr.getField( FIELD::Account ) );
  account.setBalance( DoubleConvertor::convert( cr.getField( FIELD::CashOutstanding ) ) );
  account.setMarginUsed( DoubleConvertor::convert( cr.getField( FXCM_FIX_FIELDS::FXCM_USED_MARGIN ) ) );
  account.setMarginRatio( DoubleConvertor::convert( cr.getField( FIELD::MarginRatio ) ) );
  //account.setContractSize( DoubleConvertor::convert( cr.getField( FIELD::Quantity ) ) );
  account.setMinTradeSize( DoubleConvertor::convert( cr.getField( FIELD::Quantity ) ) );

  // The CollateralReport NoPartyIDs group can be inspected for additional information such as AccountName
  // or HedgingStatus
  FIX44::CollateralReport::NoPartyIDs group;
  cr.getGroup(1, group); // CollateralReport will only have 1 NoPartyIDs group
  //cout << "  Parties -> " << endl;

  // Get the number of NoPartySubIDs repeating groups
  int number_subID = IntConvertor::convert( group.getField( FIELD::NoPartySubIDs ) );

  // for each group, print out both the PartySubIDType and the PartySubID (the value)
  for(int u = 1; u <= number_subID; u++){
    FIX44::CollateralReport::NoPartyIDs::NoPartySubIDs sub_group;
    group.getGroup(u, sub_group);

    string sub_type = sub_group.getField(FIELD::PartySubIDType);
    string sub_value = sub_group.getField(FIELD::PartySubID);
    // hedging
    if( sub_type == "4000" ){
      // cout << "    Hedging? " << " -> " << (sub_value == "0" ? "Netting." : sub_value) << endl; 
      account.setHedging( (sub_value == "0" ? false : true ) );
    } 
    // securities account id
    else if ( sub_type == "2" ) {
      account.setSecuritiesAccountID( sub_value );
    }
    // Person lastname
    else if ( sub_type == "22" ) {
      account.setPerson( sub_value );
    }
  }

  // get base currency from system parameters
  account.setCurrency( getSysParam("BASE_CRNCY") );

  setAccount( account );

  // call init
  onInit();

  // call strategy on account change
  if ( m_pstrategy != NULL ) {
    m_pstrategy->onAccountChange( *this, account );
  }
}

/*!
 * Is called if no positions are available
 * @param ack        [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::RequestForPositionsAck &ack, const SessionID &session_ID) {
  string prefix = "[onMessage:RequestForPositionsAck] ";

  FIX::PosReqStatus posReqStatus;
  FIX::PosReqResult posReqResult;
  ack.get( posReqStatus );
  ack.get( posReqResult );

  // No Positions found
  if ( posReqStatus == FIX::PosReqStatus_REJECTED && posReqResult == FIX::PosReqResult_NO_POSITIONS_FOUND_THAT_MATCH_CRITERIA ) {
    // clear positions
    m_list_marketorders.clear();
  }
  // if a PositionReport is requested and no positions exist for that request, the Text field will
  // indicate that no positions matched the requested criteria
  else if( ack.isSetField(FIELD::Text) ){
    // Call strategy onRequestAck
    if ( m_pstrategy != NULL ) {
      auto text = ack.getField( FIELD::Text );
      m_pstrategy->onRequestAck( *this, "RequestForPositionsAck", text );
    }
  }
}

/*!
 * Is called if a PositionReport is available
 * @param pr         [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::PositionReport& pr, const SessionID& session_ID){

  // cout prefix
  string prefix = "[onMessage:PositionReport] ";
  // fxcm position id
  string fxcm_pos_id = pr.getField( FXCM_FIX_FIELDS::FXCM_POS_ID );
  // Set to 'Y' if message is sent as a result of a subscription request or out of band configuration as opposed to a Position Request (AN)
  // bool isUnsolicited = BoolConvertor::convert( pr.getField( FIELD::UnsolicitedIndicator ) );

  FIX::PosReqType posReqType;
  FIX::Account account;
  FIX::Symbol symbol;
  FIX::SettlPrice settlePrice;

  pr.get( posReqType );
  pr.get( account );
  pr.get( symbol );
  pr.get( settlePrice );

  // Position is open and is not SL or TP
  if ( ! fxcm_pos_id.empty() && posReqType == FIX::PosReqType_POSITIONS ) {
    MarketOrder marketOrder;
    marketOrder.setPosID( fxcm_pos_id );
    marketOrder.setSymbol( symbol.getValue() );
    marketOrder.setPrice( settlePrice.getValue() );
    marketOrder.setAccountID( account.getValue() );

    FIX44::PositionReport::NoPositions posGroup;
    pr.getGroup(1, posGroup);

    // Position is LONG
    if ( posGroup.isSetField( FIELD::LongQty ) ) {
      marketOrder.setSide( FIX::Side_BUY );
      marketOrder.setQty( DoubleConvertor::convert( posGroup.getField( FIELD::LongQty ) ) );  
    } 
    // Position is SHORT
    else if ( posGroup.isSetField( FIELD::ShortQty ) ){
      marketOrder.setSide( FIX::Side_SELL );
      marketOrder.setQty( DoubleConvertor::convert( posGroup.getField( FIELD::ShortQty ) ) );
    }

    marketOrder.setClOrdID( pr.getField( FIELD::ClOrdID ) );
    marketOrder.setOrderID( pr.getField( FIELD::OrderID ) );
    marketOrder.setSendingTime( pr.getField( FXCM_FIX_FIELDS::FXCM_POS_OPEN_TIME ) );

    // set precision
    MarketDetail marketDetail = getMarketDetails( marketOrder.getSymbol() );
    marketOrder.setPrecision( marketDetail.getSymPrecision() );

    // add order to list
    addMarketOrder( marketOrder );

    // Update strategy
    if ( m_pstrategy != NULL ) {
      m_pstrategy->onPositionChange( *this, marketOrder, MarketOrder::Status::NEW );
    }

    // queryOrderMassStatusRequest to update order with SL and TP values
    queryOrderMassStatus();
  }

}

/*!
 * Is called if a MarketDataRequest rejected
 * @param mdr        [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::MarketDataRequestReject &mdr, const SessionID &session_ID) {
  // If MarketDataRequestReject is returned as the result of a MarketDataRequest message
  // print out the contents of the Text field but first check that it is set
  if ( mdr.isSetField( FIELD::Text ) ) {
    // Call strategy onRequestAck
    if ( m_pstrategy != NULL ) {
      auto text = mdr.getField( FIELD::Text );
      m_pstrategy->onRequestAck( *this, "MarketDataRequestReject", text );
    }
  }
}

/*!
 * Is called if a marketdata snapshot is available
 * @param mds        [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::MarketDataSnapshotFullRefresh &mds, const SessionID &session_ID) {
  // Get symbol name of the snapshot; e.g. EUR/USD. Our example only subscribes to EUR/USD so
  // this is the only possible value
  string symbol = mds.getField(FIELD::Symbol);
  string entry_date = mds.getHeader().getField(FIELD::SendingTime);

  MarketSnapshot snapshot;
  snapshot.setSymbol( symbol );
  snapshot.setSendingTime( entry_date );
  // set precision
  MarketDetail marketDetail = getMarketDetails( symbol );
  snapshot.setPrecision( marketDetail.getSymPrecision() );
  snapshot.setPointSize( marketDetail.getSymPointsize() );

  // For each MDEntry in the message, inspect the NoMDEntries group for the presence of either the Bid or Ask
  // (Offer) type
  int entry_count = IntConvertor::convert( mds.getField( FIELD::NoMDEntries ) );
  for(int i = 1; i <= entry_count; i++){
    FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
    mds.getGroup( i, group );
    string entry_type = group.getField( FIELD::MDEntryType );
    // 0 Bid
    if(entry_type == MDEntryType( MDEntryType_BID ).getString() ){
      double bid = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx ) );
      snapshot.setBid( bid );
    } 
    // 1 Ask
    else if(entry_type == MDEntryType( MDEntryType_OFFER ).getString() ) {
      double ask = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx ) );
      snapshot.setAsk( ask );
    } 
    // 7 Session High
    else if(entry_type == MDEntryType( MDEntryType_TRADING_SESSION_HIGH_PRICE ).getString() ) {
      double high = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx) );
      snapshot.setSessionHigh( high );
    }
    // 8 Session Low
    else if(entry_type == MDEntryType( MDEntryType_TRADING_SESSION_LOW_PRICE ).getString() ) {
      double low = DoubleConvertor::convert( group.getField( FIELD::MDEntryPx) );
      snapshot.setSessionLow( low );
    }
  }

  // Add market snapshot for symbol snapshot.getSymbol()
  addMarketSnapshot( snapshot );
  // handle market snapshot
  onMarketSnapshot( snapshot );

  if( m_debug_toggle_snapshot_output ){
    cout << "[onMessage:MarketData] " << snapshot << endl;
  }
}

/*!
 * Is called if an ExecutionReport for an order is available
 * @param const FIX44::ExecutionReport er
 * @param const SessionID& session_ID
 */
void FIXManager::onMessage(const FIX44::ExecutionReport& er, const SessionID& session_ID) {

  string prefix = "[onMessage:ExecutionReport] ";

  FIX::ExecType execType;
  FIX::OrdStatus ordStatus;
  FIX::OrdType ordType;
  FIX::ClOrdID clOrdID;
  FIX::LastQty lastQty;
  FIX::LastPx lastPx;
  FIX::Side side;
  FIX::Symbol symbol;
  FIX::Account account;
  
  string fxcm_pos_id = er.getField( FXCM_FIX_FIELDS::FXCM_POS_ID );
  string sendingTime = er.getHeader().getField( FIELD::SendingTime );

  er.get( execType );
  er.get( ordStatus );
  er.get( ordType );
  er.get( clOrdID );
  er.get( lastQty );
  er.get( lastPx );
  er.get( side );
  er.get( symbol );
  er.get( account );
  
  // If we have a new market order with fxcm position id.
  if ( ! fxcm_pos_id.empty() ) {

    // create market order.
    MarketOrder marketOrder;
    marketOrder.setPosID( fxcm_pos_id );
    marketOrder.setSymbol( symbol.getValue() );
    marketOrder.setSide( side.getValue() );
    marketOrder.setPrice( lastPx.getValue() );
    marketOrder.setQty( lastQty.getValue() );
    marketOrder.setAccountID( account.getValue() );
    marketOrder.setClOrdID( clOrdID.getValue() );
    marketOrder.setSendingTime( sendingTime );
    marketOrder.setStopPrice( 0 );
    marketOrder.setTakePrice( 0 );

    // MassOrderStatus
    if ( execType == FIX::ExecType_ORDER_STATUS ) {
      // get specific fields for OrderStatus
      FIX::OrderQty orderQty;
      er.get( orderQty );

      // MarketOrder with ExecType I = OrderStatus && OrdStatus 0 = New && OrdType 2 = Limit
      // update order in list, set take profit
      if ( ordStatus == FIX::OrdStatus_NEW && ordType == FIX::OrdType_LIMIT ) {
        cout << prefix << "OrderStatus->updateMarketOrder (LIMIT)" << endl;
        // set take profit price.
        marketOrder.setTakePrice( marketOrder.getPrice() );
        // set OrderQty
        marketOrder.setQty( orderQty.getValue() );
        // update order in list.
        updateMarketOrder( marketOrder, true );
      }
      // MarketOrder with ExecType I = OrderStatus && OrdStatus 0 = New && OrdType 3 = Stop
      // update order in list, set stop loss
      else if ( ordStatus == FIX::OrdStatus_NEW && ordType == FIX::OrdType_STOP ) {
        cout << prefix << "OrderStatus->updateMarketOrder (STOP)" << endl;
        // set stop price.
        marketOrder.setStopPrice( marketOrder.getPrice() );
        // set OrderQty
        marketOrder.setQty( orderQty.getValue() );
        // update order in list.
        updateMarketOrder( marketOrder, true );
      }
    }
    // MarketOrder with ExecTyp F = Trade && OrdStatus 2 = Filled && OrdType 1 = Market
    // add new order in list
    else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_MARKET ) {
      cout << prefix << "addMarketOrder " << marketOrder.getPosID() << endl;
      addMarketOrder( marketOrder );
    }
    // MarketOrder with ExecType F = Trade && OrdStatus 2 = Filled && OrdType 2 = Limit
    // take profit filled -> remove order from list
    else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_LIMIT ) {
      cout << prefix << "removeMarketOrder (LIMIT) " << marketOrder.getPosID() << endl;
      removeMarketOrder( marketOrder.getPosID() );
    }
    // MarketOrder with ExecType F = Trade && OrdStatus 2 = Filled && OrdType 3 = Stop
    // stop loss filled -> remove order form list
    else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_STOP ) { 
      cout << prefix << "removeMarketOrder (STOP)" << marketOrder.getPosID() << endl;
      removeMarketOrder( marketOrder.getPosID() );
    }
    // MarketOrder with ExecType 0 = New && OrdStatus 0 = New && OrdType 2 = Limit
    // update order in list, set take profit
    /*else if ( execType == FIX::ExecType_NEW && ordStatus == FIX::OrdStatus_NEW && ordType == FIX::OrdType_LIMIT ) {
      cout << prefix << "updateMarketOrder with take profit" << endl;
      // set take profit price.
      marketOrder.setTakePrice( DoubleConvertor::convert( er.getField( FIELD::LastPx ) ) );
      // update market order.
      updateMarketOrder( marketOrder, true );
    }
    // MarketOrder with ExecType 0 = New && OrdStatus 0 = New && OrdType 3 = Stop
    // update order in list, set stop loss
    else if ( execType == FIX::ExecType_NEW && ordStatus == FIX::OrdStatus_NEW && ordType == FIX::OrdType_STOP ) {
      cout << prefix << "updateMarketOrder with stop loss" << endl;
      // set stop loss price.
      marketOrder.setStopPrice( DoubleConvertor::convert( er.getField( FIELD::LastPx ) ) );
      // update market order.
      updateMarketOrder( marketOrder, true );
    }*/
    // MarketOrder with ExecType 4 = Canceled && OrdStatus 4 = Canceled 
    // remove order from list
    else if ( execType == FIX::ExecType_CANCELED && ordStatus == FIX::OrdStatus_CANCELED ) {
      cout << prefix << "removeMarketOrder -> CANCELED" << endl;
      // remove market order.
      removeMarketOrder( marketOrder.getPosID() );
    }
  }
  
  // ** Note on order status. **
  // In order to determine the status of an order, and also how much an order is filled, we must
  // use the OrdStatus and CumQty fields. There are 3 possible final values for OrdStatus: Filled(2),
  // Rejected(8), and Cancelled(4). When the OrdStatus field is set to one of these values, you know
  // the execution is completed. At this time the CumQty (14) can be inspected to determine if and how
  // much of an order was filled.
}

// If no allocation report is available
void FIXManager::onMessage(const FIX44::AllocationReportAck& ack, const SessionID& session_ID){
  cout << "[onMessage::AllocationReportAck] " << ack << endl;
  if ( m_pstrategy != NULL && ack.isSetField( FIELD::Text ) ) {
    auto text = ack.getField( FIELD::Text );
    m_pstrategy->onRequestAck( *this, "AllocationReportAck", text );
  }
}

// If allocation report is available
void FIXManager::onMessage(const FIX44::AllocationReport& ar, const SessionID& session_ID){
  //cout << "[onMessage:AllocationReport] " << ar << endl;
}

// Starts the FIX session. Throws FIX::ConfigError exception if our configuration settings
// do not pass validation required to construct SessionSettings
void FIXManager::startSession(const string settingsfile) {
  try {
    m_psettings = new SessionSettings(settingsfile);
    m_pstore_factory = new FileStoreFactory(*m_psettings);
    m_plog_factory = new FileLogFactory(*m_psettings);
    m_pinitiator = new SocketInitiator(*this, *m_pstore_factory, *m_psettings, *m_plog_factory/* Optional*/);
    m_pinitiator->start();
  } catch(ConfigError& error){
    cout << "[startSession:exception] " << error.what() << endl;
  }
}

// Logout and end session
void FIXManager::endSession() {
  m_pinitiator->stop();
  delete m_pinitiator;
  delete m_psettings;
  delete m_pstore_factory;
  delete m_plog_factory;
}

// Sends TradingSessionStatusRequest message in order to receive as a response the
// TradingSessionStatus message
void FIXManager::queryTradingStatus() {
  auto request = FIXFactory::TradingSessionStatusRequest( nextRequestID() );
  Session::sendToTarget( request, getOrderSessionID() );
}

// Sends the CollateralInquiry message in order to receive as a response the CollateralReport message
void FIXManager::queryAccounts(){
  // Request CollateralReport message. We will receive a CollateralReport for each
  // account under our login
  auto request = FIXFactory::CollateralInquiry( nextRequestID() );
  Session::sendToTarget(request, getOrderSessionID());
}

/*!
 * Subscribe to market data for symbol
 * If symbol quote or base is not account currency, we will subscribe automatically
 * to the counter pair. E.g.: Symbol AUD/CAD, account = USD, subscribe to USD/CAD
 * 
 * @param const std::string symbol EUR/USD
 */
void FIXManager::subscribeMarketData(const std::string symbol) {
  cout << "--> subscribeMarketData( " << symbol << " )" << endl;
  auto request = FIXFactory::MarketDataRequest( symbol, SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES );
  Session::sendToTarget( request, getMarketSessionID() );

  // add to subscriptions
  addSubscription( symbol );

  // check if we need a counter pair for price conversion
  auto counterPair = getCounterPair( symbol, getAccount().getCurrency() );
  if ( counterPair != symbol ) {
    if ( m_list_market.find( counterPair ) == m_list_market.end() ) {
      cout << "--> subscribeMarketData( " << counterPair << " ) for price conversion of " << symbol << endl;
      auto request2 = FIXFactory::MarketDataRequest( counterPair, SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES );
      Session::sendToTarget( request2, getMarketSessionID() );  
    }
  }
}

// Unsubscribe from the symbol trading security
void FIXManager::unsubscribeMarketData(const std::string symbol) {
  cout << "--> unsubscribeMarketData( " << symbol << " )" << endl;
  auto request = FIXFactory::MarketDataRequest( symbol, SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT_PLUS_UPDATE_REQUEST );
  Session::sendToTarget( request, getMarketSessionID() );

  removeSubscription( symbol );

  // check base and quote of symbol
  // check if we need a counter pair for price conversion
  auto counterPair = getCounterPair( symbol, getAccount().getCurrency() );
  if ( counterPair != symbol ) {
    if ( m_list_market.find( counterPair ) != m_list_market.end() ) {
      cout << "--> unsubscribeMarketData( " << counterPair << " ) for price conversion of " << symbol << endl;
      auto request2 = FIXFactory::MarketDataRequest( symbol, SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT_PLUS_UPDATE_REQUEST );
      Session::sendToTarget( request2, getMarketSessionID() );
    }
  }
}

/*!
 * Sends a basic market order message to buy/sell at best market price
 * 
 * @param IDEFIX::MarketOrder marketOrder (Symbol, Qty, Side musst be set!)
 * @param FIXFactory::SingleOrderType orderType MARKET_ORDER|MARKET_ORDER_SL|MARKET_ORDER_SL_TP
 */
void FIXManager::marketOrder(const MarketOrder& marketOrder, const FIXFactory::SingleOrderType orderType) {
  try {
    // normal market order.
    if ( orderType == FIXFactory::SingleOrderType::MARKET_ORDER ) {
      auto request = FIXFactory::NewOrderSingle( nextOrderID(), marketOrder );
      Session::sendToTarget( request, getOrderSessionID() );  
    } 
    // market order with stoploss (OCO)
    else if ( orderType == FIXFactory::SingleOrderType::MARKET_ORDER_SL ) {
      const std::vector<std::string> reqIDs = { nextOrderID(), nextOrderID(), nextOrderID() };
      auto olist = FIXFactory::NewOrderList( reqIDs, marketOrder );
      Session::sendToTarget( olist, getOrderSessionID() );
    }
    // market order with stoploss and takeprofit (ELS)
    else if ( orderType == FIXFactory::SingleOrderType::MARKET_ORDER_SL_TP ) {
      const std::vector<std::string> reqIDs = { nextOrderID(), nextOrderID(), nextOrderID(), nextOrderID() };
      auto olist = FIXFactory::NewOrderList( reqIDs, marketOrder );
      Session::sendToTarget( olist, getOrderSessionID() );
    }
    // stop order
    else if ( orderType == FIXFactory::SingleOrderType::STOPORDER ) {
      auto request = FIXFactory::NewOrderSingle( nextOrderID(), marketOrder, orderType );
      Session::sendToTarget( request, getOrderSessionID() );
    }
  } catch(std::exception& e){
    cout << "[marketOrder:exception] " << e.what() << endl;
  }
}

// Generate string value used to identify the request
// On deconstruct the last value is gone, it restarts at 0
string FIXManager::nextRequestID() {
  return m_reqid_manager.nextRequestID();
}

// Generate string value used to identify the order request
// This is used in order requests only
// On deconstruct the last value is stored to a file, it restarts at last order id
string FIXManager::nextOrderID() {
  return m_reqid_manager.nextOrderID();
}

// Get the settings dictionary for the session
const FIX::Dictionary* FIXManager::getSessionSettingsPtr(const SessionID& session_ID){
  const FIX::Dictionary* pSettings = m_pinitiator->getSessionSettings(session_ID);
  return pSettings;
}

// Check if this session id belongs to the market data session
bool FIXManager::isMarketDataSession(const SessionID& session_ID){
  const FIX::Dictionary* pSettings = getSessionSettingsPtr(session_ID);
  return pSettings->has("MarketDataSession") && pSettings->getBool("MarketDataSession");
}

// Check if this session id belongs to the order session
bool FIXManager::isOrderSession(const SessionID& session_ID){
  const FIX::Dictionary* pSettings = getSessionSettingsPtr(session_ID);
  return pSettings->has("OrderSession") && pSettings->getBool("OrderSession");
}

/*!
 * Handle everything which relys on a new market snapshot
 * 
 * @param const MarketSnapshot& snapshot The current market snapshot.
 */
void FIXManager::onMarketSnapshot(const MarketSnapshot& snapshot) {
  
  // Process market orders
  processMarketOrders( snapshot );

  // Process candles
  processCandles( snapshot );

  // Process strategies
  processStrategy( snapshot );

} // - onMarketSnapshot

/*!
 * Calculate pip values and profit loss for open positions
 * 
 * @param const MarketSnapshot& snapshot The market snapshot.
 */
void FIXManager::processMarketOrders(const MarketSnapshot& snapshot) {
  FIX::Locker lock(m_mutex);
  if ( m_list_marketorders.empty() ) return;

  // get account currency
  auto accountCurrency = getAccount().getCurrency();
  // get market detail for snapshot
  auto marketDetail = getMarketDetails( snapshot.getSymbol() );

  for ( auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ) {
    // handle only positions with the same symbol trading
    if ( it->second.getSymbol() == snapshot.getSymbol() ) {
      // the position (MarketOrder)
      auto position = it->second;

      // -----------------------------------------------------------------------------------------------------------------------
      // Calculate Pip & Profit/Loss Value
      // -----------------------------------------------------------------------------------------------------------------------
      // the pip value
      double pip_value = 0;
      // the conversion price
      double conversion_price = 0;

      // calculate pip value for this snapshot
      if ( accountCurrency == "USD" ) {
        pip_value = Math::get_pip_value( snapshot, position.getQty(), accountCurrency, 0, position.getSide() );
      }
      // calculate pip value for EUR account
      else if ( accountCurrency == "EUR" ) {
        // get EUR/USD snapshot
        auto eurusd_snapshot = getLatestSnapshot( "EUR/USD" );

        // AUD/USD, EUR/USD, GBP/USD, NZD/USD, ...
        if ( snapshot.getQuoteCurrency() == "USD" ) {
          // calculate pip value
          pip_value = Math::get_pip_value( snapshot, position.getQty(), accountCurrency, eurusd_snapshot.getAsk(), position.getSide() );
        }
        // USD/CAD, USD/CHF, USD/JPY...
        else if ( snapshot.getBaseCurrency() == "USD" ) {
          // set conversion price 
          //conversion_price = 1 / eurusd_snapshot.getAsk();

          // USD/CAD use EUR/CAD
          if ( snapshot.getQuoteCurrency() == "CAD" ) {
            // get latest snapshot for EUR/CAD
            auto eurcad_snapshot = getLatestSnapshot( "EUR/CAD" );
            // set conversion price
            conversion_price = eurcad_snapshot.getAsk();
          }
          // USD/CHF use EUR/CHF
          else if ( snapshot.getQuoteCurrency() == "CHF" ) {
            // get latest snapshot for EUR/CHF
            auto eurchf_snapshot = getLatestSnapshot( "EUR/CHF" );
            // set conversion price
            conversion_price = eurchf_snapshot.getAsk();
          }
          // USD/JPY use EUR/JPY
          else if ( snapshot.getQuoteCurrency() == "JPY" ) {
            // get latest snapshot for EUR/JPY
            auto eurjpy_snapshot = getLatestSnapshot( "EUR/JPY" );
            // set conversion price
            conversion_price = eurjpy_snapshot.getAsk();
          }

          // calculate pip value with conversion price
          pip_value = Math::get_pip_value( snapshot, position.getQty(), accountCurrency, conversion_price, position.getSide() );
        } // - else if base currency == USD
      } // - else if account currency == EUR

      // Calculate Profit / Loss
      const double profitloss = Math::get_profit_loss( pip_value, snapshot, position );
      
      // update position
      position.setProfitLoss( profitloss );

      // -----------------------------------------------------------------------------------------------------------------------
      // Update Account
      // -----------------------------------------------------------------------------------------------------------------------
      auto account = getAccount();
      // account equity
      account.setEquity( Math::get_equity( account.getBalance(), position.getProfitLoss() ) );
      // free margin
      account.setFreeMargin( Math::get_free_margin( account.getBalance(), account.getEquity(), account.getMarginUsed() ) );
      // margin ratio
      account.setMarginRatio( Math::get_margin_ratio( account.getEquity(), account.getFreeMargin() ) );
      // -----------------------------------------------------------------------------------------------------------------------
      // Update Strategy -> onPosition
      // -----------------------------------------------------------------------------------------------------------------------      
      // @todo Update strategy onPosition
      
      // -----------------------------------------------------------------------------------------------------------------------
      // DEBUG output
      // -----------------------------------------------------------------------------------------------------------------------
      cout << "-----" << endl;
      // cout << " pip_v        " << pip_value << " " << accountCurrency << endl;
      // cout << " conversion_p " << conversion_price << endl;
      // cout << " profit_loss  " << profitloss << " " << accountCurrency << endl;
      cout << " PnL          " << position.getProfitLoss() << " " << accountCurrency << endl;
      cout << " Balance      " << account.getBalance() << " " << accountCurrency << endl;
      cout << " Equity       " << account.getEquity() << " " << accountCurrency << endl;
      cout << " Margin Used  " << account.getMarginUsed() << " " << accountCurrency << endl;
      cout << " Free Margin  " << account.getFreeMargin() << " " << accountCurrency << endl;
      cout << " Margin Ratio " << account.getMarginRatio() << " %" << endl;
    } // - if snapshot symbol == position symbol
  } // - for marketorder loop
}

/*!
 * Update strategies
 * 
 * @param const MarketSnapshot& snapshot The current market snapshot
 */
void FIXManager::processStrategy(const MarketSnapshot& snapshot) {
  if ( m_pstrategy != NULL ) {
    m_pstrategy->onTick(*this, snapshot);
  }
}

/*!
 * Update candle periods
 * 
 * @param const MarketSnapshot& snapshot The current market snapshot
 */
void FIXManager::processCandles(const MarketSnapshot& snapshot) {
  FIX::Locker lock(m_mutex);

  if ( m_list_candle_periods.empty() ) return;

  // look for candle periods for this symbol
  auto periods_it = m_list_candle_periods.find( snapshot.getSymbol() );
  if ( periods_it != m_list_candle_periods.end() ) {
    // get periods vector
    auto periods_list = periods_it->second;
    // iterate through periods and add tick
    for ( auto period_it = periods_list.begin(); period_it != periods_list.end(); ++period_it ) {
      addCandleTick( snapshot, *period_it );
    }
  }
}

/*!
 * Returns last market snapshot for symbol
 * @param  symbol symbol
 * @return        IDEFIX::MarketSnapshot
 */
MarketSnapshot FIXManager::getLatestSnapshot(const string symbol) {
  MarketSnapshot snapshot;
  Market market = getMarket(symbol);

  FIX::Locker lock(m_mutex);
  if( market.isValid() ){
    snapshot = market.getLatestSnapshot();
  }

  return snapshot;
}

/*!
 * Send request to close all positions for symbol
 * @param symbol sring
 */
void FIXManager::closeAllPositions(const string symbol){
  if( m_list_marketorders.empty() ) return;

  cout << "[closeAllPositions] " << endl;
  for ( auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ) {
    if ( it->second.getSymbol() == symbol ){
      closePosition( it->second );
    }
  }
}

/*!
 * Send market close order
 * @param marketOrder [description]
 */
void FIXManager::closePosition(const IDEFIX::MarketOrder &marketOrder){
  try {
    auto marketDetail = getMarketDetails( marketOrder.getSymbol() );
    cout << " - " << marketOrder.getPosID() << " P&L " << std::setprecision( marketDetail.getSymPrecision() ) << std::fixed << marketOrder.getProfitLoss() << " " << getAccount().getCurrency() << endl;

    auto request = FIXFactory::NewOrderSingle( nextOrderID(), marketOrder, FIXFactory::SingleOrderType::CLOSEORDER );
    Session::sendToTarget(request, getOrderSessionID());  
  } catch(std::exception& e){
    cout << "[closePosition:exception] " << e.what() << endl;
  }
}

/*!
 * Send request to close each winning position
 * @param std::string symbol Close only positions for this symbol.
 */
void FIXManager::closeWinners(const string symbol) {
  if ( m_list_marketorders.empty() ) return;

  cout << "[closeWinners]" << endl;
  for ( auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ) {
    if ( it->second.getSymbol() == symbol && it->second.getProfitLoss() > 0 ) {
      closePosition( it->second );
    }
  }
}

/*!
 * Send request to close each loosing position
 * @param std::string symbol Close only positions for this symbol.
 */
void FIXManager::closeLoosers(const std::string symbol) {
  if ( m_list_marketorders.empty() ) return;

  cout << "[closeLoosers]" << endl;
  for ( auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ) {
    if ( it->second.getSymbol() == symbol && it->second.getProfitLoss() < 0 ) {
      closePosition( it->second );
    }
  }
}

/*!
 * Sends RequestForPositions which will return PositionReport messages if positions
 * matching the requested criteria exist; otherwise, a RequestForPositionsAck will be
 * sent with the acknowledgement that no positions exist. 
 * @param type FIX::PosReqType http://fixwiki.fixtrading.org/index.php/PosReqType
 */
void FIXManager::queryPositionReport(const FIX::PosReqType type){
  try {
    auto request = FIXFactory::RequestForPositions( nextRequestID(), getAccountID(), type);
    Session::sendToTarget(request, getOrderSessionID());
  } catch( std::exception& e ){
    cout << "[queryPositionReport:exception] " << e.what() << endl;
  }
}

/*!
 * Sends OrderMassStatusRequest to get an ExecutionReport for every order at FXCM
 */
void FIXManager::queryOrderMassStatus() {
  try {
    auto request = FIXFactory::OrderMassStatusRequest( nextRequestID(), getAccountID() );  
    Session::sendToTarget( request, getOrderSessionID() );
  } catch( std::exception& e ){
    cout << "[queryOrderMassStatus:exception] " << e.what() << endl;
  }
}

void FIXManager::toggleSnapshotOutput(){
  m_debug_toggle_snapshot_output = ! m_debug_toggle_snapshot_output;
}

void FIXManager::togglePNLOutput() {
  m_debug_toggle_update_prices_output = ! m_debug_toggle_update_prices_output;
}

string FIXManager::getAccountID() const {
  return m_account.getAccountID();
}

/*!
 * Set Account
 * 
 * @param IDEFIX::Account account
 */
void FIXManager::setAccount(IDEFIX::Account account) {
  if ( m_account != account ) {
    m_account = account;
  }
}

/*!
 * Get Account
 * 
 * @return
 */
IDEFIX::Account FIXManager::getAccount() {
  return m_account;
}

SessionID FIXManager::getMarketSessionID() const {
  return m_market_sessionID;
}

void FIXManager::setMarketSessionID(const FIX::SessionID &session_ID){
  if( m_market_sessionID != session_ID ){
    m_market_sessionID = session_ID;
  }
}

SessionID FIXManager::getOrderSessionID() const {
  return m_order_sessionID;
}

void FIXManager::setOrderSessionID(const FIX::SessionID &session_ID){
  if( m_order_sessionID != session_ID ){
    m_order_sessionID = session_ID;
  }
}

/*!
 * Returns the market for given symbol
 * @param  symbol [description]
 * @return        [description] 
 */
Market FIXManager::getMarket(const string symbol) {
  Market market;
  FIX::Locker lock(m_mutex);
  map<string, Market>::iterator it = m_list_market.find(symbol);
  if( it != m_list_market.end() ){
    // found market
    market = it->second;
  }

  return market;
}

/*!
 * Add market to market list
 * @param Market market
 */
void FIXManager::addMarket(const Market market){
  FIX::Locker lock(m_mutex);
  if( m_list_market.count(market.getSymbol()) == 0 ){
    // add market
    m_list_market.insert( pair<string, Market>(market.getSymbol(), market) );
  }
}

/*!
 * Adds a snapshot to the market list
 * @param snapshot [description]
 */
void FIXManager::addMarketSnapshot(const MarketSnapshot snapshot){
  FIX::Locker lock(m_mutex);
  map<string, Market>::iterator marketIterator = m_list_market.find(snapshot.getSymbol());
  if( marketIterator != m_list_market.end() ){
    // found market, add snapshot
    marketIterator->second.add(snapshot);
  } else {
    // add market with snapshot
    Market market(snapshot);
    addMarket(market);
  }
}

/*!
 * Add a marketOrder to the order list
 * @param marketOrder [description]
 */
void FIXManager::addMarketOrder(const MarketOrder marketOrder){
  FIX::Locker lock(m_mutex);
  map<string, MarketOrder>::iterator moIterator = m_list_marketorders.find(marketOrder.getPosID());
  if( moIterator == m_list_marketorders.end() ){
    // posID not found
    m_list_marketorders.insert( pair<string, MarketOrder>(marketOrder.getPosID(), marketOrder ) );
  }
}

/*!
 * Remove MarketOrder from order list
 * 
 * @param posID FXCM position id
 */
void FIXManager::removeMarketOrder(const string posID){
  FIX::Locker lock(m_mutex);
  map<string, MarketOrder>::iterator moIterator = m_list_marketorders.find(posID);
  if( moIterator != m_list_marketorders.end() ){
    // Update strategy
    if ( m_pstrategy != NULL ) {
      m_pstrategy->onPositionChange( *this, moIterator->second, MarketOrder::Status::REMOVED );
    }
    // posID found, remove
    m_list_marketorders.erase( moIterator );
  }
}

/*!
 * Update take profit or stop loss field if isUnsolicitd = true, else update all fields
 * @param const MarketOrder& rH
 * @param const bool isUnsolicited set to true to update stop loss or take profit
 */
void FIXManager::updateMarketOrder(const MarketOrder& rH, const bool isUnsolicited){
  FIX::Locker lock(m_mutex);
  map<string, MarketOrder>::iterator moIterator = m_list_marketorders.find( rH.getPosID() );
  if( moIterator != m_list_marketorders.end() ){
    // original order
    MarketOrder lH = moIterator->second;
    // update position because of take profit or stop loss.
    if( isUnsolicited ){
      // stoploss or takeprofit?
      if( lH.getSide() == FIX::Side_BUY && rH.getPrice() > lH.getPrice() ) {
        // take profit buy order
        lH.setTakePrice( rH.getTakePrice() );
      } else if( lH.getSide() == FIX::Side_BUY && rH.getPrice() < lH.getPrice() ) {
        // stop loss buy order
        lH.setStopPrice( rH.getStopPrice() );
      } else if( lH.getSide() == FIX::Side_SELL && rH.getPrice() > lH.getPrice() ) {
        // stop loss sell order
        lH.setStopPrice( rH.getStopPrice() );
      } else if ( lH.getSide() == FIX::Side_SELL && rH.getPrice() < lH.getPrice() ) {
        // take profit sell order
        lH.setTakePrice( rH.getTakePrice() );
      }
    } else {
      // Normal all field update
      lH.setClOrdID( rH.getClOrdID() );
      lH.setOrderID( rH.getOrderID() );
      lH.setAccountID( rH.getAccountID() );
      lH.setSymbol( rH.getSymbol() );
      lH.setSendingTime( rH.getSendingTime() );
      lH.setSide( rH.getSide() );
      lH.setQty( rH.getQty() );
      lH.setPrice( rH.getPrice() );
      lH.setStopPrice( rH.getStopPrice() );
      lH.setTakePrice( rH.getTakePrice() );
      lH.setProfitLoss( rH.getProfitLoss() );
    }

    // Update strategy
    if ( m_pstrategy != NULL ) {
      m_pstrategy->onPositionChange( *this, lH, MarketOrder::Status::UPDATE );
    }
  }
}

/*!
 * Returns the market order by fxcm_pos_id
 * @param  fxcm_pos_id [description]
 * @return             [description]
 */
MarketOrder FIXManager::getMarketOrder(const string fxcm_pos_id) const {
  MarketOrder marketOrder;
  FIX::Locker lock(m_mutex);
  for(auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ){
    if( it->second.getPosID() == fxcm_pos_id ){
      marketOrder = it->second;
      break;
    }
  }
  return marketOrder;
}

/*!
 * Returns the market order by clOrdID
 * @param  clOrdID [description]
 * @return         [description]
 */
MarketOrder FIXManager::getMarketOrder(const ClOrdID clOrdID) const {
  MarketOrder marketOrder;
  FIX::Locker lock(m_mutex);
  for(auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ){
    if( it->second.getClOrdID() == clOrdID ){
      marketOrder = it->second;
      break;
    }
  }
  return marketOrder;
}

/*!
 * Add market detail object to list
 * @param const MarketDetail& marketDetail
 */
void FIXManager::addMarketDetail(const MarketDetail& marketDetail){
  FIX::Locker lock(m_mutex);
  if ( m_market_details.count(marketDetail.getSymbol()) == 0 ) {
    // add market detail
    m_market_details.insert( pair<std::string, MarketDetail>(marketDetail.getSymbol(), marketDetail) );
  }
}

/*!
 * Get MarketDetail for symbol
 * @param const std::string& symbol
 * @return
 */
MarketDetail FIXManager::getMarketDetails(const std::string& symbol) {
  FIX::Locker lock(m_mutex);
  MarketDetail marketDetail;
  for(auto it = m_market_details.begin(); it != m_market_details.end(); ++it) {
    if( it->first == symbol ) {
      marketDetail = it->second;
      break;
    }
  }
  return marketDetail;
}

/*!
 * Insert key,value pair to system params
 * @param key   [description]
 * @param value [description]
 */
void FIXManager::addSysParam(const string key, const string value){
  FIX::Locker lock(m_mutex);
  // check if the key already exisits
  auto it = m_system_params.find(key);
  if( it == m_system_params.end() ){
    m_system_params.insert( pair<string, string>(key, value) );
  }
}

/*!
 * Returns a reference to the element in m_system_params or an empty string
 * @param  key [description]
 * @return     [description]
 */
string FIXManager::getSysParam(const string key){
  FIX::Locker lock(m_mutex);
  string value;
  auto it = m_system_params.find(key);
  if( it != m_system_params.end() ){
    value = m_system_params.at(key);
  }
  return value;
}

/*!
 * Show system parameter list
 */
void FIXManager::showSysParamList() {
  FIX::Locker lock(m_mutex);
  if( m_system_params.empty() ) return;

  cout << "[System Parameters]" << endl;
  for ( auto it = m_system_params.begin(); it != m_system_params.end(); ++it ) {
    cout << "  " << it->first << " = " << it->second << endl;
  }
}

/*!
 * Show available market list 
 */
void FIXManager::showAvailableMarketList() {
  FIX::Locker lock(m_mutex);
  if ( m_market_details.empty() ) return;

  cout << "[Available Markets]" << endl;
  for ( auto it = m_market_details.begin(); it != m_market_details.end(); ++it ) {
    cout << "  " << it->first << endl;
  }
}

/*!
 * Show details for given market
 * 
 * @param const std::string symbol
 */
void FIXManager::showMarketDetail(const string symbol) {
  FIX::Locker lock(m_mutex);
  if ( m_market_details.empty() ) return;

  auto it = m_market_details.find( symbol );
  if( it != m_market_details.end() ) {
    cout << it->second << endl;
  } else {
    cout << " -- not found --" << endl;
  }
}

/*!
 * Returns a string with output format of base currency like 0.00 $
 * 
 * @param const double value The value to show.
 * @return std::string
 */
string FIXManager::formatBaseCurrency(const double value) {
  auto baseCrncyPrecision = IntConvertor::convert( getSysParam( "BASE_CRNCY_PRECISION" ) );
  auto baseCrncySymbol = getSysParam( "BASE_CRNCY_SYMBOL" );

  ostringstream os;
  os << std::setprecision( baseCrncyPrecision ) << std::fixed << value;

  string format;
  format.append( baseCrncySymbol ).append( " " ).append( os.str() );

  return format;
}

/*!
 * Call this, if you want to handle things after login, establishing sessions and receiving collateral report
 */
void FIXManager::onInit() {
  // check if we already initialized
  if( m_list_market.size() > 0 ) return;

  // call strategy init
  if ( m_pstrategy != NULL ) {
    m_pstrategy->onInit( *this );
  }
    
  queryPositionReport();
}
/*!
 * Call this, if you want to handle things before exiting
 */
void FIXManager::onExit() {
  // call strategy exit
  if ( m_pstrategy != NULL ) {
    m_pstrategy->onExit( *this );
  }
  
  // unsubscribe from all subscriptions
  if ( ! m_symbol_subscriptions.empty() ) {
    for( auto it = m_symbol_subscriptions.begin(); it != m_symbol_subscriptions.end(); ++it ) {
      unsubscribeMarketData( *it );
    }
  }
}

/*!
 * Add symbol to subscription list
 * @param const std::string symbol
 */
void FIXManager::addSubscription(const string symbol) {
  FIX::Locker lock(m_mutex);
  if ( std::find( m_symbol_subscriptions.begin(), m_symbol_subscriptions.end(), symbol ) != m_symbol_subscriptions.end() ) {
    m_symbol_subscriptions.push_back( symbol );
  }
}

/*!
 * Remove symbol from subscription list
 * @param const std::string symbol
 */
void FIXManager::removeSubscription(const string symbol) {
  FIX::Locker lock(m_mutex);
  for( auto it = m_symbol_subscriptions.begin(); it != m_symbol_subscriptions.end(); ++it ) {
    if ( *it == symbol ) {
      m_symbol_subscriptions.erase( it );  
    }
  }
}

/*!
 * Set a strategy
 * 
 * @param Strategy* strategy The Strategy to add
 */
void FIXManager::setStrategy(Strategy* strategy) {
  if ( strategy->getIdentifier().empty() ) {
    // throw error?
    return;
  }

  m_pstrategy = strategy;
}

/*!
 * Add candle period 
 * 
 * @param const std::string  symbol The symbol to add er period for
 * @param const unsigned int period The period in seconds
 */
void FIXManager::addCandlePeriod(const std::string symbol, const unsigned int period) {
  FIX::Locker lock(m_mutex);
  auto it = m_list_candle_periods.find( symbol );
  if ( it != m_list_candle_periods.end() ) {
    // add period to list
    auto periods = it->second;
    if ( std::find( periods.begin(), periods.end(), period ) == periods.end() ) {
      periods.push_back( period );
    }
  } else {
    // add new pair    
    std::vector<unsigned int> v;
    v.push_back( period );
    
    typedef std::pair<std::string, std::vector<unsigned int> > periodPair;
    m_list_candle_periods.insert( periodPair( symbol, v ) );
  }
}

/*!
 * Remove candle period abo, frees space by removing candles from stack
 * 
 * @param const std::string  symbol The symbol to remove the period
 * @param const unsigned int period The period to remove
 */
void FIXManager::remCandlePeriod(const std::string symbol, const unsigned int period) {
  FIX::Locker lock(m_mutex);
  auto periodit = m_list_candle_periods.find( symbol );
  if ( periodit != m_list_candle_periods.end() ) {
    // // remove candle data from candle list
    // auto candleit = m_list_candles.find( symbol );
    // // if candles exists
    // if ( candleit != m_list_candles.end() ) {
    //   // get candle list
    //   auto candle_list = candleit->second;
    //   // loop through all candles
    //   for ( auto candlei = 0; candlei < candle_list.size(); candlei++ ) {
    //     // check if the candle period is the same to remove
    //     auto candle = candle_list.at( candlei );
    //     if ( candle.getPeriod() == period ) {
    //       // remove candle
    //       candle_list.remove( candlei );
    //     } // - if candle period
    //   } // - for
    // } // if candleit != end
    
    // remove candle period
    auto period_list = periodit->second;
    auto period_it = std::find( period_list.begin(), period_list.end(), period );
    if ( period_it != period_list.end() ) {
      // erase
      period_list.erase( period_it );
    }
  } // if periodit != end
}

/*!
 * Add tick to candle from market snapshot
 * 
 * @param const MarketSnapshot& snapshot The market snapshot
 * @param const unsigned int    period   The candle period to add for
 */
void FIXManager::addCandleTick(const MarketSnapshot& snapshot, const unsigned int period) {
  FIX::Locker lock(m_mutex);
  
  // The tick to add
  Tick tick( snapshot.getBid(), snapshot.getAsk(), snapshot.getSendingTime() );

  // check if candle list already exists
  auto candleit = m_list_candles.find( snapshot.getSymbol() );
  if ( candleit != m_list_candles.end() ) {
    // get period list
    auto period_list = candleit->second;
    // check if period exists
    auto periodit = period_list.find( period );
    if ( periodit != period_list.end() ) {
      // get candle list
      auto candle_list = periodit->second;
      // get last candle
      auto candle = candle_list.back();

      // tick utc stimestamp
      FIX::UtcTimeStamp tickTime = UtcTimeStampConvertor::convert( tick.getSendingTime() );
      // check if tick time is less then ( last candle open time + period )
      FIX::UtcTimeStamp futureTime = UtcTimeStampConvertor::convert( candle.getOpenTime() );
      // add seconds
      futureTime += period;

      cout << "TickTime " << tickTime.getHour() << ":" << tickTime.getMinute() << ":" << tickTime.getSecond() << "." << tickTime.getFraction(3) << endl;
      cout << "FutuTime " << futureTime.getHour() << ":" << futureTime.getMinute() << ":" << futureTime.getSecond() << "." << futureTime.getFraction(3) << endl;

      if ( tickTime < futureTime ) {
        // add tick to candle
        candle.addTick( tick );
      }
      // else close candle
      else {
        // add new candle with tick
        candle.close( tick.getSendingTime() );
        // create new candle
        Candle next_candle( candle.getPeriod(), candle.getPrecision() );
        next_candle.addTick( tick );
        // add candle to stack
        candle_list.push_back( next_candle );
        // call strategy with new candle
        if ( m_pstrategy != NULL ) {
          m_pstrategy->onCandle( *this, next_candle );
        }
      } // else end
    } // if periodit != period_list end
  }
  // add symbol with fresh candle list
  else {

    // if checkTS second is not equal period seconds, return and wait for it
    FIX::UtcTimeStamp checkTS = UtcTimeStamp();
    if( checkTS.getSecond() != period ) {
      return;
    }

    cout << "Add next or first candle" << endl;
    // Next Candle
    Candle next_candle( period, snapshot.getPrecision() );
    next_candle.addTick( tick );

    // Next Candle List
    std::vector<Candle> next_candle_list;
    next_candle_list.push_back( next_candle );

    // Next Period Map -> Candle List
    map<unsigned int, std::vector<Candle> > next_period_map;
    next_period_map.insert( std::pair<unsigned int, std::vector<Candle> >( period, next_candle_list ) );

    // Add to total list
    m_list_candles.insert( std::pair<string, map<unsigned int, std::vector<Candle> > >( snapshot.getSymbol(), next_period_map ) );

    // call strategy with new candle
    if ( m_pstrategy != NULL ) {
      m_pstrategy->onCandle( *this, next_candle );
    }
  }
}
}; // namespace idefix