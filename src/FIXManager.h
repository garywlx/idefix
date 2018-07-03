/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */
#ifndef IDEFIX_FIXMANAGER_H
#define IDEFIX_FIXMANAGER_H

#include <iostream>
#include <vector>
#include <map>
#include <utility>
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
#include <quickfix/Mutex.h>
#include "RequestId.h"
#include "Market.h"

using namespace std;
using namespace FIX;

namespace IDEFIX {
class FIXManager: public MessageCracker, public Application {
private:
  // for debugging
  bool m_debug_toggle_snapshot_output;

  mutable FIX::Mutex m_mutex;

  // Pointer to SessionSettings from SessionSettingsFile
  SessionSettings *m_psettings;
  // Pointer to File Store Factory
  FileStoreFactory *m_pstore_factory;
  // Pointer to File Log Factory
  FileLogFactory *m_plog_factory;
  // Pointer to Socket
  SocketInitiator *m_pinitiator;
  // RequestID Manager
  RequestId m_reqid_manager;

  // the session id for market data, such as tick prices
  SessionID m_market_sessionID;
  // the session id for order management, such as open/closing positions
  SessionID m_order_sessionID;
  // the account id 
  string m_accountID;

  // hold all market snapshots per symbol
  map<string, Market> m_list_market;
  // hold all open market positions
  vector<MarketOrder> m_list_marketorders;
  // hold system parameters
  map<string, string> m_system_params;


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

public:
  FIXManager();
  FIXManager(const string settingsFile);

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

  void onMessage(const FIX44::TradingSessionStatus& tss, const SessionID& session_ID);
  void onMessage(const FIX44::CollateralInquiryAck& ack, const SessionID& session_ID);
  void onMessage(const FIX44::CollateralReport& cr, const SessionID& session_ID);
  void onMessage(const FIX44::RequestForPositionsAck& ack, const SessionID& session_ID);
  void onMessage(const FIX44::PositionReport& pr, const SessionID& session_ID);
  void onMessage(const FIX44::MarketDataRequestReject& mdr, const SessionID& session_ID);
  void onMessage(const FIX44::MarketDataSnapshotFullRefresh& mds, const SessionID& session_ID);
  void onMessage(const FIX44::ExecutionReport& er, const SessionID& session_ID);
    
  void startSession(const string settingsfile);
  void endSession();

  // All Message Query Methods
  void queryTradingStatus();
  void queryAccounts();
  void queryPositionReport(const FIX::PosReqType type = PosReqType_POSITIONS);
  void queryClosePosition(const std::string position_id, const FIX::Symbol symbol, const FIX::Side side, const FIX::OrderQty qty);
  void queryMarketData(const FIX::Symbol symbol, const FIX::SubscriptionRequestType requestType);

  void subscribeMarketData(const FIX::Symbol symbol);
  void unsubscribeMarketData(const FIX::Symbol symbol);
  void marketOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty);
  void stopOrder(const FIX::Symbol symbol, const FIX::Side side, const double qty, const double price);
  void marketOrderWithStoploss(MarketOrder& marketOrder);
  void marketOrderWithStopLossTakeProfit(MarketOrder& marketOrder);
  void closeAllPositions(const FIX::Symbol symbol);

  // Public Getter & Setter
  MarketSnapshot getLatestSnapshot(const string symbol);

  void debug();
  void toggleSnapshotOutput();

private:
  void updatePrices(const MarketSnapshot snapshot);
  string nextRequestID();
  string nextOrderID();

  const FIX::Dictionary* getSessionSettingsPtr(const SessionID& session_ID);
  bool isMarketDataSession(const SessionID& session_ID);
  bool isOrderSession(const SessionID& session_ID);
  
  // Private Getter & Setter
  string getAccountID() const;
  void setAccountID(const string accountID);

  SessionID getMarketSessionID() const;
  void setMarketSessionID(const SessionID& session_ID);

  SessionID getOrderSessionID() const;
  void setOrderSessionID(const SessionID& session_ID);

  Market getMarket(const string symbol);
  void addMarket(const Market market);
  void addMarketSnapshot(const MarketSnapshot snapshot);

  vector<MarketOrder> getMarketOrderList() const;
  MarketOrder getMarketOrder(const string fxcm_pos_id) const;
  MarketOrder getMarketOrder(const ClOrdID clOrdID) const;

  void addSysParam(const string key, const string value);
  string getSysParam(const string key);
}; // class fixmanager
}; // namespace idefix

#endif //IDEFIX_FIXMANAGER_H
