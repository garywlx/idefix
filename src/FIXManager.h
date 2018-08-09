/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */
#ifndef IDEFIX_FIXMANAGER_H
#define IDEFIX_FIXMANAGER_H

// Semantic versioning. idefix version can be printed with IDEFIX_VERSION();
#define IDEFIX_VERSION_MAJOR 0
#define IDEFIX_VERSION_MINOR 1
#define IDEFIX_VERSION_PATCH 5

#define IDEFIX_VERSION() printf("%d.%d.%d", IDEFIX_VERSION_MAJOR, IDEFIX_VERSION_MINOR, IDEFIX_VERSION_PATCH)

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <cmath>
#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/fix44/CollateralInquiryAck.h>
#include <quickfix/fix44/CollateralReport.h>
#include <quickfix/fix44/ExecutionReport.h>
#include <quickfix/fix44/MarketDataRequestReject.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/PositionReport.h>
#include <quickfix/fix44/RequestForPositionsAck.h>
#include <quickfix/fix44/SecurityList.h>
#include <quickfix/fix44/TradingSessionStatus.h>
#include <quickfix/fix44/AllocationReportAck.h>
#include <quickfix/fix44/AllocationReport.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/Session.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/Mutex.h>
#include <quickfix/Fields.h>
#include "RequestId.h"
#include "Market.h"
#include "MarketOrder.h"
#include "MarketSnapshot.h"
#include "MarketDetail.h"
#include "FXCMFields.h"
#include "FIXFactory.h"
#include "Account.h"
#include "Math.h"
#include "Pairs.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std;
using namespace FIX;

namespace IDEFIX {
class Strategy;
class Indicator;

class FIXManager: public MessageCracker, public Application {
private:
  // Synchronizing multithreading
  mutable FIX::Mutex m_mutex;

  // Pointer to SessionSettings from SessionSettingsFile
  SessionSettings *m_psettings;
  // Pointer to File Store Factory
  FileStoreFactory *m_pstore_factory;
  // Pointer to File Log Factory
  FileLogFactory *m_plog_factory;
  // Pointer to Socket
  SocketInitiator *m_pinitiator;
  // The strategy to use
  Strategy *m_pstrategy;
  // RequestID Manager
  RequestId m_reqid_manager;

  // console logger
  std::shared_ptr<spdlog::logger> m_console;
  // tradelog logger
  std::shared_ptr<spdlog::logger> m_tradelog;

  // the session id for market data, such as tick prices
  SessionID m_market_sessionID;
  // the session id for order management, such as open/closing positions
  SessionID m_order_sessionID;
  // The account
  IDEFIX::Account m_account;

  // hold all market snapshots per symbol list[symbol] = Market
  map<string, Market> m_list_market;
  // hold all open market positions list[posid] = marketOrder
  map<string, MarketOrder> m_list_marketorders;
  // hold system parameters list[key] = value
  map<string, string> m_system_params;
  // hold all market details list[symbol] = MarketDetail
  map<string, MarketDetail> m_market_details;
  // hold all subscriptions symbols
  vector<string> m_symbol_subscriptions;
  // hold all indicators per symbol list[symbol][] = Indicator*
  map<string, vector<Indicator*> > m_list_indicators;

  // if the app is exiting, don't log tick data etc anymore
  bool m_is_exiting;
  
public:
  FIXManager();
  FIXManager(const string settingsFile, Strategy* strategy);
  ~FIXManager();

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
  void onMessage(const FIX44::AllocationReportAck& ack, const SessionID& session_ID);
  void onMessage(const FIX44::AllocationReport& ar, const SessionID& session_ID);

  // All Message Query Methods
  void queryTradingStatus();
  void queryAccounts();
  void queryPositionReport(const FIX::PosReqType type = PosReqType_POSITIONS);
  void queryOrderMassStatus();

  void subscribeMarketData(const std::string symbol);
  void unsubscribeMarketData(const std::string symbol);
  
  void marketOrder(const MarketOrder& marketOrder, const FIXFactory::SingleOrderType orderType = FIXFactory::SingleOrderType::MARKET_ORDER);

  void closeAllPositions(const string symbol);
  void closePosition(const MarketOrder& marketOrder);
  void closeWinners(const string symbol);
  void closeLoosers(const string symbol);

  // Public Getter & Setter
  MarketSnapshot getLatestSnapshot(const string symbol);
  MarketDetail getMarketDetails(const std::string& symbol);
  Account getAccount();
  string getAccountID() const;
  Market getMarket(const string symbol);
  
  void showSysParamList();
  void showAvailableMarketList();
  void showMarketDetail(const string symbol);

  void onExit();

  std::shared_ptr<spdlog::logger> console();
  void tradelog(const MarketOrder& marketOrder);

  bool isExiting();
  void setExiting(const bool status);

  void addIndicator(const std::string symbol, Indicator* indicator);
  void remIndicator(const std::string symbol, const std::string name);
  Indicator* getIndicator(const std::string symbol, const std::string name);

private:
  void onInit();
  void startSession(const string settingsfile);
  void endSession();

  void onMarketSnapshot(const MarketSnapshot& snapshot);
  
  void processMarketOrders(const MarketSnapshot& snapshot);
  void processStrategy(const MarketSnapshot& snapshot);
  void processIndicators(const MarketSnapshot& snapshot);

  string nextRequestID();
  string nextOrderID();

  const FIX::Dictionary* getSessionSettingsPtr(const SessionID& session_ID);
  bool isMarketDataSession(const SessionID& session_ID);
  bool isOrderSession(const SessionID& session_ID);
  
  void setAccount(const Account account);

  SessionID getMarketSessionID() const;
  void setMarketSessionID(const SessionID& session_ID);

  SessionID getOrderSessionID() const;
  void setOrderSessionID(const SessionID& session_ID);

  void addMarket(const Market market);
  void addMarketSnapshot(const MarketSnapshot snapshot);
  void addMarketOrder(const MarketOrder marketOrder);
  void removeMarketOrder(const string posID);
  void updateMarketOrder(const MarketOrder& marketOrder, const bool isUnsolicited = false);

  MarketOrder getMarketOrder(const string fxcm_pos_id) const;
  MarketOrder getMarketOrder(const ClOrdID clOrdID) const;

  void addMarketDetail(const MarketDetail& marketDetail);

  void addSysParam(const string key, const string value);
  string getSysParam(const string key);

  void addSubscription(const string symbol);
  void removeSubscription(const string symbol);

  bool setStrategy(Strategy* strategy);
}; // class fixmanager
}; // namespace idefix

#endif //IDEFIX_FIXMANAGER_H
