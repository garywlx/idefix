/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef IDEFIX_FIXAPP_H
#define IDEFIX_FIXAPP_H

#include <iostream>
#include <vector>
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
#include <quickfix/MessageCracker.h>
#include <quickfix/Session.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>

using namespace std;
using namespace FIX;

class FIXApp: public MessageCracker, public Application {
private:
  SessionSettings *_settings;
  FileStoreFactory *_store_factory;
  FileLogFactory *_log_factory;
  SocketInitiator *_initiator;

  // used as a counter for producing unique request identifiers
  unsigned int _requestID;
  // obsolete
 // SessionID _sessionID;
  // the session id for market data, such as tick prices
  SessionID _market_sessionID;
  // the session id for order management, such as open/closing positions
  SessionID _order_sessionID;
  // hold the account ids in a list
  vector<string> _list_accountID;

  // Custom FXCM FIX fields
  enum FXCM_FIX_FIELDS
  {
    FXCM_FIELD_PRODUCT_ID      = 9080,
    FXCM_POS_ID                = 9041,
    FXCM_POS_OPEN_TIME         = 9042,
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
  FIXApp();
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

  // Starts the FIX session. Throws FIX::ConfigError exception if our configuration settings
  // do not pass validation required to construct SessionSettings
  void StartSession(const string settingsfile);
  // Logout and end session
  void EndSession();

  // Sends TradingSessionStatusRequest message in order to receive as a response the
  // TradingSessionStatus message
  void GetTradingStatus();
  // Sends the CollateralInquiry message in order to receive as a response the
  // CollateralReport message.
  void GetAccounts();
  // Sends RequestForPositions which will return PositionReport messages if positions
  // matching the requested criteria exist; otherwise, a RequestForPositionsAck will be
  // sent with the acknowledgement that no positions exist. In our example, we request
  // positions for all accounts under our login
  void GetPositions();
  // Subscribes to the EUR/USD trading security
  void SubscribeMarketData();
  // Unsubscribes from the EUR/USD trading security
  void UnsubscribeMarketData();
  // Sends a basic NewOrderSingle message to buy EUR/USD at the
  // current market price
  void MarketOrder();
  // Generate string value used to populate the fields in each message
  // which are used as a custom identifier
  string NextRequestID();
  // Adds string accountIDs to our vector<string> being used to
  // account for the accountIDs under our login
  void RecordAccount(string accountID);

  const FIX::Dictionary* GetSessionSettingsPtr(const SessionID& session_ID);
  bool IsMarketDataSession(const SessionID& session_ID);
  bool IsOrderSession(const SessionID& session_ID);
};


#endif //IDEFIX_FIXAPP_H
