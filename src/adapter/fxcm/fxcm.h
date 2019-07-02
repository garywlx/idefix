#pragma once

#include "core/adapter.h"

#include <unordered_map>
#include <vector>

#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/fix44/Reject.h>
#include <quickfix/fix44/CollateralInquiryAck.h>
#include <quickfix/fix44/CollateralReport.h>
#include <quickfix/fix44/ExecutionReport.h>
#include <quickfix/fix44/MarketDataRequestReject.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/PositionReport.h>
#include <quickfix/fix44/RequestForPositionsAck.h>
#include <quickfix/fix44/SecurityList.h>
#include <quickfix/fix44/TradingSessionStatus.h>
#include <quickfix/fix44/BusinessMessageReject.h>
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
class FXCM: public MessageCracker, public Application, public NetworkAdapter {
public:

	enum FXCM_FIX_FIELDS {
	  	FXCM_SYM_ID                  = 9000, // Int
	    FXCM_SYM_PRECISION           = 9001, // Int
	    FXCM_SYM_POINT_SIZE          = 9002, // Float
	    FXCM_SYM_INTEREST_BUY        = 9003, // Float
	    FXCM_SYM_INTEREST_SELL       = 9004, // Float
	    FXCM_SYM_SORT_ORDER          = 9005, // Int
	    // 6-10 Reserved
	    FXCM_TIMING_INTERVAL         = 9011, // Int
	    FXCM_START_DATE              = 9012, // UTCDate
	    FXCM_START_TIME              = 9013, // UTCTimeOnly
	    FXCM_END_DATE                = 9014, // UTCDate
	    FXCM_END_TIME                = 9015, // UTCTimeOnly
	    FXCM_NO_PARAMS               = 9016, // Int
	    FXCM_PARAM_NAME              = 9017, // String
	    FXCM_PARAM_VALUE             = 9018, // String
	    FXCM_SERVER_TIMEZONE         = 9019, // Int
	    FXCM_CONTINOUS_FLAG          = 9020, // Int
	    // 21-24 Not in use for Buyer Interface
	    FXCM_REQUEST_REJECT_REASON   = 9025, // Int
	    // 26-28 Not in use for Buyer Interface
	    FXCM_ERROR_DETAILS           = 9029, // String
	    FXCM_SERVER_TIMEZONE_NAME    = 9030, // String
	    // 31-35 Not in use for Buyer Interface
	    // 36-37 Reserved
	    FXCM_USED_MARGIN             = 9038, // Float
	    // 39 Reserved
	    FXCM_POS_INTEREST            = 9040, // Float
	    FXCM_POS_ID                  = 9041, // String
	    FXCM_POS_OPEN_TIME           = 9042, // UTCTimestamp
	    FXCM_CLOSE_SETTLE_PRICE      = 9043, // Float
	    FXCM_POS_CLOSE_TIME          = 9044, // UTCTimestamp
	    FXCM_MARGIN_CALL             = 9045, // String
	    FXCM_USED_MARGIN3            = 9046, // Float
	    FXCM_CASH_DAILY              = 9047, // Float
	    FXCM_CLOSE_CLORDID           = 9048, // String
	    FXCM_CLOSE_SECONDARY_CLORDID = 9049, // String
	    FXCM_ORD_TYPE                = 9050, // String
	    FXCM_ORD_STATUS              = 9051, // String
	    FXCM_CLOSE_PNL               = 9052, // String
	    FXCM_POS_COMMISSION          = 9053, // Float
	    FXCM_CLOSE_ORDERID           = 9054, // String
	    // 55-59 Not set
	    FXCM_MAX_NO_RESULTS          = 9060, // Int
	    FXCM_PEG_FLUCTUATE_PTS       = 9061, // Int
	    // 62-75 Not set
	    FXCM_SUBSCRIPTION_STATUS     = 9076, // String
	    // 77 Not set
	    FXCM_POS_ID_REF              = 9078, // String
	    FXCM_CONTINGENCY_ID          = 9079, // String
	    FXCM_FIELD_PRODUCT_ID        = 9080, // Int
	    // 81-89 Not set
	    FXCM_COND_DIST_STOP          = 9090, // Float
	    FXCM_COND_DIST_LIMIT         = 9091, // Float
	    FXCM_COND_DIST_ENTRY_STOP    = 9092, // Float
	    FXCM_COND_DIST_ENTRY_LIMIT   = 9093, // Float
	    FXCM_MAX_QUANTITY            = 9094, // Float
	    FXCM_MIN_QUANTITY            = 9095, // Float
	    FXCM_TRADING_STATUS          = 9096  // String
	  };

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
	void onMessage(const FIX44::Reject& r, const SessionID& sessionID);
	void onMessage(const FIX44::TradingSessionStatus& tss, const SessionID& sessionID);
	void onMessage(const FIX44::CollateralInquiryAck& ack, const SessionID& sessionID);
	void onMessage(const FIX44::CollateralReport& cr, const SessionID& sessionID);
	void onMessage(const FIX44::RequestForPositionsAck& ack, const SessionID& sessionID);
	void onMessage(const FIX44::PositionReport& pr, const SessionID& sessionID);
	void onMessage(const FIX44::MarketDataRequestReject& mdr, const SessionID& sessionID);
	void onMessage(const FIX44::MarketDataSnapshotFullRefresh& mds, const SessionID& sessionID);
	void onMessage(const FIX44::ExecutionReport& er, const SessionID& sessionID);
	void onMessage(const FIX44::BusinessMessageReject& msg, const SessionID& sessionID);
	// void onMessage(const FIX44::AllocationReportAck& ack, const SessionID& sessionID);
	// void onMessage(const FIX44::AllocationReport& ar, const SessionID& sessionID);

	/// -----
	/// NetworkAdapter API
	/// -----

	// Starts the FIX session. Throws FIX::ConfigError exception if our configuration settings
	// do not pass validation required to construct SessionSettings 
	void connect() noexcept;
	// Logout and end session 
	void disconnect() noexcept;
	// Login 
	void login() noexcept;
    // Logout
	void logout() noexcept;
	// subscribe to market data
	void subscribeMarketData(const std::string& symbol);
	// unsubscribe market data
	void unsubscribeMarketData(const std::string& symbol);
	// check wether there is a connection available
	bool isConnected();
	// send order to market
	void sendOrder(const std::shared_ptr<Order> order);
	// subscribe to position update
	void subscribePositionUpdates(const std::string& accountid);
	// send order status request
	void sendOrderStatusRequest(const std::string& accountid, const std::string& orderid, const std::string& symbol);
	// send order mass status request
	void sendOrderMassStatusRequest(const std::string& accountid);
	// send position report request
	void sendPositionReportRequest(const std::string& accountid, const enums::ExecutionType exec_type);

private:
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
	// void sendPositionRequest();

};
};