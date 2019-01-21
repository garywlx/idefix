/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */
#ifndef IDEFIX_FIXMANAGER_H
#define IDEFIX_FIXMANAGER_H

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
#include "Pairs.h"
#include "SignalType.h"

#include <nod/nod.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace std;
using namespace FIX;

namespace IDEFIX {

class FIXManager: public MessageCracker, public Application {
public:
  // Synchronizing multithreading
  mutable FIX::Mutex m_mutex;
private:
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

  // console logger
  std::shared_ptr<spdlog::logger> m_console;
  // tradelog logger
  std::shared_ptr<spdlog::logger> m_tradelog;

  // the session id for market data, such as tick prices
  SessionID m_market_sessionID;
  // the session id for order management, such as open/closing positions
  SessionID m_order_sessionID;
  // The account
  std::shared_ptr<IDEFIX::Account> m_account;

  // hold all market snapshots per symbol list[symbol] = Market
  map<std::string, Market> m_list_market;
  // hold all open market positions list[posid] = marketOrder
  map<std::string, MarketOrder> m_list_marketorders;
  // hold system parameters list[key] = value
  map<std::string, std::string> m_system_params;
  // hold all market details list[symbol] = MarketDetail
  map<std::string, MarketDetail> m_market_details;
  // hold all subscriptions symbols
  vector<std::string> m_symbol_subscriptions;

  // if the app is exiting, don't log tick data etc anymore
  bool m_is_exiting;
  
public:
  // signals
  // on_tick
  nod::signal<void(const MarketSnapshot&)> on_tick;
  // on_init
  nod::signal<void()> on_init;
  // on_exit
  nod::signal<void()> on_exit;
  // on_update_marketorder
  nod::signal<void(const MarketOrder&, const MarketOrder::Status)> on_update_marketorder;
  // on_error
  nod::signal<void(const std::string& sender, const std::string& message)> on_error;
  // on_before_session_start
  nod::signal<void()> on_before_session_start;
  // on_before_session_end
  nod::signal<void()> on_before_session_end;
  // on_account_change
  nod::signal<void(std::shared_ptr<IDEFIX::Account>)> on_account_change;
  // on_market_order
  nod::signal<void(const SignalType type, const MarketOrder&)> on_market_order;
  

  FIXManager();
  // FIXManager(const std::string settingsFile);
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

  void closeAllPositions(const std::string symbol);
  void closeAllPositions(const std::string symbol, const char side);
  void closePosition(const MarketOrder& marketOrder);
  void closeWinners(const std::string symbol);
  void closeLoosers(const std::string symbol);

  // Public Getter & Setter
  std::shared_ptr<MarketSnapshot> getLatestSnapshot(const std::string symbol);
  std::shared_ptr<MarketDetail> getMarketDetails(const std::string& symbol);
  std::shared_ptr<Account> getAccount();
  std::string getAccountID() const;
  std::shared_ptr<Market> getMarket(const std::string& symbol);
  
  void showSysParamList();
  void showAvailableMarketList();
  void showMarketDetail(const std::string symbol);

  std::shared_ptr<spdlog::logger> console();
  void tradelog(const MarketOrder& marketOrder);

  bool isExiting();
  void setExiting(const bool status);

  bool hasOpenPositions(const std::string symbol);

  void connect(const std::string settingsFile);
  void disconnect();

private:
  void onInit();
  void onExit();

  void onMarketSnapshot(const MarketSnapshot& snapshot);
  
  void processMarketOrders(const MarketSnapshot& snapshot);

  std::string nextRequestID();
  std::string nextOrderID();

  const FIX::Dictionary* getSessionSettingsPtr(const SessionID& session_ID);
  bool isMarketDataSession(const SessionID& session_ID);
  bool isOrderSession(const SessionID& session_ID);
  
  void setAccount(std::shared_ptr<Account> account);

  SessionID getMarketSessionID() const;
  void setMarketSessionID(const SessionID& session_ID);

  SessionID getOrderSessionID() const;
  void setOrderSessionID(const SessionID& session_ID);

  void addMarket(const Market market);
  void addMarketSnapshot(const MarketSnapshot snapshot);
  void addMarketOrder(const MarketOrder marketOrder);
  void removeMarketOrder(const std::string posID);
  void updateMarketOrder(const MarketOrder& marketOrder, const bool isUnsolicited = false);

  std::shared_ptr<MarketOrder> getMarketOrder(const std::string fxcm_pos_id) const;
  std::shared_ptr<MarketOrder> getMarketOrder(const ClOrdID clOrdID) const;

  void addMarketDetail(const MarketDetail& marketDetail);

  void addSysParam(const std::string key, const std::string value);
  std::string getSysParam(const std::string key);

  void addSubscription(const std::string symbol);
  void removeSubscription(const std::string symbol);

  // bool setStrategy(Strategy* strategy);
}; // class fixmanager
}; // namespace idefix

#endif //IDEFIX_FIXMANAGER_H
