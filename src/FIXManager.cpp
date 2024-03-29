/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */
#include "Exceptions.h"
#include "FIXManager.h"
#include "MathHelper.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "CSVHandler.h"
#include <quickfix/Utility.h>
#include <quickfix/Dictionary.h>
#include <cmath>
#include <string>
#include <exception>

namespace IDEFIX {
/*!
 * Constructs FIXManager and starts a FIX Session from settings file
 *
 * @param const std::string settingsFile The FIX settings file
 */
FIXManager::FIXManager(): m_is_exiting( false ) {

#ifndef CMAKE_RELEASE_LOG
  // set up console
  m_console = spdlog::stdout_color_mt( "console" );
#else
  // overwrite console
  m_console = spdlog::daily_logger_mt( "console", "release.log", 0, 0 );
#endif

  // set console pattern
  m_console->set_pattern( "%Y-%m-%d %T.%e: %^%v%$" );
  
  // check if folder trades exists, if not create it
  file_mkdir( "trades/" );

  // set up trade log
  m_tradelog = spdlog::daily_logger_mt("tradelog", "trades/trades.log", 0, 0 );
  // set tradelog pattern
  m_tradelog->set_pattern( "%Y-%m-%d %T.%e,%v" );
  // flush logger every
  spdlog::flush_every( chrono::seconds( 3 ) );
  
}

/*!
 * Deconstructor FIXManager
 */
FIXManager::~FIXManager() {
  if ( m_pinitiator != nullptr ) {
    delete m_pinitiator;
  }
  if ( m_psettings != nullptr ) {
    delete m_psettings;
  }
  if ( m_pstore_factory != nullptr ) {
    delete m_pstore_factory;
  }
  if ( m_plog_factory != nullptr ) {
    delete m_plog_factory;
  }
}

// Gets called when quickfix creates a new session. A session comes into and remains in existence
// for the life of the application
void FIXManager::onCreate(const SessionID &sessionID) {
  console()->info( "[onCreate] send Logon(A) message." );
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
    console()->info( "[onLogon] {} (MarketSession)", getMarketSessionID().toString() );
    queryTradingStatus();
  }

  if( isOrderSession(sessionID) ){
    setOrderSessionID(sessionID);
    console()->info( "[onLogon] {} (OrderSession)", getOrderSessionID().toString() );
  }
}

// Notifies you when an FIX session is no longer online. This could happen during a normal logout
// exchange or because of a forced termination or a loss of network connection
void FIXManager::onLogout(const SessionID &session_ID) {
  // Session logout
  if( isMarketDataSession(session_ID) ){
    console()->info( "[onLogout] {} (MarketSession)", session_ID.toString() );
  }

  if( isOrderSession(session_ID) ){
    console()->info( "[onLogout] {} (OrderSession)", session_ID.toString() );
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
  
  string msgtype = message.getHeader().getField(FIELD::MsgType);
  if(MsgType_Reject == msgtype){
    string text = message.getField(FIELD::Text);
    string tagID = message.getField(371); // RefTagID
    string msgType = message.getField(372); // RefMsgType

    console()->warn( "[fromAdmin:Reject] tagID {} msgType {}: {}", tagID, msgType, text );
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
  // 
  string msgtype = message.getHeader().getField(FIELD::MsgType);
  if(MsgType_Reject == msgtype){
    string text = message.getField(FIELD::Text);
    string tagID = message.getField(371); // RefTagID
    string msgType = message.getField(372); // RefMsgType

    console()->warn( "[fromApp:Reject] tagID {} msgType {}: {}", tagID, msgType, text );
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
  console()->info( "[onMessage::TradingSessionStatus] Markets are {}", (trad_status == "2" ? "open" : "closed") );

  // Within the TradingSessionStatus message is an embeded SecurityList. From SecurityList we can see
  // the list of available trading securities and information relevant to each; e.g., point sizes,
  // minimum and maximum order quantities by security, etc.
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
  // Read field FXCMNoParam (9016) which shows us how many system parameters are in the message
  int params_count = IntConvertor::convert( tss.getField( FXCM_NO_PARAMS ) ); // FXCMNoParam (9016)
  for( int i = 1; i < params_count; i++ ) {
    // For each parameter, print out both the name of the parameter and the value of the parameter.
    // FXCMParamName (9017) is the name of the parameter and FXCMParamValue(9018) is of course the value
    FIX::FieldMap field_map = tss.getGroupRef( i, FXCM_NO_PARAMS );
    addSysParam( field_map.getField( FXCM_PARAM_NAME ), field_map.getField( FXCM_PARAM_VALUE ) );
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
    // if ( m_pstrategy != NULL ) {
    //   auto text = ack.getField( FIELD::Text );
    //   m_pstrategy->onRequestAck( *this, "CollateralInquiryAck", text );
    // }
  }

  if( ack.isSetField( FXCM_FIX_FIELDS::FXCM_REQUEST_REJECT_REASON ) ) {
    // Call strategy onRequestAck
    // if ( m_pstrategy != NULL ) {
    //   auto text = ack.getField( FXCM_FIX_FIELDS::FXCM_REQUEST_REJECT_REASON );
    //   m_pstrategy->onRequestAck( *this, "CollateralInquiryAck", text );
    // }
  }
}

// CollateralReport is a message containing important information for each account under the login.
// It is returned as a response to CollateralInquiry. You will receive a CollateralReport for each account
// under your login. Notable fields include Account(1) which is the AccountID and CashOutstanding(901) which
// is the account balance
void FIXManager::onMessage(const FIX44::CollateralReport &cr, const SessionID &session_ID) {
  std::shared_ptr<Account> account = std::make_shared<Account>();
  account->setAccountID( cr.getField( FIELD::Account ) );
  account->setBalance( DoubleConvertor::convert( cr.getField( FIELD::CashOutstanding ) ) );
  account->setMarginUsed( DoubleConvertor::convert( cr.getField( FXCM_FIX_FIELDS::FXCM_USED_MARGIN ) ) );
  account->setMarginRatio( DoubleConvertor::convert( cr.getField( FIELD::MarginRatio ) ) );
  //account->setContractSize( DoubleConvertor::convert( cr.getField( FIELD::Quantity ) ) );
  account->setMinTradeSize( DoubleConvertor::convert( cr.getField( FIELD::Quantity ) ) );

  // The CollateralReport NoPartyIDs group can be inspected for additional information such as AccountName
  // or HedgingStatus
  FIX44::CollateralReport::NoPartyIDs group;
  cr.getGroup(1, group); // CollateralReport will only have 1 NoPartyIDs group

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
      account->setHedging( (sub_value == "0" ? false : true ) );
    } 
    // securities account id
    else if ( sub_type == "2" ) {
      account->setSecuritiesAccountID( sub_value );
    }
    // Person lastname
    else if ( sub_type == "22" ) {
      account->setPerson( sub_value );
    }
  }

  // get base currency from system parameters
  account->setCurrency( getSysParam("BASE_CRNCY") );

  // set account
  setAccount( account );

  // output account
  console()->info( "[Account] {} Balance: {:.2f} {}", account->getAccountID(), account->getBalance(), account->getCurrency() );

  // if there are no market orders, set account free margin to account balance
  // otherwise free margin will be set inside function processMarketOrders
  if ( m_list_marketorders.empty() ) {
    // set account free margin to balance
    m_account->setFreeMargin( account->getBalance() );
  }

  // check if we are already initialized
  if ( m_symbol_subscriptions.empty() && ! isExiting() ) {
    // call init
    onInit();
  }
}

/*!
 * Is called if no positions are available
 * @param ack        [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::RequestForPositionsAck &ack, const SessionID &session_ID) {
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
    auto text = ack.getField( FIELD::Text );
    console()->info("[RequestForPositionsAck] {}", text);
  }
}

/*!
 * Is called if a PositionReport is available
 * @param pr         [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::PositionReport& pr, const SessionID& session_ID) {
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
    auto marketDetail = getMarketDetails( marketOrder.getSymbol() );
    marketOrder.setPrecision( marketDetail->getSymPrecision() );

    // add order to list
    addMarketOrder( marketOrder );

    // signal
    on_update_marketorder( marketOrder, MarketOrder::Status::NEW );

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
    console()->error("[MarketData] {}", mdr.getField( FIELD::Text ) );
    on_error( __FUNCTION__, mdr.getField( FIELD::Text ) );
    
    // Call strategy onRequestAck 
    // if ( m_pstrategy != NULL ) {
    //   auto text = mdr.getField( FIELD::Text );
    //   m_pstrategy->onRequestAck( *this, "MarketDataRequestReject", text );
    // }
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
  auto marketDetail = getMarketDetails( symbol );
  snapshot.setPrecision( marketDetail->getSymPrecision() );
  snapshot.setPointSize( marketDetail->getSymPointsize() );

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
}

/*!
 * Is called if an ExecutionReport for an order is available
 * 
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
  FIX::CumQty cumQty;
  
  string fxcm_pos_id = er.getField( FXCM_FIX_FIELDS::FXCM_POS_ID );
  string sendingTime = er.getHeader().getField( FIELD::SendingTime );

  er.get( execType );
  er.get( ordStatus );
  er.get( ordType );
  er.get( clOrdID );
  er.get( cumQty );
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
    marketOrder.setClosePrice( 0 );

    auto md = getMarketDetails( symbol.getValue() );
    marketOrder.setPrecision( md->getSymPrecision() );
    marketOrder.setPointSize( md->getSymPointsize() );

    // MassOrderStatus
    if ( execType == FIX::ExecType_NEW ) { // FIX::ExecType_ORDER_STATUS
      // get specific fields for OrderStatus
      FIX::OrderQty orderQty;
      er.get( orderQty );

      // MarketOrder with ExecType I = OrderStatus && OrdStatus 0 = New && OrdType 2 = Limit
      // update order in list, set take profit
      if ( ordStatus == FIX::OrdStatus_NEW && ordType == FIX::OrdType_LIMIT && orderQty.getValue() > 0 ) {
        // console output
        console()->info( "{} OrderStatus->updateMarketOrder (LIMIT) {} {} ", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
        // set take profit price.
        marketOrder.setTakePrice( marketOrder.getPrice() );
        // set OrderQty
        marketOrder.setQty( orderQty.getValue() );
        // update order in list.
        updateMarketOrder( marketOrder, true );
        // signal
        on_market_order( SignalType::MARKET_ORDER_SET_TP, marketOrder );
      }
      // MarketOrder with ExecType I = OrderStatus && OrdStatus 0 = New && OrdType 3 = Stop
      // update order in list, set stop loss
      else if ( ordStatus == FIX::OrdStatus_NEW && ordType == FIX::OrdType_STOP && orderQty.getValue() > 0 ) {
        // console output
        console()->info( "{} OrderStatus->updateMarketOrder (STOP) {} {}", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
        // set stop price.
        marketOrder.setStopPrice( marketOrder.getPrice() );
        // set OrderQty
        marketOrder.setQty( orderQty.getValue() );
        // update order in list.
        updateMarketOrder( marketOrder, true );
        // signal
        on_market_order( SignalType::MARKET_ORDER_SET_SL, marketOrder );
      }
    }
    // MarketOrder with ExecTyp F = Trade && OrdStatus 2 = Filled && OrdType 1 = Market
    // add new order in list
    else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_MARKET ) {
      // console output
      console()->info( "{} addMarketOrder {} {} fill @ {:.5f} {} qty {:.2f}", prefix, marketOrder.getSymbol(), marketOrder.getPosID(), marketOrder.getPrice(), marketOrder.getSideStr(), marketOrder.getQty() );
      // add order  
      addMarketOrder( marketOrder );
      // trade log
      tradelog( marketOrder );
      // signal
      on_market_order( SignalType::MARKET_ORDER_NEW, marketOrder );
    }
    // MarketOrder with ExecType F = Trade && OrdStatus 2 = Filled && OrdType 2 = Limit
    // take profit filled -> remove order from list
    else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_LIMIT ) {
      // console output
      console()->info( "{} removeMarketOrder (LIMIT) {} {}", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
      // trade log
      tradelog( marketOrder );
      // remove market order
      removeMarketOrder( marketOrder.getPosID() );
      // signal
      on_market_order( SignalType::MARKET_ORDER_TP_HIT, marketOrder );
    }
    // MarketOrder with ExecType F = Trade && OrdStatus 2 = Filled && OrdType 3 = Stop
    // stop loss filled -> remove order form list
    else if ( execType == FIX::ExecType_TRADE && ordStatus == FIX::OrdStatus_FILLED && ordType == FIX::OrdType_STOP ) { 
      // console output
      console()->info( "{} removeMarketOrder (STOP) {} {}", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
      // trade log
      tradelog( marketOrder );
      // remove market order
      removeMarketOrder( marketOrder.getPosID() );
      // signal
      on_market_order( SignalType::MARKET_ORDER_SL_HIT, marketOrder );
    }
    // MarketOrder with ExecType 4 = Canceled && OrdStatus 4 = Canceled 
    // remove order from list
    else if ( execType == FIX::ExecType_CANCELED && ordStatus == FIX::OrdStatus_CANCELED ) {
      // console output
      console()->info( "{} removeMarketOrder (CANCELED) {} {}", prefix, marketOrder.getPosID(), marketOrder.getSymbol() );
      // remove market order.
      removeMarketOrder( marketOrder.getPosID() );
      // signal
      on_market_order( SignalType::MARKET_ORDER_CANCELED, marketOrder );
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
  // if ( m_pstrategy != NULL && ack.isSetField( FIELD::Text ) ) {
  //   auto text = ack.getField( FIELD::Text );
  //   m_pstrategy->onRequestAck( *this, "AllocationReportAck", text );
  // }
}

// If allocation report is available
void FIXManager::onMessage(const FIX44::AllocationReport& ar, const SessionID& session_ID){
  
}

/*!
 * Start session and connect
 * 
 * @param const std::string settingsFile
 */
void FIXManager::connect(const std::string settingsFile) {
  try {
    // signal
    on_before_session_start();

    m_psettings = new SessionSettings( settingsFile );
    m_pstore_factory = new FileStoreFactory(*m_psettings);
    m_plog_factory = new FileLogFactory(*m_psettings);
    m_pinitiator = new SocketInitiator(*this, *m_pstore_factory, *m_psettings, *m_plog_factory/* Optional*/);
    m_pinitiator->start();
  } catch( ConfigError& error ){
    console()->error( "[connect:exception] {}", error.what() );
    on_error( __FUNCTION__, error.what() );
  } catch( ... ) {
    console()->error( "[connect:exception] unknown error." );
    on_error( __FUNCTION__, "unknown error." );
  }
}

// Logout and end session
void FIXManager::disconnect() {
  try {
    // signal
    on_before_session_end();

    // call stop method of socket initiator
    m_pinitiator->stop();  
  } catch( ConfigError& error ) {
    console()->error( "[disconnect:exception] unknown error." );
    on_error( __FUNCTION__, error.what() );
  } catch( ... ) {
    console()->error( "[disconnect:exception] unknown error." );
    on_error( __FUNCTION__, "unknown error." );
  }
}

// Sends TradingSessionStatusRequest message in order to receive as a response the
// TradingSessionStatus message
void FIXManager::queryTradingStatus() {
  try {
    auto request = FIXFactory::TradingSessionStatusRequest( nextRequestID() );
    Session::sendToTarget( request, getOrderSessionID() );  
  } catch( SessionNotFound& e ) {
    console()->error( "[queryTradingStatus] SessionNotFound");
    on_error( __FUNCTION__, "SessionNotFound" );
  } catch( ... ) {
    console()->error( "[queryTradingStatus] catch." );
    on_error( __FUNCTION__, "unknown error." );
  }
  
}

// Sends the CollateralInquiry message in order to receive as a response the CollateralReport message
void FIXManager::queryAccounts(){
  try {
    // Request CollateralReport message. We will receive a CollateralReport for each
    // account under our login
    auto request = FIXFactory::CollateralInquiry( nextRequestID() );
    Session::sendToTarget(request, getOrderSessionID());  
  } catch( SessionNotFound& e ) {
    console()->error( "[queryTradingStatus] SessionNotFound");
    on_error( __FUNCTION__, "SessionNotFound" );
  } catch( ... ) {
    console()->error( "[queryAccounts] catch." );
    on_error( __FUNCTION__, "unknown error." );
  }
  
}

/*!
 * Subscribe to market data for symbol
 * If symbol quote or base is not account currency, we will subscribe automatically
 * to the counter pair. E.g.: Symbol AUD/CAD, account = USD, subscribe to USD/CAD
 * 
 * @param const std::string symbol EUR/USD
 */
void FIXManager::subscribeMarketData(const std::string symbol) {
  console()->info( "[subscribeMarketData] {}", symbol );
  auto request = FIXFactory::MarketDataRequest( symbol, SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES );
  Session::sendToTarget( request, getMarketSessionID() );

  // add to subscriptions
  addSubscription( symbol );

  // check if we need a counter pair for price conversion
  auto counterPair = getCounterPair( symbol, getAccount()->getCurrency() );
  if ( ! counterPair.empty() && counterPair != symbol ) {
    if ( m_list_market.find( counterPair ) == m_list_market.end() ) {
      console()->info( "[subscribeMarketData] {} for price conversion of {}", counterPair, symbol );
      auto request2 = FIXFactory::MarketDataRequest( counterPair, SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES );
      Session::sendToTarget( request2, getMarketSessionID() );  
    }
  }
}

// Unsubscribe from the symbol trading security
void FIXManager::unsubscribeMarketData(const std::string symbol) {
  console()->info( "[unsubscribeMarketData] {}", symbol );

  auto request = FIXFactory::MarketDataRequest( symbol, SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT_PLUS_UPDATE_REQUEST );
  Session::sendToTarget( request, getMarketSessionID() );

  removeSubscription( symbol );

  // check base and quote of symbol
  // check if we need a counter pair for price conversion
  auto counterPair = getCounterPair( symbol, getAccount()->getCurrency() );
  if ( ! counterPair.empty() && counterPair != symbol ) {
    if ( m_list_market.find( counterPair ) != m_list_market.end() ) {
      console()->info( "[unsubscribeMarketData] {} for price conversion of {}", counterPair, symbol );
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
    // not found
    else {
      throw IDEFIX::element_not_found( __FILE__, __LINE__ );
    }
  } catch(IDEFIX::element_not_found* e) {
    console()->error( "[marketOrder:IDEFIX::element_not_found] {}", e->what() );
    on_error( __FUNCTION__, e->what() );
  } catch(std::exception& e){
    console()->error( "[marketOrder:exception] {}", e.what() );
    on_error( __FUNCTION__, e.what() );
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
  
  processMarketOrders( snapshot );

  // signal
  on_tick( snapshot );

} // - onMarketSnapshot

/*!
 * Calculate pip values and profit loss for open positions
 * 
 * @param const MarketSnapshot& snapshot The market snapshot.
 */
void FIXManager::processMarketOrders(const MarketSnapshot& snapshot) {
  FIX::Locker lock(m_mutex);
  if ( isExiting() ) return;
  
  if ( m_list_marketorders.empty() ) return;

  // get account currency
  auto accountCurrency = getAccount()->getCurrency();
  // get market detail for snapshot
  //@deprecated auto marketDetail = getMarketDetails( snapshot.getSymbol() );

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
          pip_value = Math::get_pip_value( snapshot, position.getQty(), accountCurrency, eurusd_snapshot->getAsk(), position.getSide() );
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
            conversion_price = eurcad_snapshot->getAsk();
          }
          // USD/CHF use EUR/CHF
          else if ( snapshot.getQuoteCurrency() == "CHF" ) {
            // get latest snapshot for EUR/CHF
            auto eurchf_snapshot = getLatestSnapshot( "EUR/CHF" );
            // set conversion price
            conversion_price = eurchf_snapshot->getAsk();
          }
          // USD/JPY use EUR/JPY
          else if ( snapshot.getQuoteCurrency() == "JPY" ) {
            // get latest snapshot for EUR/JPY
            auto eurjpy_snapshot = getLatestSnapshot( "EUR/JPY" );
            // set conversion price
            conversion_price = eurjpy_snapshot->getAsk();
          }

          // calculate pip value with conversion price
          pip_value = Math::get_pip_value( snapshot, position.getQty(), accountCurrency, conversion_price, position.getSide() );
        } // - else if base currency == USD
      } // - else if account currency == EUR

      // Calculate Profit / Loss
      const double profitloss = Math::get_profit_loss( pip_value, snapshot, position );
      
      // update position
      it->second.setProfitLoss( profitloss );
      position.setProfitLoss( profitloss );

      // Signal
      on_update_marketorder( position, MarketOrder::Status::UPDATE );

      // -----------------------------------------------------------------------------------------------------------------------
      // Update Account
      // -----------------------------------------------------------------------------------------------------------------------
      auto account = getAccount();
      // account equity
      account->setEquity( Math::get_equity( account->getBalance(), position.getProfitLoss() ) );
      // free margin
      auto free_margin = Math::get_free_margin( account->getBalance(), account->getEquity(), account->getMarginUsed() );
      account->setFreeMargin( free_margin );
      // margin ratio
      account->setMarginRatio( Math::get_margin_ratio( account->getEquity(), account->getFreeMargin() ) );
      // Signal
      on_account_change( account );
      
      // -----------------------------------------------------------------------------------------------------------------------
      // DEBUG output
      // -----------------------------------------------------------------------------------------------------------------------
      // console()->info( "---" );
      // console()->info( " PosID         {}", position.getPosID() );
      // console()->info( " Side          {}", position.getSideStr() );
      // console()->info( " Qty           {:.0f}", position.getQty() );
      // console()->info( " Open Price    {:.5f}", position.getPrice() );
      // console()->info( " Ask  Price    {:.5f}", snapshot.getAsk() );
      // console()->info( " Bid  Price    {:.5f}", snapshot.getBid() );      
      // console()->info( " PipValue      {:.5f}", pip_value );
      // console()->info( " PnL           {:.2f} {}", position.getProfitLoss(), accountCurrency );
      // console()->info( " Balance       {:.2f} {}", account.getBalance(), accountCurrency );
      // console()->info( " Equity        {:.2f} {}", account.getEquity(), accountCurrency );
      // console()->info( " Free Margin   {:.2f} {}", account.getFreeMargin(), accountCurrency );
      // console()->info( " Margin Ratio  {:.2f} %%", account.getMarginRatio() );
    } // - if snapshot symbol == position symbol
  } // - for marketorder loop
}

/*!
 * Returns last market snapshot for symbol
 * @param  symbol symbol
 * @return std::shared_ptr<MarketSnapshot> 
 */
std::shared_ptr<MarketSnapshot> FIXManager::getLatestSnapshot(const string symbol) {
  FIX::Locker lock( m_mutex );

  auto market = getMarket( symbol );
  if( market->isValid() ){
    return std::make_shared<MarketSnapshot>( market->getLatestSnapshot() );
  }

  return nullptr;
}

/*!
 * Send request to close all positions for symbol
 * @param symbol sring
 */
void FIXManager::closeAllPositions(const string symbol){
  if( m_list_marketorders.empty() ) return;

  console()->info( "[closeAllPositions] {}", symbol );
  
  for ( auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ) {
    if ( it->second.getSymbol() == symbol ){
      closePosition( it->second );
    }
  }
}

void FIXManager::closeAllPositions(const std::string symbol, const char side) {
  if ( m_list_marketorders.empty() ) return;

  console()->info( "[closeAllPositions] {} {}", symbol, side );

  for ( auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ) {
    if ( it->second.getSymbol() == symbol && it->second.getSide() == side ){
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

    std::ostringstream oss;
    oss << " - {} P&L {:.";
    oss << marketDetail->getSymPrecision();
    oss << "f} {}";

    console()->info( oss.str().c_str(), marketOrder.getPosID(), marketOrder.getProfitLoss(), getAccount()->getCurrency() );

    auto request = FIXFactory::NewOrderSingle( nextOrderID(), marketOrder, FIXFactory::SingleOrderType::CLOSEORDER );
    Session::sendToTarget(request, getOrderSessionID());  
  } catch(std::exception& e){
    on_error( __FUNCTION__, e.what() );
    console()->error( "[closePosition:exception] {}", e.what() );
  }
}

/*!
 * Send request to close each winning position
 * @param std::string symbol Close only positions for this symbol.
 */
void FIXManager::closeWinners(const string symbol) {
  if ( m_list_marketorders.empty() ) return;

  console()->info( "[closeWinners]" );
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

  console()->info( "[closeLoosers]" );
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
    console()->error( "[queryPositionReport:exception] {}", e.what() );
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
    console()->error( "[queryOrderMassStatus:exception] {}", e.what() );
  }
}

string FIXManager::getAccountID() const {
  return m_account->getAccountID();
}

/*!
 * Set Account
 * 
 * @param std::shared_ptr<Account> account
 */
void FIXManager::setAccount(std::shared_ptr<Account> account) {
  if ( m_account != account ) {
    m_account = account;
    // signal
    on_account_change( account );
  }
}

/*!
 * Get Account
 * 
 * @return
 */
std::shared_ptr<IDEFIX::Account> FIXManager::getAccount() {
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
 * @return std::shared_ptr<Market>|nullptr
 */
std::shared_ptr<Market> FIXManager::getMarket(const string& symbol) {
  FIX::Locker lock( m_mutex );
  map<string, Market>::iterator it = m_list_market.find( symbol );
  if( it != m_list_market.end() ){
    // found market
    return std::make_shared<Market>( it->second );
  }

  return nullptr;
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
  map<string, Market>::iterator marketIterator = m_list_market.find( snapshot.getSymbol() );
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
    // Send update signal
    on_update_marketorder( moIterator->second, MarketOrder::Status::REMOVED );
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
    MarketOrder second = moIterator->second;
    // update position because of take profit or stop loss.
    if( isUnsolicited ){
      // stoploss or takeprofit?
      if( second.getSide() == FIX::Side_BUY && rH.getPrice() > second.getPrice() ) {
        // take profit buy order
        second.setTakePrice( rH.getTakePrice() );
      } else if( second.getSide() == FIX::Side_BUY && rH.getPrice() < second.getPrice() ) {
        // stop loss buy order
        second.setStopPrice( rH.getStopPrice() );
      } else if( second.getSide() == FIX::Side_SELL && rH.getPrice() > second.getPrice() ) {
        // stop loss sell order
        second.setStopPrice( rH.getStopPrice() );
      } else if ( second.getSide() == FIX::Side_SELL && rH.getPrice() < second.getPrice() ) {
        // take profit sell order
        second.setTakePrice( rH.getTakePrice() );
      }
    } else {
      // Normal all field update
      second.setClOrdID( rH.getClOrdID() );
      second.setOrderID( rH.getOrderID() );
      second.setAccountID( rH.getAccountID() );
      second.setSymbol( rH.getSymbol() );
      second.setSendingTime( rH.getSendingTime() );
      second.setSide( rH.getSide() );
      second.setQty( rH.getQty() );
      second.setPrice( rH.getPrice() );
      second.setStopPrice( rH.getStopPrice() );
      second.setTakePrice( rH.getTakePrice() );
      second.setProfitLoss( rH.getProfitLoss() );
    }

    // signal
    on_update_marketorder( second, MarketOrder::Status::UPDATE );
  }
}

/*!
 * Returns the market order by fxcm_pos_id
 * @param const std::string fxcm_pos_id 
 * 
 * @return std::shared_ptr<MarketOrder>|nullptr
 */
std::shared_ptr<MarketOrder> FIXManager::getMarketOrder(const std::string fxcm_pos_id) const {
  FIX::Locker lock( m_mutex );
  for(auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ){
    if( it->second.getPosID() == fxcm_pos_id ){
      return std::make_shared<MarketOrder>( it->second );
      break;
    }
  }
  return nullptr;
}

/*!
 * Returns the market order by clOrdID
 * @param const ClOrdID clOrdID
 * 
 * @return std::shared_ptr<MarketOrder>|nullptr
 */
std::shared_ptr<MarketOrder> FIXManager::getMarketOrder(const ClOrdID clOrdID) const {
  FIX::Locker lock(m_mutex);
  for(auto it = m_list_marketorders.begin(); it != m_list_marketorders.end(); ++it ){
    if( it->second.getClOrdID() == clOrdID ){
      return std::make_shared<MarketOrder>( it->second );
      break;
    }
  }
  return nullptr;
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
 * @return std::shared_ptr<MarketDetail>
 */
std::shared_ptr<MarketDetail> FIXManager::getMarketDetails(const std::string& symbol) {
  FIX::Locker lock(m_mutex);
  
  for(auto it = m_market_details.begin(); it != m_market_details.end(); ++it) {
    if( it->first == symbol ) {
      return std::make_shared<MarketDetail>( it->second );
      break;
    }
  }

  return nullptr;
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

  console()->info( "[System Parameters]" );
  for ( auto it = m_system_params.begin(); it != m_system_params.end(); ++it ) {
    console()->info( " {} = {}", it->first, it->second );
  }
}

/*!
 * Show available market list 
 */
void FIXManager::showAvailableMarketList() {
  FIX::Locker lock(m_mutex);
  if ( m_market_details.empty() ) return;

  console()->info( "[Available Markets]" );
  for ( auto it = m_market_details.begin(); it != m_market_details.end(); ++it ) {
    console()->info( " {}", it->first );
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
    console()->info( "{}", it->second.toString() );
  } else {
    console()->warn( " -- not found --" );
  }
}

/*!
 * Call this, if you want to handle things after login, establishing sessions and receiving collateral report
 */
void FIXManager::onInit() {
  FIX::Locker lock( m_mutex );
  if ( ! isExiting() ) {
    // output 
    console()->info( "[INFO] - Press 0 to exit! -" );
  }

  // signal
  on_init();

  // Query position reports
  queryPositionReport();
}
/*!
 * Call this, if you want to handle things before exiting
 */
void FIXManager::onExit() {
  FIX::Locker lock( m_mutex );

  if ( ! isExiting() ) return;

  // signal
  on_exit();
}

/*!
 * Add symbol to subscription list
 * @param const std::string symbol
 */
void FIXManager::addSubscription(const string symbol) {
  FIX::Locker lock(m_mutex);
  if ( std::find( m_symbol_subscriptions.begin(), m_symbol_subscriptions.end(), symbol ) == m_symbol_subscriptions.end() ) {
    m_symbol_subscriptions.push_back( symbol );
  }
}

/*!
 * Remove symbol from subscription list
 * @param const std::string symbol
 */
void FIXManager::removeSubscription(const string symbol) {
  FIX::Locker lock(m_mutex);
  auto it = std::find( m_symbol_subscriptions.begin(), m_symbol_subscriptions.end(), symbol );
  if ( it != m_symbol_subscriptions.end() ) {
    m_symbol_subscriptions.erase( it );
  }
}

/*!
 * Get console shared pointer
 * 
 * @return std::shared_ptr<spdlog::logger>
 */
std::shared_ptr<spdlog::logger> FIXManager::console() {
  FIX::Locker lock( m_mutex );
  return m_console;
}

/*!
 * Log market order to trade log
 * 
 * @param const MarketOrder& marketOrder
 */
void FIXManager::tradelog(const MarketOrder& marketOrder) {
  FIX::Locker lock( m_mutex );
  
  try {
    std::stringstream filename_ss;
    filename_ss << marketOrder.getSymbol() << "_trades.csv";

    auto open_dt = FIX::UtcTimeStampConvertor::convert( marketOrder.getSendingTime() );
      std::stringstream open_ss;
      open_ss << open_dt.getYear() << "-" 
           << std::setfill('0') << std::setw(2) << open_dt.getMonth() << "-" 
           << std::setfill('0') << std::setw(2) << open_dt.getDay() << " " 
           << std::setfill('0') << std::setw(2) << open_dt.getHour() << ":" 
           << std::setfill('0') << std::setw(2) << open_dt.getMinute() << ":" 
           << std::setfill('0') << std::setw(2) << open_dt.getSecond() << "." << open_dt.getMillisecond();

    std::stringstream line_ss;
    // set pos id
    line_ss << marketOrder.getPosID()       << ",";
    // set symbol
    line_ss << marketOrder.getSymbol()      << ",";
    // set side char
    line_ss << marketOrder.getSideStr()     << ",";
    // set open time
    line_ss << open_ss.str()                << ",";
    // set open price
    line_ss << std::setprecision( marketOrder.getPrecision() ) << std::fixed << marketOrder.getPrice() << ",";
    // set stop price
    if ( marketOrder.getStopPrice() > 0 ) {
      line_ss << std::setprecision( marketOrder.getPrecision() ) << std::fixed << marketOrder.getStopPrice() << ",";  
    } else {
      line_ss << 0 << ",";
    }
    
    // line_ss << close_ss.str()               << ",";
    // set close price if available
    if ( marketOrder.getClosePrice() > 0 ) {
      line_ss << std::setprecision( marketOrder.getPrecision() ) << std::fixed << marketOrder.getClosePrice()  << ",";
      line_ss << Math::get_spread(marketOrder.getPrice(), marketOrder.getClosePrice(), marketOrder.getPointSize() ) << ",";  
    } else {
      line_ss << 0 << ",";
    }
    // set qty
    line_ss << marketOrder.getQty();

    // output
    CSVHandler csv;
    csv.set_path( "public_html/" );
    csv.set_filename( filename_ss.str() );
    csv.add_line( line_ss.str() );
  } catch ( std::exception& e ) {
    on_error( __FUNCTION__, e.what() );
  } catch ( ... ) {
    on_error( __FUNCTION__, "unknown error." );
  }
}

/*!
 * Get exiting status
 * 
 * @return bool
 */
bool FIXManager::isExiting() {
  return m_is_exiting;
}

/*!
 * Set exiting var. If set to true, the app will exit.
 * 
 * @param const bool status 
 */
void FIXManager::setExiting(const bool status) {
  if ( m_is_exiting != status ) {
    m_is_exiting = status;
  }

  if ( m_is_exiting ) {
    // Call onExit handler
    onExit();
  }
}

/*!
 * Get boolean if there are open positions for the symbol
 * 
 * @param const std::string symbol
 * @return bool
 */
bool FIXManager::hasOpenPositions(const std::string symbol) {
  FIX::Locker lock( m_mutex );

  if ( m_list_marketorders.empty() ) return false;

  int i = 0;
  for ( auto p : m_list_marketorders ) {
    if ( p.second.getSymbol() == symbol ) {
      i++;
    }
  }

  return i > 0;
}

}; // namespace idefix
