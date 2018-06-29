/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */
#include "FIXManager.h"
#include <cmath>
#include <string>
#include "TradeMath.h"

namespace IDEFIX {

// #define DEBUG_MARKETSNAPSHOT

/*!
 * Constructs FIXManager
 */
FIXManager::FIXManager(): m_debug_toggle_snapshot_output(false) {}

/*!
 * Constructs FIXManager and starts a FIX Session from settings file
 */
FIXManager::FIXManager(const string settingsFile): m_debug_toggle_snapshot_output(false) {
  startSession(settingsFile);
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
  
  //cout << "[fromAdmin] " << message << endl;
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
  cout << "[onMessage:TradingSessionStatus] " << (trad_status == "2" ? "open" : "closed") << endl;

  // Within the TradingSessionStatus message is an embeded SecurityList. From SecurityList we can see
  // the list of available trading securities and information relevant to each; e.g., point sizes,
  // minimum and maximum order quantities by security, etc.
  // cout << " SecurityList via TradingSessionStatus -> " << endl;
  // int symbols_count = IntConvertor::convert(tss.getField(FIELD::NoRelatedSym));
  // for(int i = 1; i <= symbols_count; i++){
  //   // Get the NoRelatedSym group and for each, print out the Symbol value
  //   FIX44::SecurityList::NoRelatedSym symbols_group;
  //   tss.getGroup(i, symbols_group);
  //   string symbol = symbols_group.getField(FIELD::Symbol);
  //   cout << "   Symbol -> " << symbol << endl;
  // }

  // Also within TradingSessionStatus are FXCM system parameters. This includes important information
  // such as account base currency, server time zone, the time at which the trading day ends, and more.
  cout << "  System Parameters via TradingSessionStatus -> " << endl;
  // Read field FXCMNoParam (9016) which shows us how many system parameters are in the message
  int params_count = IntConvertor::convert(tss.getField(FXCM_NO_PARAMS)); // FXCMNoParam (9016)
  for(int i = 1; i < params_count; i++){
    // For each parameter, print out both the name of the parameter and the value of the parameter.
    // FXCMParamName (9017) is the name of the parameter and FXCMParamValue(9018) is of course the value
    FIX::FieldMap field_map = tss.getGroupRef(i, FXCM_NO_PARAMS);
    cout << "     Param Name -> " << field_map.getField(FXCM_PARAM_NAME)
                                  << " - Param Value -> " << field_map.getField(FXCM_PARAM_VALUE) << endl;
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
  if( ack.isSetField(FIELD::Text) ){
    cout << "[onMessage:CollateralInquiryAck] " << ack.getField(FIELD::Text) << endl;
  }
}

// CollateralReport is a message containing important information for each account under the login.
// It is returned as a response to CollateralInquiry. You will receive a CollateralReport for each account
// under your login. Notable fields include Account(1) which is the AccountID and CashOutstanding(901) which
// is the account balance
void FIXManager::onMessage(const FIX44::CollateralReport &cr, const SessionID &session_ID) {
  cout << "[onMessage:CollateralReport] " << endl;

  string accountID = cr.getField(FIELD::Account);
  cout << "AccountID: " << accountID << endl;
  setAccountID(accountID);

  // Get account balance, which is the cash balance in the account, not including any profit or losses on open
  // trades
  string balance = cr.getField(FIELD::CashOutstanding);
  string margin = cr.getField(FXCM_USED_MARGIN);
  
//  cout << "    AccountID -> " << accountID << endl;
  cout << "    Balance -> " << balance << endl;
  cout << "    Margin -> " << margin << endl;

  // The CollateralReport NoPartyIDs group can be inspected for additional information such as AccountName
  // or HedgingStatus
  FIX44::CollateralReport::NoPartyIDs group;
  cr.getGroup(1, group); // CollateralReport will only have 1 NoPartyIDs group
  cout << "  Parties -> " << endl;

  // Get the number of NoPartySubIDs repeating groups
  int number_subID = IntConvertor::convert(group.getField(FIELD::NoPartySubIDs));

  // for each group, print out both the PartySubIDType and the PartySubID (the value)
  for(int u = 1; u <= number_subID; u++){
    FIX44::CollateralReport::NoPartyIDs::NoPartySubIDs sub_group;
    group.getGroup(u, sub_group);

    string sub_type = sub_group.getField(FIELD::PartySubIDType);
    string sub_value = sub_group.getField(FIELD::PartySubID);
    if( sub_type == "4000" ){
      cout << "    Hedging? " << " -> " << (sub_value == "0" ? "Netting." : sub_value) << endl; 
    } else {
      cout << "    " << sub_type << " -> " << sub_value << endl;  
    }
  }
}

/*!
 * Is called if no positions are available
 * @param ack        [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::RequestForPositionsAck &ack, const SessionID &session_ID) {
  string pos_reqID = ack.getField(FIELD::PosReqID);
  cout << "RequestForPositionsAck -> PosReqID - " << pos_reqID << endl;

  // if a PositionReport is requested and no positions exist for that rquest, the Text field will
  // indicate that no positions matched the requested criteria
  if( ack.isSetField(FIELD::Text) ){
    cout << "RequestForPositionAck -> Text - " << ack.getField(FIELD::Text) << endl;
  }
}

/*!
 * Is called if a PositionReport is available
 * @param pr         [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::PositionReport& pr, const SessionID& session_ID){
  // Print out important position related information such as accountID and symbol
  string accountID = pr.getField(FIELD::Account);
  string symbol = pr.getField(FIELD::Symbol);
  string clordID = pr.getField(FIELD::ClOrdID);
  string positionID = pr.getField(FXCM_POS_ID);
  string pos_open_time = pr.getField(FXCM_POS_OPEN_TIME);  
  string currency = pr.getField(FIELD::Currency);
  string settleprice = pr.getField(FIELD::SettlPrice);
  string pos_margin = pr.getField(FXCM_USED_MARGIN);

  bool isOpenPos = pr.getField(FIELD::PosReqType) == "0";

  cout << "[onMessage:PositionReport]" << endl;
  cout << "  PositionID  -> " << positionID << endl;
  cout << "  Account     -> " << accountID << endl;
  cout << "  Symbol      -> " << symbol << endl;
  cout << "  Open Time   -> " << pos_open_time << endl;
  cout << "  Currency    -> " << currency << endl;
  cout << "  SettlPrice  -> " << settleprice << endl;
  
  if( isOpenPos ){

    FIX44::PositionReport::NoPositions posGroup;
    pr.getGroup(1, posGroup);

    string qty;
    Side side;
    if(posGroup.isSetField(FIELD::LongQty)) {
      side = Side(FIX::Side_BUY);
      qty = posGroup.getField(FIELD::LongQty);
    } else if(pr.isSetField(FIELD::ShortQty)){
      side = Side(FIX::Side_SELL);
      qty = posGroup.getField(FIELD::ShortQty);
    }

    cout << "  Margin      -> " << pos_margin << endl;
    cout << "  Qty         -> " << qty << endl;
    cout << "  Side        -> " << (side.getValue() == Side_BUY ? "Buy" : "Sell") << endl;  
  }

  // only for closed positions
  if( ! isOpenPos ){
    string pos_close_price = pr.getField(FXCM_CLOSE_SETTLE_PRICE);
    string pos_close_pl = pr.getField(FXCM_CLOSE_PNL);  
    cout << "  P/L -> " << pos_close_pl << endl;  
    cout << "  ClosePrice -> " << pos_close_price << endl;
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
  cout << "[onMessage:MarketDataRequestReject] " << endl;
  if(mdr.isSetField(FIELD::Text)){
    cout << " Text -> " << mdr.getField(FIELD::Text) << endl;
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
  snapshot.setSymbol(symbol);
  snapshot.setSendingTime(entry_date);
  // @todo set this value by FXCM_SYM_PRECISION
  snapshot.setPrecision(5);

  // For each MDEntry in the message, inspect the NoMDEntries group for the presence of either the Bid or Ask
  // (Offer) type
  int entry_count = IntConvertor::convert(mds.getField(FIELD::NoMDEntries));
  for(int i = 1; i <= entry_count; i++){
    FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
    mds.getGroup(i, group);
    string entry_type = group.getField(FIELD::MDEntryType);
    // 0 Bid
    if(entry_type == MDEntryType(MDEntryType_BID).getString()){
      double bid = DoubleConvertor::convert(group.getField(FIELD::MDEntryPx));
      // cout << " MDEntryType_BID " << fixed << bid;
      snapshot.setBid(bid);
    } 
    // 1 Ask
    else if(entry_type == MDEntryType(MDEntryType_OFFER).getString()) {
      double ask = DoubleConvertor::convert(group.getField(FIELD::MDEntryPx));
      // cout << " MDEntryType_OFFER " << fixed << ask;
      snapshot.setAsk(ask);
    } 
    // 7 Session High
    else if(entry_type == MDEntryType(MDEntryType_TRADING_SESSION_HIGH_PRICE).getString()) {
      double high = DoubleConvertor::convert(group.getField(FIELD::MDEntryPx));
      // cout << " MDEntryType_TRADING_SESSION_HIGH_PRICE " << fixed << high;
      snapshot.setSessionHigh(high);
    }
    // 8 Session Low
    else if(entry_type == MDEntryType(MDEntryType_TRADING_SESSION_LOW_PRICE).getString()) {
      double low = DoubleConvertor::convert(group.getField(FIELD::MDEntryPx));
      // cout << " MDEntryType_TRADING_SESSION_LOW_PRICE " << fixed << low;
      snapshot.setSessionLow(low);
    }
  }

  // cout << endl;

  // Add market snapshot for symbol snapshot.getSymbol()
  addMarketSnapshot(snapshot);
  // save/update last snapshot
  updatePrices(snapshot);

  if( m_debug_toggle_snapshot_output ){
    cout << "[onMessage:MarketData] " << snapshot << endl;
  }
}

/*!
 * Is called if an ExecutionReport for a new order is available
 * @param er         [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::ExecutionReport &er, const SessionID &session_ID) {

  string coutPrefix = "[onMessage:ExecutionReport] ";

  FIX::ExecType execType;
  er.get(execType);
  
  FIX::ClOrdID clOrdID;
  er.get(clOrdID);

  // What kind of execution report is this?
  if(FIX::ExecType_FILL == execType.getValue() || FIX::ExecType_PARTIAL_FILL == execType.getValue()){
    // Fill || Partial Fill
    FIX::LastPx lastPx;
    er.get(lastPx);
    FIX::OrdStatus ordStatus;
    er.get(ordStatus);
    string posid = er.getField(FXCM_POS_ID);

    if( posid.empty() ){
      cout << coutPrefix << "FXCM_POS_ID is empty." << endl;
    }

    // add order to list?
    // @todo

  } else if(FIX::ExecType_REJECTED == execType.getValue()){
    // Rejected
    cout << coutPrefix << "REJECTED: " << er << endl;
  } else if(FIX::ExecType_CANCELED == execType.getValue()){
    // Cancelled
    cout << coutPrefix << "CANCELLED: " << er << endl;
  } else if(FIX::ExecType_NEW == execType.getValue()){
    // New order was accepted but has not yet been filled
  } else {
    // No idea
    cout << coutPrefix << "execType: " << execType << ": " << er << endl;
  }

  /*// Get status from field
  string status = er.getField(FIELD::OrdStatus);
  string statustext;
  string clordid = er.getField(FIELD::ClOrdID);
  string posid = er.getField(FXCM_POS_ID);

  switch(stoi(status)){
    case 2: // Filled
      statustext = "Filled";

      // move market order from pending to filled
      for(auto it = m_list_pending_orders.begin(); it !=  m_list_pending_orders.end(); ++it ){
        if( (*it).clOrdID == ClOrdID(clordid) ){

          (*it).ordStatus = OrdStatus(OrdStatus_FILLED);
          (*it).posID = posid;

          // add to filled orders
          m_list_filled_orders.push_back(*it);
          // remove from pending orders
          m_list_pending_orders.erase(it);
          break;
        }
      }

    break;
    case 8: // Cancelled
      statustext = "Cancelled";

      removeMarketOrderFromPendingList(clordid);
    break;
    case 4: // Rejected
      statustext = "Rejected";

      removeMarketOrderFromPendingList(clordid);
    break;
    default:
      statustext = status;
    break;
  }

  cout << "[onMessage:ExecutionReport]" << endl;
  cout << "  OrdStatus -> " << status << " " << statustext << endl;
  cout << "  Text -> " << er.getField(FIELD::Text) << endl;
  cout << "  ClOrdID -> " << clordid << endl;
  cout << "  Account -> " << er.getField(FIELD::Account) << endl;
  cout << "  OrderID -> " << er.getField(FIELD::OrderID) << endl;
  cout << "  LastQty -> " << er.getField(FIELD::LastQty) << endl;
  cout << "  ExecType -> " << er.getField(FIELD::ExecType) << endl;
  cout << "  CumQty  -> " << er.getField(FIELD::CumQty) << endl;*/

  // ** Note on order status. **
  // In order to determine the status of an order, and also how much an order is filled, we must
  // use the OrdStatus and CumQty fields. There are 3 possible final values for OrdStatus: Filled(2),
  // Rejected(8), and Cancelled(4). When the OrdStatus field is set to one of these values, you know
  // the execution is completed. At this time the CumQty (14) can be inspected to determine if and how
  // much of an order was filled.
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
    cout << error.what() << endl;
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
  // Request TradingSessionStatus message
  FIX44::TradingSessionStatusRequest request;
  request.setField(TradSesReqID(nextRequestID()));
  request.setField(TradingSessionID("FXCM"));
  request.setField(SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT));

  Session::sendToTarget(request, getOrderSessionID());
}

// Sends the CollateralInquiry message in order to receive as a response the CollateralReport message
void FIXManager::queryAccounts(){
  // Request CollateralReport message. We will receive a CollateralReport for each
  // account under our login
  FIX44::CollateralInquiry request;
  request.setField(CollInquiryID(nextRequestID()));
  request.setField(TradingSessionID("FXCM"));
  request.setField(SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT));

  Session::sendToTarget(request, getOrderSessionID());
}

/*!
 * Subscribe and Unsubscribe to/from MarketData
 * @param symbol      FIX::Symbol
 * @param requestType FIX::SubscriptionRequestType
 */
void FIXManager::queryMarketData(const FIX::Symbol symbol, const FIX::SubscriptionRequestType requestType){
  FIX44::MarketDataRequest request;
  request.setField(MDReqID("Request_" + symbol.getValue()));
  request.setField(requestType);
  request.setField(MarketDepth(0));
  request.setField(NoRelatedSym(1));

  // Add the NoRelatedSym group to the request with Symbol
  // field set to EUR/USD
  FIX44::MarketDataRequest::NoRelatedSym symbols_group;
  symbols_group.setField(symbol);
  request.addGroup(symbols_group);

  // Add the NoMDEntryTypes group to the request for each MDEntryType
  // that we are subscribing to. This includes Bid, Offer, High and Low
  FIX44::MarketDataRequest::NoMDEntryTypes entry_types;
  entry_types.setField(MDEntryType(MDEntryType_BID));
  request.addGroup(entry_types);
  entry_types.setField(MDEntryType(MDEntryType_OFFER));
  request.addGroup(entry_types);
  entry_types.setField(MDEntryType(MDEntryType_TRADING_SESSION_HIGH_PRICE));
  request.addGroup(entry_types);
  entry_types.setField(MDEntryType(MDEntryType_TRADING_SESSION_LOW_PRICE));
  request.addGroup(entry_types);

  Session::sendToTarget(request, getMarketSessionID());
}

// Subscribes to the EUR/USD trading security
void FIXManager::subscribeMarketData(const FIX::Symbol symbol) {
  queryMarketData(symbol, SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));
}

// Unsubscribe from the EUR/USD trading security
void FIXManager::unsubscribeMarketData(const FIX::Symbol symbol) {
  queryMarketData(symbol, SubscriptionRequestType(
      SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT_PLUS_UPDATE_REQUEST));
}

/*!
 * Sends a basic market order message to buy/sell at best market price
 * @param symbol [description]
 * @param side   [description]
 * @param qty    [description]
 */
void FIXManager::marketOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty) {
  FIX44::NewOrderSingle request;
  request.setField(ClOrdID(nextOrderID()));
  request.setField(Account(getAccountID()));
  request.setField(symbol);
  request.setField(TradingSessionID("FXCM"));
  request.setField(TransactTime());
  request.setField(OrdType(FIX::OrdType_MARKET));
  request.setField(OrderQty(qty));
  request.setField(Side(side));
  request.setField(OrdType(OrdType_MARKET));
  request.setField(TimeInForce(TimeInForce_FILL_OR_KILL));
  
  Session::sendToTarget(request, getOrderSessionID());
}

/*!
 * Send stop/take profit order at given price with fill or kill validity
 * @param symbol [description]
 * @param side   [description]
 * @param qty    [description]
 * @param price  [description]
 */
void FIXManager::stopOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty, const double price){
  FIX44::NewOrderSingle request;
  request.setField(ClOrdID(nextOrderID()));
  request.setField(Account(getAccountID()));
  request.setField(Symbol(symbol));
  request.setField(TradingSessionID("FXCM"));
  request.setField(TransactTime());
  request.setField(OrdType(FIX::OrdType_STOP));
  request.setField(OrderQty(qty));
  request.setField(Side(side));
  request.setField(StopPx(price));
  request.setField(PositionEffect(PositionEffect_CLOSE));

  Session::sendToTarget(request, getOrderSessionID());
}

/*!
 * Send market order with stoploss
 * @param marketOrder
 */
void FIXManager::marketOrderWithStoploss(MarketOrder& marketOrder){
  FIX44::NewOrderList olist;

  olist.setField(FIX::ListID(nextOrderID()));
  olist.setField(FIX::TotNoOrders(2));
  olist.setField(FIELD::ContingencyType, "101");

  // Order
  FIX44::NewOrderList::NoOrders order;
  order.setField(ClOrdID(nextOrderID()));
  order.setField(ListSeqNo(0));
  order.setField(ClOrdLinkID("1")); // link to another clordid in list
  order.setField(Account(getAccountID()));
  order.setField(marketOrder.getSymbol());
  order.setField(marketOrder.getSide());
  order.setField(marketOrder.getQty());
  order.setField(OrdType(OrdType_MARKET));
  olist.addGroup(order);

  // StopLoss
  FIX44::NewOrderList::NoOrders stop;
  stop.setField(ClOrdID(nextOrderID()));
  stop.setField(ListSeqNo(1));
  stop.setField(ClOrdLinkID("2"));
  stop.setField(Account(getAccountID()));
  stop.setField(Side(marketOrder.getSide().getValue() == Side_BUY ? Side_SELL : Side_BUY));
  stop.setField(marketOrder.getSymbol());
  stop.setField(marketOrder.getQty());
  stop.setField(OrdType(OrdType_STOP));
  stop.setField(marketOrder.getStopPrice());
  olist.addGroup(stop);

  Session::sendToTarget(olist, getOrderSessionID());
}

/*!
 * Send market order with stoploss and takeprofit order
 * @param marketOrder [description]
 */
void FIXManager::marketOrderWithStopLossTakeProfit(IDEFIX::MarketOrder &marketOrder){
  FIX44::NewOrderList olist;
  olist.setField(FIX::ListID(nextOrderID()));
  olist.setField(FIX::TotNoOrders(3));
  olist.setField(FIELD::ContingencyType, "101");

  // Order
  FIX44::NewOrderList::NoOrders order;
  order.setField(ClOrdID(nextOrderID()));
  order.setField(ListSeqNo(0));
  order.setField(ClOrdLinkID("1")); // link to another clordid in list
  order.setField(Account(getAccountID()));
  order.setField(marketOrder.getSymbol());
  order.setField(marketOrder.getSide());
  order.setField(marketOrder.getQty());
  order.setField(OrdType(OrdType_MARKET));
  olist.addGroup(order);

  // StopLoss
  FIX44::NewOrderList::NoOrders stop;
  stop.setField(ClOrdID(nextOrderID()));
  stop.setField(ListSeqNo(1));
  stop.setField(ClOrdLinkID("2"));
  stop.setField(Account(getAccountID()));
  stop.setField(Side(marketOrder.getSide().getValue() == Side_BUY ? Side_SELL : Side_BUY));
  stop.setField(marketOrder.getSymbol());
  stop.setField(marketOrder.getQty());
  stop.setField(OrdType(OrdType_STOP));
  stop.setField(marketOrder.getStopPrice());
  olist.addGroup(stop);

  // TakeProfit
  FIX44::NewOrderList::NoOrders limit;
  limit.setField(ClOrdID(nextOrderID()));
  limit.setField(ListSeqNo(2));
  limit.setField(ClOrdLinkID("2"));
  limit.setField(Account(getAccountID()));
  limit.setField(Side(marketOrder.getSide().getValue() == Side_BUY ? Side_SELL : Side_BUY));
  limit.setField(marketOrder.getSymbol());
  limit.setField(marketOrder.getQty());
  limit.setField(OrdType(OrdType_LIMIT));
  limit.setField(marketOrder.getTakePrice());
  olist.addGroup(limit);

  cout << "[marketOrderWithStopLossTakeProfit] message: " << olist << endl;

  Session::sendToTarget(olist, getOrderSessionID());
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
 * Add market snapshot for symbol and recalculates prices
 * @param MarketSnapshot snapshot
 */
void FIXManager::updatePrices(const MarketSnapshot snapshot){
  // Update positions with profit and loss

}

/*!
 * Returns last market snapshot for symbol
 * @param  symbol FIX::Symbol
 * @return        IDEFIX::MarketSnapshot
 */
MarketSnapshot FIXManager::getLatestSnapshot(const FIX::Symbol symbol) const {
  MarketSnapshot snapshot;
  auto marketList = getMarketList();
  for(auto it = marketList.begin(); it != marketList.end(); ++it ){
    if( (*it).getSymbol() == symbol ){
      snapshot = (*it).getLatestSnapshot();
      break;
    }
  }

  return snapshot;
}

/*!
 * Send request to close all positions for symbol
 * @param symbol FIX::Symbol
 */
void FIXManager::closeAllPositions(const FIX::Symbol symbol){
  /*if( ! m_list_filled_orders.empty() ){
    for(auto it = m_list_filled_orders.begin(); it != m_list_filled_orders.end(); ++it){
      MarketOrder mo = *it;
      cout << "[closeAllPositions] " << mo << endl;
      queryClosePosition(mo.posID, mo.symbol, (mo.side == Side_BUY ? Side_SELL : Side_BUY), mo.qty);
    }
  }*/
}

/*!
 * Sends RequestForPositions which will return PositionReport messages if positions
 * matching the requested criteria exist; otherwise, a RequestForPositionsAck will be
 * sent with the acknowledgement that no positions exist. 
 * @param type FIX::PosReqType http://fixwiki.fixtrading.org/index.php/PosReqType
 */
void FIXManager::queryPositionReport(const FIX::PosReqType type){

  string accountID = getAccountID();

  // Set default fields
  FIX44::RequestForPositions request;
  request.setField(PosReqID("PosRequest_" + nextRequestID()));
  request.setField(PosReqType(type));

  // AccountID for the request. This must be set for routing purposes. We must
  // also set the Parties AccountID field in the NoPartySubIDs group
  request.setField(Account(accountID));
  request.setField(SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));
  request.setField(AccountType(AccountType_ACCOUNT_IS_CARRIED_ON_NON_CUSTOMER_SIDE_OF_BOOKS_AND_IS_CROSS_MARGINED));
  request.setField(TransactTime());
  request.setField(ClearingBusinessDate());
  request.setField(TradingSessionID("FXCM"));

  // Set NoPartyIDs group. These values are always as seen below
  request.setField(NoPartyIDs(1));
  FIX44::RequestForPositions::NoPartyIDs parties_group;
  parties_group.setField(PartyID("FXCM ID"));
  parties_group.setField(PartyIDSource('D'));
  parties_group.setField(PartyRole(3));
  parties_group.setField(NoPartySubIDs(1));

  // Set NoPartySubIDs group
  FIX44::RequestForPositions::NoPartyIDs::NoPartySubIDs sub_parties;
  sub_parties.setField(PartySubIDType(PartySubIDType_SECURITIES_ACCOUNT_NUMBER));

  // Set Parties AccountID
  sub_parties.setField(PartySubID(accountID));

  // Add NoPartySubIds group
  parties_group.addGroup(sub_parties);

  // Add NoPartyIDs group
  request.addGroup(parties_group);

  Session::sendToTarget(request, getOrderSessionID());
}

/*!
 * Send message to close position
 * @param position_id [description]
 */
void FIXManager::queryClosePosition(const std::string position_id, const FIX::Symbol symbol, const FIX::Side side, const FIX::OrderQty qty){
  FIX44::NewOrderSingle order;
  order.setField(ClOrdID(nextOrderID()));
  order.setField(Account(getAccountID()));
  order.setField(Symbol(symbol));
  order.setField(Side(side));
  order.setField(TransactTime());
  order.setField(OrderQty(qty));
  order.setField(OrdType(OrdType_MARKET));
  order.setField(FXCM_POS_ID, position_id);

  Session::sendToTarget(order, getOrderSessionID());
}

void FIXManager::debug(){
  
}

void FIXManager::toggleSnapshotOutput(){
  m_debug_toggle_snapshot_output = ! m_debug_toggle_snapshot_output;
}

string FIXManager::getAccountID() const {
  return m_accountID;
}

void FIXManager::setAccountID(const string accountID){
  if( m_accountID != accountID ){
    m_accountID = accountID;
  }
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
 * Returns the market list
 * @return vector<Market>
 */
vector<Market> FIXManager::getMarketList() const {
  return m_list_market;
}

/*!
 * Returns the market for given symbol
 * @param  symbol [description]
 * @return        [description]
 */
Market FIXManager::getMarket(const FIX::Symbol symbol){
  Market market;
  auto marketList = getMarketList();
  for(auto it = marketList.begin(); it != marketList.end(); ++it ){
    if( (*it).getSymbol() == symbol ){
      market = *it;
      break;
    }
  }

  return market;
}

/*!
 * Adds a snapshot to the market list
 * @param snapshot [description]
 */
void FIXManager::addMarketSnapshot(const MarketSnapshot snapshot){
  auto market = getMarket(snapshot.getSymbol());
  if( ! market.isValid() ) {
    // market does not exist, create market with snapshot
    Market nMarket(snapshot);
    auto marketList = getMarketList();
    // add market to the list
    marketList.push_back(nMarket);
  } else {
    // market exist
    market.add(snapshot);
  }
}

/*!
 * Returns the market orders list
 * @return vector<MarketOrder>
 */
vector<MarketOrder> FIXManager::getMarketOrderList() const {
  return m_list_marketorders;
}

/*!
 * Returns the market order by fxcm_pos_id
 * @param  fxcm_pos_id [description]
 * @return             [description]
 */
MarketOrder FIXManager::getMarketOrder(const string fxcm_pos_id) const {
  MarketOrder marketOrder;
  auto list = getMarketOrderList();
  for(auto it = list.begin(); it != list.end(); ++it ){
    if( (*it).getPosID() == fxcm_pos_id ){
      marketOrder = *it;
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
  auto list = getMarketOrderList();
  for(auto it = list.begin(); it != list.end(); ++it ){
    if( (*it).getClOrdID() == clOrdID ){
      marketOrder = *it;
      break;
    }
  }
  return marketOrder;
}
}; // namespace idefix