#pragma once

#include "../../core/adapter.h"
#include "fxcmfields.h"

#include <unordered_map>
#include <vector>

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
// #include <quickfix/fix44/AllocationReportAck.h>
// #include <quickfix/fix44/AllocationReport.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/Session.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/Mutex.h>
#include <quickfix/Fields.h>

using namespace FIX;
using namespace std;

namespace idefix {
class FXCM: public MessageCracker, public Application, public virtual NetworkAdapter {
public:
	// Synchronizing multithreading
  	mutable FIX::Mutex m_mutex;

  	FXCM();

  	// FIX Namespace. These are callbacks which indicate when the session is created,
	// when we logon and logout, and when messages are exchanged 
	void onCreate(const SessionID& sessionID);
	void onLogon(const SessionID& sessionID);
	void onLogout(const SessionID& sessionID);
	void toAdmin(Message& message, const SessionID& sessionID);
	void toApp(Message &message, const SessionID &sessionID)
	throw( FIX::DoNotSend );
	void fromAdmin(const Message& message, const SessionID& sessionID)
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon );
	void fromApp(const Message& message, const SessionID& sessionID)
	throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType );

	// Overloaded onMessage methods used in conjuction with MessageCracker class. FIX::MessageCracker
	// receives a generic Message in the FIX fromApp and fromAdmin callbacks, constructs the
	// message sub type and invokes the appropriate onMessage method below.
	void onMessage(const FIX44::TradingSessionStatus& tss, const SessionID& sessionID);
	void onMessage(const FIX44::CollateralInquiryAck& ack, const SessionID& sessionID);
	void onMessage(const FIX44::CollateralReport& cr, const SessionID& sessionID);
	void onMessage(const FIX44::RequestForPositionsAck& ack, const SessionID& sessionID);
	void onMessage(const FIX44::PositionReport& pr, const SessionID& sessionID);
	void onMessage(const FIX44::MarketDataRequestReject& mdr, const SessionID& sessionID);
	void onMessage(const FIX44::MarketDataSnapshotFullRefresh& mds, const SessionID& sessionID);
	void onMessage(const FIX44::ExecutionReport& er, const SessionID& sessionID);
	// void onMessage(const FIX44::AllocationReportAck& ack, const SessionID& sessionID);
	// void onMessage(const FIX44::AllocationReport& ar, const SessionID& sessionID);

	// Starts the FIX session. Throws FIX::ConfigError exception if our configuration settings
	// do not pass validation required to construct SessionSettings 
	void connect() noexcept;
	// Logout and end session 
	void disconnect() noexcept;
	// check wether there is a connection available
	bool isConnected();
private:
	// Custom FXCM FIX fields
	// enum FXCM_FIX_FIELDS
	// {
	// 	FXCM_FIELD_PRODUCT_ID      = 9080,
	// 	FXCM_POS_ID                = 9041,
	// 	FXCM_POS_OPEN_TIME         = 9042,
	// 	FXCM_ERROR_DETAILS         = 9029,
	// 	FXCM_REQUEST_REJECT_REASON = 9025,
	// 	FXCM_USED_MARGIN           = 9038,
	// 	FXCM_POS_CLOSE_TIME        = 9044,
	// 	FXCM_MARGIN_CALL           = 9045,
	// 	FXCM_ORD_TYPE              = 9050,
	// 	FXCM_ORD_STATUS            = 9051,
	// 	FXCM_CLOSE_PNL             = 9052,
	// 	FXCM_SYM_POINT_SIZE        = 9002,
	// 	FXCM_SYM_PRECISION         = 9001,
	// 	FXCM_TRADING_STATUS        = 9096,
	// 	FXCM_PEG_FLUCTUATE_PTS     = 9061,
	// 	FXCM_NO_PARAMS             = 9016,
	// 	FXCM_PARAM_NAME            = 9017,
	// 	FXCM_PARAM_VALUE           = 9018
	// };

	// Store sessions
	typedef std::unordered_map<std::string, SessionID> SessionMap;
	SessionMap m_sessions;
	// store request id counter
	unsigned int m_request_id;
	// Pointer to SessionSettings from SessionSettingsFile
	SessionSettings *m_settings_ptr;
	// Pointer to File Store Factory
	FileStoreFactory *m_store_factory_ptr;
	// Pointer to File Log Factory
	FileLogFactory *m_log_factory_ptr;
	// Pointer to Socket
	SocketInitiator *m_initiator_ptr;
	// store all accountids
	std::vector<std::string> m_account_vec;

	// get session id by name: order|market
	SessionID getSessID(const std::string& name) const;
	// get next request id for requests
	std::string nextRequestID();
	// store an account
	void recordAccount(std::string accountID);

	// Requests
	// Send trading status request
	void sendTradingStatusRequest();
	// Send account request
	void sendAccountsRequest();
	// Send position request
	void sendPositionRequest();
	// Send market data subscription
	void sendMarketDataSubscription(const std::string& symbol);
	// Send market data unsubscription
	void sendMarketDataUnsubscription(const std::string& symbol);

};
};