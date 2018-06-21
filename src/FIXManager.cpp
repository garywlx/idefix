/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */
#include "FIXManager.h"
#include <cmath>

FIXManager::FIXManager() {
  // Initialize unsigned int requestID to 1. We will use this as a counter
  // for making request ids
  _requestID = 1000;
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
    _market_sessionID = sessionID;
    cout << "[onLogon] " << _market_sessionID << " (MarketDataSession)" << endl;

    getTradingStatus();
  }

  if( isOrderSession(sessionID) ){
    _order_sessionID = sessionID;
    cout << "[onLogon] " << _order_sessionID << " (OrderSession)" << endl;
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
    string str_username = _settings->get().getString("Username");
    string str_password = _settings->get().getString("Password");
    message.setField(Username(str_username));
    message.setField(Password(str_password));
  }
  // All messages sent to FXCM must contain the TargetSubID field (both Administrative and
  // Application messages). Here we set this.
  string sub_ID = _settings->get().getString("TargetSubID");
  message.getHeader().setField(TargetSubID(sub_ID));
}

// A callback for application messages that you are being sent to a counterparty
void FIXManager::toApp(Message &message, const SessionID &session_ID)
  throw( FIX::DoNotSend ) {
  // All messages sent to FXCM must contain the TargetSubID field (both Administrative and
  // Application messages).
  string sub_ID = _settings->get().getString("TargetSubID");
  message.getHeader().setField(TargetSubID(sub_ID));
}

// Notifies you when an administrative message is sent from FXCM to your FIX engine.
void FIXManager::fromAdmin(const Message &message, const SessionID &session_ID)
  throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon ) {
  // Call MessageCracker.crack method to handle the message by one of our
  // overloaded onMessage methods below
  //cout << "[fromAdmin] " << message << endl;
  crack(message, session_ID);
}

// One of the core entry points for your FIX application. Every application level request will come through here
void FIXManager::fromApp(const Message &message, const SessionID &session_ID)
  throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType ) {
  // Call MessageCracker.crack method to handle the message by one of our
  // overloaded onMessage methods below
  // cout << "[fromApp] " << message << endl;
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
  getAccounts();

  // ** Note on Text(58)
  // You will notice that Text(58) field is always set to "Market is closed. Any trading functionality
  // is not available." This field is always set to this value; therefore, do not use this field
  // to determine if the trading desk is open. As stated above, use TradSesStatus for this purpose
 }

void FIXManager::onMessage(const FIX44::CollateralInquiryAck &ack, const SessionID &session_ID) {

}

// CollateralReport is a message containing important information for each account under the login.
// It is returned as a response to CollateralInquiry. You will receive a CollateralReport for each account
// under your login. Notable fields include Account(1) which is the AccountID and CashOutstanding(901) which
// is the account balance
void FIXManager::onMessage(const FIX44::CollateralReport &cr, const SessionID &session_ID) {
  cout << "[onMessage:CollateralReport] " << endl;

  string accountID = cr.getField(FIELD::Account);

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
    cout << "    " << sub_type << " -> " << sub_value << endl;
  }

  // Add the accountID to our vector<string> being used to track all accounts under the login
  recordAccount(accountID);
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
  string positionID = pr.getField(FXCM_POS_ID);
  string pos_open_time = pr.getField(FXCM_POS_OPEN_TIME);

  cout << "[onMessage:PositionReport]" << endl;
  cout << "  Account -> " << accountID << endl;
  cout << "  Symbol -> " << symbol << endl;
  cout << "  PositionID -> " << positionID << endl;
  cout << "  Open Time -> " << pos_open_time << endl;
  cout << "  Currency -> " << pr.getField(FIELD::Currency) << endl;
  cout << "  SettlPrice -> " << pr.getField(FIELD::SettlPrice) << endl;

}

/*!
 * Is called if a MarketDataRequest rejected
 * @param mdr        [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::MarketDataRequestReject &mdr, const SessionID &session_ID) {
  // If MarketDataRequestReject is returned as the result of a MarketDataRequest message
  // print out the contents of the Text field but first check that it is set
  cout << "MarketDataRequestReject -> " << endl;
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

  // Declare variables for both the bid and ask prices. We will read the MarketDataSnapshotFullRefresh
  // message for these values
  double bid_price = 0;
  double ask_price = 0;

  // For each MDEntry in the message, inspect the NoMDEntries group for the presence of either the Bid or Ask
  // (Offer) type
  int entry_count = IntConvertor::convert(mds.getField(FIELD::NoMDEntries));
  for(int i = 1; i < entry_count; i++){
    FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
    mds.getGroup(i, group);
    string entry_type = group.getField(FIELD::MDEntryType);
    if(entry_type == "0"){
      // BID Price
      bid_price = DoubleConvertor::convert(group.getField(FIELD::MDEntryPx));
    } else if(entry_type == "1"){
      // ASK Price
      ask_price = DoubleConvertor::convert(group.getField(FIELD::MDEntryPx));
    } 
  } 

  // http://www.youngbrokers.net/2013/bid-ask-spread/
  double spread = std::abs(ask_price - bid_price) * 10000;

  // save/update last snapshot
  addMarketSnapshot(Symbol(symbol), bid_price, ask_price, spread, entry_date);

#ifdef SHOW_MARKETSNAPSHOT
  cout << symbol << " " << entry_date << " " << " Bid - " << bid_price << " Ask " << ask_price << " Spread " << std::fixed << spread << endl;
#endif
}

/*!
 * Is called if an ExecutionReport for a new order is available
 * @param er         [description]
 * @param session_ID [description]
 */
void FIXManager::onMessage(const FIX44::ExecutionReport &er, const SessionID &session_ID) {

  string status = er.getField(FIELD::OrdStatus);
  string statustext;
  if(status == "2"){
    statustext = "Filled";
  } else if(status == "8"){
    statustext = "Cancelled";
  } else if(status == "4"){
    statustext = "Rejected";
  } else {
    statustext = status;
  }

  cout << "[onMessage:ExecutionReport]" << endl;
  cout << "  OrdStatus -> " << status << " " << statustext << endl;
  cout << "  Text -> " << er.getField(FIELD::Text) << endl;
  cout << "  ClOrdID -> " << er.getField(FIELD::ClOrdID) << endl;
  cout << "  Account -> " << er.getField(FIELD::Account) << endl;
  cout << "  OrderID -> " << er.getField(FIELD::OrderID) << endl;
  cout << "  LastQty -> " << er.getField(FIELD::LastQty) << endl;
  cout << "  ExecType -> " << er.getField(FIELD::ExecType) << endl;
  cout << "  CumQty  -> " << er.getField(FIELD::CumQty) << endl;

  // ** Note on order status. **
  // In order to determine the status of an order, and also how much an order is filled, we must
  // use the OrdStatus and CumQty fields. There are 3 possible final values for OrdStatus: Filled(2),
  // Rejected(8), and Cancelled(4). When the OrdStatus field is set to one of these values, you know
  // the execution is completed. At this time the CumQty (14) can be inspected to determine if and how
  // much of an order was filled.
}

void FIXManager::onMessage(const FIX44::OrderCancelReject& ocr, const SessionID& session_ID){

}

void FIXManager::onMessage(const FIX44::TradeCaptureReport& tcr, const SessionID& session_ID){
  cout << "[onMessage:TradeCaptureReport]" << endl;
  cout << tcr << endl;
}

void FIXManager::onMessage(const FIX44::TradeCaptureReportAck& ack, const SessionID& session_ID){
  cout << "[onMessage:TradeCaptureReportAck]" << endl;

}

void FIXManager::onMessage(const FIX44::TradeCaptureReportRequest& tcrr, const SessionID& session_ID){
  cout << "[onMessage:TradeCaptureReportRequest]" << endl;
  cout << tcrr << endl;
}

void FIXManager::onMessage(const FIX44::TradeCaptureReportRequestAck& ack, const SessionID& session_ID){
  cout << "[onMessage:TradeCaptureReportRequestAck]" << endl;
}

// Starts the FIX session. Throws FIX::ConfigError exception if our configuration settings
// do not pass validation required to construct SessionSettings
void FIXManager::startSession(const string settingsfile) {
  try {
    _settings = new SessionSettings(settingsfile);
    _store_factory = new FileStoreFactory(*_settings);
    _log_factory = new FileLogFactory(*_settings);
    _initiator = new SocketInitiator(*this, *_store_factory, *_settings, *_log_factory/* Optional*/);
    _initiator->start();
  } catch(ConfigError& error){
    cout << error.what() << endl;
  }
}

// Logout and end session
void FIXManager::endSession() {
  _initiator->stop();
  delete _initiator;
  delete _settings;
  delete _store_factory;
  delete _log_factory;
}

// Sends TradingSessionStatusRequest message in order to receive as a response the
// TradingSessionStatus message
void FIXManager::getTradingStatus() {
  // Request TradingSessionStatus message
  FIX44::TradingSessionStatusRequest request;
  request.setField(TradSesReqID(nextRequestID()));
  request.setField(TradingSessionID("FXCM"));
  request.setField(SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT));
  Session::sendToTarget(request, _order_sessionID);
}

// Sends the CollateralInquiry message in order to receive as a response the CollateralReport message
void FIXManager::getAccounts(){
  // Request CollateralReport message. We will receive a CollateralReport for each
  // account under our login
  FIX44::CollateralInquiry request;
  request.setField(CollInquiryID(nextRequestID()));
  request.setField(TradingSessionID("FXCM"));
  request.setField(SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT));
  Session::sendToTarget(request, _order_sessionID);
}

// Sends RequestForPositions which will return PositionReport messages if positions
// matching the requested criteria exist; otherwise, a RequestForPositionsAck will be
// sent with the acknoledgement that no positions exist. In our example, we request
// positions for all accounts under our login.
void FIXManager::getPositions() {
  // Here we will get positions for each account under our login. To do this,
  // we will send a RequestForPositions message that contains the accountID
  // associated with our request. For each account in our list, we send RequestForPositions.
  int total_accounts = (int) _list_accountID.size();
  for(int i = 0; i < total_accounts; i++){
    string accountID = _list_accountID.at(i);

    // Set default fields
    FIX44::RequestForPositions request;
    request.setField(PosReqID(nextRequestID()));
    request.setField(PosReqType(PosReqType_POSITIONS));

    // AccountID for the request. This must be set for routing purposes. We must
    // also set the Parties AccountID field in the NoPartySubIDs group
    request.setField(Account(accountID));
    request.setField(SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT));
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

    // Send request
    Session::sendToTarget(request, _order_sessionID);
  }
}

// Subscribes to the EUR/USD trading security
void FIXManager::subscribeMarketData(const FIX::Symbol symbol) {
  // Subscribe to market data for EUR/USD
  string request_ID = "Request_";
  request_ID.append(symbol);

  FIX44::MarketDataRequest request;
  request.setField(MDReqID(request_ID));
  request.setField(SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));
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

  Session::sendToTarget(request, _market_sessionID);
}

// Unsubscribe from the EUR/USD trading security
void FIXManager::unsubscribeMarketData(const FIX::Symbol symbol) {
  // Unsubscribe from EUR/USD. Note that our request_ID is the exact same that was sent for our
  // request to subscribe. This is necessary to unsubscribe. This request below is identical to our request
  // to subscribe with the exception that SubscriptionRequestType is set to
  // "SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT_PLUS_UPDATE_REQUEST"
  string request_ID = "Request_";
  request_ID.append(symbol);

  FIX44::MarketDataRequest request;
  request.setField(MDReqID(request_ID));
  request.setField(SubscriptionRequestType(
      SubscriptionRequestType_DISABLE_PREVIOUS_SNAPSHOT_PLUS_UPDATE_REQUEST));
  request.setField(MarketDepth(0));
  request.setField(NoRelatedSym(1));

  // Add the NoRelatedSym group to the request with Symbol
  // field set to EUR/USD
  FIX44::MarketDataRequest::NoRelatedSym symbols_group;
  symbols_group.setField(symbol);

  request.addGroup(symbols_group);

  // Add the NoMDEntryTypes group to the request for each MDEntryType
  // that we are subscribing to. This includes Bid, Offer, High, and Low
  FIX44::MarketDataRequest::NoMDEntryTypes entry_types;
  entry_types.setField(MDEntryType(MDEntryType_BID));
  request.addGroup(entry_types);
  entry_types.setField(MDEntryType(MDEntryType_OFFER));
  request.addGroup(entry_types);
  entry_types.setField(MDEntryType(MDEntryType_TRADING_SESSION_HIGH_PRICE));
  request.addGroup(entry_types);
  entry_types.setField(MDEntryType(MDEntryType_TRADING_SESSION_LOW_PRICE));
  request.addGroup(entry_types);

  Session::sendToTarget(request, _market_sessionID);
}

/*!
 * Sends a basic market order message to buy/sell at best market price
 * @param symbol [description]
 * @param side   [description]
 * @param qty    [description]
 */
void FIXManager::marketOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty) {
  FIX44::NewOrderSingle request;
  request.setField(ClOrdID(nextRequestID()));
  request.setField(Account(_list_accountID.at(0)));
  request.setField(Symbol(symbol));
  request.setField(TradingSessionID("FXCM"));
  request.setField(TransactTime());
  request.setField(OrdType(FIX::OrdType_MARKET));
  request.setField(OrderQty(qty));
  request.setField(Side(side));
  request.setField(OrdType(OrdType_MARKET));
  request.setField(TimeInForce(TimeInForce_FILL_OR_KILL));
  
  Session::sendToTarget(request, _order_sessionID);
}

/*!
 * Send limit order at given price with fill or kill validity
 * @param symbol FIX::Symbol
 * @param side   FIX::Side http://fixwiki.org/fixwiki/Side
 * @param qty    double 
 * @param price  double
 */
void FIXManager::limitOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty, const double price){
  FIX44::NewOrderSingle request;
  request.setField(ClOrdID(nextRequestID()));
  request.setField(Account(_list_accountID.at(0)));
  request.setField(Symbol(symbol));
  request.setField(TradingSessionID("FXCM"));
  request.setField(OrdType(FIX::OrdType_LIMIT));
  request.setField(TransactTime());
  request.setField(OrderQty(qty));
  request.setField(Side(side));
  request.setField(TimeInForce(TimeInForce_FILL_OR_KILL));

  Session::sendToTarget(request, _order_sessionID);
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
  request.setField(ClOrdID(nextRequestID()));
  request.setField(Account(_list_accountID.at(0)));
  request.setField(Symbol(symbol));
  request.setField(TradingSessionID("FXCM"));
  request.setField(TransactTime());
  request.setField(OrdType(FIX::OrdType_STOP));
  request.setField(OrderQty(qty));
  request.setField(Side(side));
  request.setField(StopPx(price));
  request.setField(PositionEffect(PositionEffect_CLOSE));

  Session::sendToTarget(request, _order_sessionID);
}

/*!
 * Get information about all trades
 */
void FIXManager::sendTradeCaptureRequest() {
  FIX44::TradeCaptureReportRequest request;
  request.setField(TradeRequestID("TradeRequest_" + nextRequestID()));
  request.setField(TradeRequestType(0)); // all trades
  request.setField(Text());
  request.setField(SubscriptionRequestType(SubscriptionRequestType_SNAPSHOT));
  request.setField(ExecID());
  request.setField(ExecType());
  request.setField(OrderID());
  request.setField(ClOrdID());

  Session::sendToTarget(request, _market_sessionID);
}

// Generate string value used to populate the fields in each message
// which are used as a custom identifier
string FIXManager::nextRequestID() {
  if(_requestID == 65535){
    _requestID = 1;
  }

  _requestID++;
  string next_ID = IntConvertor::convert(_requestID);
  return next_ID;
}

// Adds string accountIDs to our vector<string> being used to
// account for the accountIDs under our login
void FIXManager::recordAccount(string accountID) {
  int size = (int) _list_accountID.size();
  if( size == 0 ){
    _list_accountID.push_back(accountID);
  } else {
    for(int i = 0; i < size; i++){
      if(_list_accountID.at(i) == accountID){
        break;
      }

      if(i == size - 1){
        _list_accountID.push_back(accountID);
      }
    }
  }
}

// Get the settings dictionary for the session
const FIX::Dictionary* FIXManager::getSessionSettingsPtr(const SessionID& session_ID){
  const FIX::Dictionary* pSettings = _initiator->getSessionSettings(session_ID);
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
 * Add market snapshot for symbol
 * @param symbol       FIX::Symbol
 * @param bid          FIX::Price
 * @param ask          FIX::Price
 * @param spread       FIX::Price
 * @param sending_time std::string
 */
void FIXManager::addMarketSnapshot(const FIX::Symbol symbol, const FIX::Price bid, const FIX::Price ask, const FIX::Price spread, const std::string sending_time){
  // check if symbol is already in list
  int size = (int) _list_market_snapshot.size();
  if( size == 0 ){
    // add entry
    IDEFIX::MarketSnapshot ms {symbol, bid, ask, spread, sending_time};
    _list_market_snapshot.push_back(ms);
  } else {
    for(int i = 0; i < size; i++){
      auto snapshot = _list_market_snapshot.at(i);
      if(snapshot.symbol == symbol){
        snapshot.bid = bid;
        snapshot.ask = ask;
        snapshot.sending_time = sending_time;
        break;
      }

      // add entry
      if( i == size - 1 ){
        IDEFIX::MarketSnapshot ms {symbol, bid, ask, spread, sending_time};
        _list_market_snapshot.push_back(ms);
      }
    }
  }
}

/*!
 * Returns last market snapshot for symbol
 * @param  symbol FIX::Symbol
 * @return        IDEFIX::MarketSnapshot
 */
IDEFIX::MarketSnapshot FIXManager::marketSnapshot(const FIX::Symbol symbol) const {
  int size = (int) _list_market_snapshot.size();
  IDEFIX::MarketSnapshot ms {symbol, 0, 0, 0, ""};
  
  for(int i = 0; i < size; i++){
    auto _ms = _list_market_snapshot.at(i);
    if( _ms.symbol == symbol ){
      ms.bid = _ms.bid;
      ms.ask = _ms.ask;
      ms.spread = _ms.spread;
      ms.sending_time = _ms.sending_time;
    }
  }

  return ms;
}

/*!
 * Send request to close all positions for symbol
 * @param symbol FIX::Symbol
 */
void FIXManager::closeAllPositions(const FIX::Symbol symbol){

}
