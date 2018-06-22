/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */
#ifndef IDEFIX_FIXMANAGER_H
#define IDEFIX_FIXMANAGER_H

#include <iostream>
#include <vector>
#include "types.h"
#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/fix44/CollateralInquiry.h>
#include <quickfix/fix44/CollateralInquiryAck.h>
#include <quickfix/fix44/CollateralReport.h>
#include <quickfix/fix44/ExecutionReport.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataRequestReject.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/NewOrderList.h>
#include <quickfix/fix44/NewOrderSingle.h>
#include <quickfix/fix44/PositionReport.h>
#include <quickfix/fix44/RequestForPositions.h>
#include <quickfix/fix44/RequestForPositionsAck.h>
#include <quickfix/fix44/SecurityList.h>
#include <quickfix/fix44/TradingSessionStatus.h>
#include <quickfix/fix44/TradingSessionStatusRequest.h>
#include <quickfix/fix44/OrderCancelRequest.h>
#include <quickfix/fix44/OrderCancelReject.h>
#include <quickfix/fix44/TradeCaptureReportRequest.h>
#include <quickfix/fix44/TradeCaptureReport.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/Session.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>

using namespace std;
using namespace FIX;
using namespace IDEFIX;

class FIXManager: public MessageCracker, public Application {
private:
  SessionSettings *_settings;
  FileStoreFactory *_store_factory;
  FileLogFactory *_log_factory;
  SocketInitiator *_initiator;

  // used as a counter for producing unique request identifiers
  unsigned int _requestID;
  // the session id for market data, such as tick prices
  SessionID _market_sessionID;
  // the session id for order management, such as open/closing positions
  SessionID _order_sessionID;
  // hold the account ids in a list
  vector<string> _list_accountID;
  // hold the last bid and ask price of symbol
  vector<IDEFIX::MarketSnapshot> _list_market_snapshot;

  // Custom FXCM FIX fields
  enum FXCM_FIX_FIELDS
  {
    FXCM_FIELD_PRODUCT_ID      = 9080,
    FXCM_POS_ID                = 9041,
    FXCM_POS_OPEN_TIME         = 9042,
    FXCM_CLOSE_SETTLE_PRICE    = 9043,
    FXCM_ERROR_DETAILS         = 9029,
    FXCM_REQUEST_REJECT_REASON = 9025,
    FXCM_USED_MARGIN           = 9038,
    FXCM_POS_CLOSE_TIME        = 9044,
    FXCM_MARGIN_CALL           = 9045,
    FXCM_ORD_TYPE              = 9050,
    FXCM_ORD_STATUS            = 9051,
    FXCM_CLOSE_PNL             = 9052,
    FXCM_SYM_POINT_SIZE        = 9002,
    FXCM_SYM_PRECISION         = 9001,
    FXCM_TRADING_STATUS        = 9096,
    FXCM_PEG_FLUCTUATE_PTS     = 9061,
    FXCM_NO_PARAMS             = 9016,
    FXCM_PARAM_NAME            = 9017,
    FXCM_PARAM_VALUE           = 9018
  };

  // add market snapshot with last price information
  void addMarketSnapshot(const FIX::Symbol symbol, const FIX::Price bid, const FIX::Price ask, const FIX::Price spread, const std::string sending_time);

public:
  FIXManager();
  // FIX Namespace. These are callbacks which indicate when the session is created,
  // when we logon and logout, and when messages are exchanged
  void onCreate(const SessionID& sessionID);
  void onLogon(const SessionID& sessionID);
  void onLogout(const SessionID& session_ID);
  void toAdmin(Message& message, const SessionID& session_ID);
  void toApp(Message &message, const SessionID &session_ID)
  throw( FIX::DoNotSend );
  void fromAdmin(const Message& message, const SessionID& session_ID)
  throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon );
  void fromApp(const Message& message, const SessionID& session_ID)
  throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType );

  // Overloaded onMessage methods used in conjuction with MessageCracker class. FIX::MessageCracker
  // receives a generic Message in the FIX fromApp and fromAdmin callbacks, constructs the
  // message sub type and invokes the appropriate onMessage method below.
  void onMessage(const FIX44::TradingSessionStatus& tss, const SessionID& session_ID);
  void onMessage(const FIX44::CollateralInquiryAck& ack, const SessionID& session_ID);
  void onMessage(const FIX44::CollateralReport& cr, const SessionID& session_ID);
  void onMessage(const FIX44::RequestForPositionsAck& ack, const SessionID& session_ID);
  void onMessage(const FIX44::PositionReport& pr, const SessionID& session_ID);
  void onMessage(const FIX44::MarketDataRequestReject& mdr, const SessionID& session_ID);
  void onMessage(const FIX44::MarketDataSnapshotFullRefresh& mds, const SessionID& session_ID);
  void onMessage(const FIX44::ExecutionReport& er, const SessionID& session_ID);
  void onMessage(const FIX44::OrderCancelReject& ocr, const SessionID& session_ID);
  void onMessage(const FIX44::TradeCaptureReport& tcr, const SessionID& session_ID);
  void onMessage(const FIX44::TradeCaptureReportAck& ack, const SessionID& session_ID);
  void onMessage(const FIX44::TradeCaptureReportRequest& tcrr, const SessionID& session_ID);
  void onMessage(const FIX44::TradeCaptureReportRequestAck& ack, const SessionID& session_ID);
  

  // Starts the FIX session. Throws FIX::ConfigError exception if our configuration settings
  // do not pass validation required to construct SessionSettings
  void startSession(const string settingsfile);
  // Logout and end session
  void endSession();

  // Sends TradingSessionStatusRequest message in order to receive as a response the
  // TradingSessionStatus message
  void getTradingStatus();
  // Sends the CollateralInquiry message in order to receive as a response the
  // CollateralReport message.
  void queryAccounts();
  
  // Subscribes to a trading security
  void subscribeMarketData(const FIX::Symbol symbol);
  // Unsubscribes from a trading security
  void unsubscribeMarketData(const FIX::Symbol symbol);
  // sends a market order with fill or kill validity
  void marketOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty);
  // Send limit order at given price with fill or kill validity
  void limitOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty, const double price);
  // sends a stop order at given price with fill or kill validity
  void stopOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty, const double price);
  
  void sendTradeCaptureRequest();
  // Generate string value used to populate the fields in each message
  // which are used as a custom identifier
  string nextRequestID();
  // Adds string accountIDs to our vector<string> being used to
  // account for the accountIDs under our login
  void recordAccount(string accountID);

  const FIX::Dictionary* getSessionSettingsPtr(const SessionID& session_ID);
  bool isMarketDataSession(const SessionID& session_ID);
  bool isOrderSession(const SessionID& session_ID);

  void closeAllPositions(const FIX::Symbol symbol);

  // Returns last market snapshot for symbol
  IDEFIX::MarketSnapshot marketSnapshot(const FIX::Symbol symbol) const;

  // Sends RequestForPositions which will return PositionReport messages if positions
  // matching the requested criteria exist; otherwise, a RequestForPositionsAck will be
  // sent with the acknowledgement that no positions exist. In our example, we request
  // positions for all accounts under our login
  void queryPositionReport(const FIX::PosReqType type = PosReqType_POSITIONS);
  // query message to close a position with position_id
  void queryClosePosition(const std::string position_id, const FIX::Symbol symbol, const FIX::Side side, const FIX::OrderQty qty);
};


#endif //IDEFIX_FIXMANAGER_H
