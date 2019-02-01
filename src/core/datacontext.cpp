#include "datacontext.h"
#include "core/utility.h"
#include "core/logger.h"

#include <functional>

namespace idefix {
DataContext::DataContext(std::unique_ptr<NetworkAdapter> network_adapter): m_network_ptr( std::move( network_adapter ) ), m_is_connected(false) {

}

/// ----------------------------------------------------------------------------------------------
/// DATA MANAGEMENT
/// ----------------------------------------------------------------------------------------------

/**
 * Gets account list
 *
 * @return std::vector<std::shared_ptr<Account>>
 */
std::vector< std::shared_ptr<Account> > DataContext::getAccounts() {
	return m_account_list;
}

/**
 * Get instrument set
 * 
 * @return std::vector< std::shared_ptr<Instrument> >
 */
std::vector< std::shared_ptr<Instrument> > DataContext::getInstruments() {
	return m_instrument_list;
}

/**
 * Get instrument
 * 
 * @param const std::string& symbol
 * @return std::shared_ptr<Instrument>
 */
std::shared_ptr<Instrument> DataContext::getInstrument(const std::string& symbol) {
	// prevent RAII race conditions
	std::lock_guard<std::mutex> lock( m_mutex );
	auto it = std::find_if( m_instrument_list.begin(), m_instrument_list.end(), [&](std::shared_ptr<Instrument> lh) {
		return lh->getSymbol() == symbol;
	});
	if ( it == m_instrument_list.end() ) return nullptr;
	return *it;
}

/**
 * Add account
 * 
 * @param std:shared_ptr<Account> account
 */
void DataContext::addAccount(std::shared_ptr<Account> account) {
	std::lock_guard<std::mutex> lock( m_mutex );

	auto it = std::find_if( m_account_list.begin(), m_account_list.end(), [&]( std::shared_ptr<Account> lh) {
		return lh->getAccountID() == account->getAccountID();
	});

	if ( it != m_account_list.end() ) return;

	m_account_list.push_back( std::move( account ) );
}

/**
 * Add instrument
 * 
 * @param std::shared_ptr<Instrument> instrument
 */
void DataContext::addInstrument(std::shared_ptr<Instrument> instrument) {
	// prevent RAII race conditions
	std::lock_guard<std::mutex> lock( m_mutex );

	auto it = std::find_if( m_instrument_list.begin(), m_instrument_list.end(), [&](std::shared_ptr<Instrument> lh) {
		return lh->getSymbol() == instrument->getSymbol();
	});
	if ( it != m_instrument_list.end() ) return;

	// add instrument to list
	m_instrument_list.push_back( std::move( instrument ) );
}

/**
 * Add instrument by symbol
 * 
 * @param const std::string& symbol 
 */
void DataContext::addInstrument(const std::string& symbol) {
	addInstrument( std::make_shared<Instrument>( symbol ) );
} 

/**
 * Connect to exchange and initialize signal/slots
 */
void DataContext::connect() {
	if ( m_network_ptr ) {
		// connect signals/slots
		connectNetworkSlots();

		// connect to exchange
		m_network_ptr->connect();
	}
}

/**
 * Disconnect from exchange
 */
void DataContext::disconnect() {
	if ( m_network_ptr ) {
		// unsubscribe from instruments
		for ( auto& instrument : m_instrument_list ) {
			if ( instrument->getVolume() > 0 ) {
				unsubscribe( instrument->getSymbol() );
			}
		}

		// disconnect from exchange
		m_network_ptr->disconnect();
	}
}

/**
 * Send subscription request to exchange
 * 
 * @param const std::string& symbol
 */
void DataContext::subscribe(const std::string& symbol) {
	if ( m_network_ptr ) {
		m_network_ptr->subscribeMarketData( symbol );
	}
}

/**
 * Send unsubscription request to exchange
 * 
 * @param const std::string& symbol
 */
void DataContext::unsubscribe(const std::string& symbol) {
	if ( m_network_ptr ) {
		m_network_ptr->unsubscribeMarketData( symbol );
	}
}

/**
 * Determines if this account (broker) supports hedging.
 * 
 * @return bool
 */
bool DataContext::supportsHedging() {
	auto value = getExchangeSetting( "SUPPORTS_HEDGING" );
	if ( value.empty() ) return false;
	return ( value == "Y" );
}

// Gets value of exchange setting
std::string DataContext::getExchangeSetting(const std::string& key) {
	return FindInMap( m_exchange_settings, key );
}

/**
 * Determines if trading desk is open
 * 
 * @return bool
 */
bool DataContext::isTradingDeskOpen() {
	return m_trading_desk_open;
}

/// ----------------------------------------------------------------------------------------------
/// ORDER MANAGEMENT
/// ----------------------------------------------------------------------------------------------

/**
 * Send order to exchange
 * 
 * @param const std::string symbol
 * @param const enums::OrderAction action
 * @param const double qty
 * @param const double price = 0 == market | > 0 == pending order
 * @param const double sl = 0 == no sl | > 0 == stop order
 * @param const double tp = 0 == no tp | > 0 == limit order
 */
void DataContext::createOrder(const std::string symbol, const enums::OrderAction action, const double qty, const double price, const double sl, const double tp) {
	if ( symbol.empty() ) {
		onError( "sendOrder: symbol is empty." );
		return;
	}

	if ( qty <= 0 ) {
		onError( "sendOrder: qty is <= 0." );
		return;
	}

	auto accounts = getAccounts();
	if ( accounts.size() == 0 ) {
		onError( "sendOrder: Account list is empty." );
		return;
	}

	auto order = std::make_shared<Order>();
	order->setAction( action );
	order->setSymbol( symbol );
	order->setQuantity( qty );
	order->setAccountID( accounts.at( 0 )->getAccountID() );

	if ( price == 0 ) {
		order->setType( enums::OrderType::MARKET );
	} 

	if ( sl > 0 ) {
		order->setStopPrice( sl );
	}

	if ( tp > 0 ) {
		order->setLimitPrice( tp );
	}

	order->setTIF( enums::TIF::FOK );

	// send order
	m_network_ptr->sendOrder( std::move( order ) );
}

/**
 * Close filled order
 * 
 * @param const std::string& order_id
 */
void DataContext::closeOrder(const std::string& order_id) {
	if ( order_id.empty() ) {
		onError( "closeOrder: OrderID is empty." );
		return;
	}

	auto order = std::make_shared<Order>();
	order->setOrderID( order_id );
	order->setType( enums::OrderType::CLOSE );

	bool found = false;
	for ( auto& active_order : m_order_list ) {
		if ( active_order->getOrderID() == order_id ) {
			order->setQuantity( active_order->getQuantity() );
			order->setTIF( active_order->getTIF() );
			order->setAction( active_order->getAction() );
			order->setAccountID( active_order->getAccountID() );
			order->setSymbol( active_order->getSymbol() );
			found = true;
			break;
		}
	}

	if ( found ) {
		// send to exchange
		m_network_ptr->sendOrder( std::move( order ) );
	}
}

/**
 * Cancels all of the open orders for this instrument
 * 
 * @param std::shared_ptr<Instrument> instrument
 */
void DataContext::cancelOrders(std::shared_ptr<Instrument> instrument) {
	if ( instrument == nullptr ) return;

	std::vector<Order> orders;
	for ( auto& order_ptr : m_order_list ) {
		if ( order_ptr == nullptr ) continue;
		if ( instrument->getSymbol() == order_ptr->getSymbol() ) {
			orders.push_back( *order_ptr );
		}
	}

	if ( orders.size() > 0 ) {
		cancelOrders( orders );
	}
}

/**
 * Use this method to cancel one or more existing orders.
 * 
 * @param const std::vector<Order> orders
 */
void DataContext::cancelOrders(const std::vector<Order> orders) {

}

// /**
//  * Creates a new 'Market' order.
//  * 
//  * @param std::shared_ptr<Instrument> instrument 
//  * @param enums::OrderAction          action
//  * @param double                      qty
//  * @return Order
//  */
// std::shared_ptr<Order> DataContext::createMarketOrder(std::shared_ptr<Instrument> instrument, enums::OrderAction action, double qty) {
// 	return nullptr;
// }

// /**
//  * Creates a new 'Market' order with reference id.
//  * 
//  * @param std::shared_ptr<Instrument> instrument 
//  * @param const std::string           ref_id
//  * @param enums::OrderAction          action
//  * @param double                      qty
//  * @return Order
//  */
// std::shared_ptr<Order> DataContext::createMarketOrder(std::shared_ptr<Instrument> instrument, const std::string ref_id, enums::OrderAction action, double qty) {
// 	return nullptr;
// }

// /**
//  * Creates a new 'Stop' order.
//  * 
//  * @param std::shared_ptr<Instrument> instrument 
//  * @param enums::OrderAction          action
//  * @param enums::TIF                  tif
//  * @param double                      qty
//  * @param double                      stop_price
//  * @return Order
//  */
// std::shared_ptr<Order> DataContext::createStopOrder(std::shared_ptr<Instrument> instrument, enums::OrderAction action, enums::TIF tif, double qty, double stop_price) {
// 	return nullptr;
// }

// *
//  * Creates a new 'Stop' order with reference id.
//  * 
//  * @param std::shared_ptr<Instrument> instrument 
//  * @param const std::string           ref_id
//  * @param enums::OrderAction          action
//  * @param enums::TIF                  tif
//  * @param double                      qty
//  * @param double                      stop_price
//  * @return Order
 
// std::shared_ptr<Order> DataContext::createStopOrder(std::shared_ptr<Instrument> instrument, const std::string ref_id, enums::OrderAction action, enums::TIF tif, double qty, double stop_price) {
// 	return nullptr;
// }

// /**
//  * Creates a new 'Limit' order.
//  * 
//  * @param std::shared_ptr<Instrument> instrument 
//  * @param enums::OrderAction          action
//  * @param enums::TIF                  tif
//  * @param double                      qty
//  * @param double                      limit_price
//  * @return Order
//  */
// std::shared_ptr<Order> DataContext::createLimitOrder(std::shared_ptr<Instrument> instrument, enums::OrderAction action, enums::TIF tif, double qty, double limit_price) {
// 	return nullptr;
// }

// /**
//  * Creates a new 'Limit' order with reference id.
//  * 
//  * @param std::shared_ptr<Instrument> instrument 
//  * @param const std::string           ref_id
//  * @param enums::OrderAction          action
//  * @param enums::TIF                  tif
//  * @param double                      qty
//  * @param double                      limit_price
//  * @return Order
//  */
// std::shared_ptr<Order> DataContext::createLimitOrder(std::shared_ptr<Instrument> instrument, const std::string ref_id, enums::OrderAction action, enums::TIF tif, double qty, double limit_price) {
// 	return nullptr;
// }

// /**
//  * Submit orders
//  * 
//  * @param const std::vector<Order> orders
//  */
// void DataContext::submitOrders(const std::vector<Order> orders) {

// }

/**
 * Gets the list of active orders
 *
 * @return std::vector< std::shared_ptr<Order> >
 */
std::vector< std::shared_ptr<Order> > DataContext::getActiveOrders() {
	return m_order_list;
}


/// ----------------------------------------------------------------------------------------------
/// SLOTS
/// ----------------------------------------------------------------------------------------------

/**
 * Slot to register all available instruments
 * 
 * @param const std::vector<Instrument> instruments
 */
void DataContext::slotExchangeInstrumentList(const std::vector<Instrument> instruments) {
	for ( const auto& instrument : instruments ) {
		addInstrument( std::make_shared<Instrument>( instrument ) );
	}
}

/**
 * Slot to register the exchange settings
 * 
 * @param const ExchangeSettingsMap settings
 */
void DataContext::slotExchangeSettings(const ExchangeSettingsMap settings) {
	for ( const auto& setting : settings ) {
		m_exchange_settings.emplace( setting.first, setting.second );
	}
}

/**
 * Slot when exchange is ready with initializing, connecting and gathering settings data
 */
void DataContext::slotExchangeReady() {
	for ( const auto& setting : m_exchange_settings ) {
		SPDLOG_INFO("Setting {} = {}", setting.first, setting.second );
	}
	SPDLOG_INFO("Registered instruments: {:d}", m_instrument_list.size() );
	SPDLOG_INFO("Exchange ready.");

	// emit signal
	onReady();
}

/**
 * Slot when exchange is successfully connected
 */
void DataContext::slotExchangeConnected() {
	m_is_connected = true;

	SPDLOG_INFO("Exchange connected.");

	// emit signal
	onConnected();
}

/**
 * Slot when exchange is successfully disconnected
 */
void DataContext::slotExchangeDisconnected() {
	m_is_connected = false;

	SPDLOG_INFO("Exchange disconnected.");

	// emit signal
	onDisconnected();
}

/**
 * Slot when exchange logged in
 * 
 * @param const std::string session_name either order|market
 */
void DataContext::slotExchangeLogon(const std::string session_name) {
	if ( session_name == "order" ) {
		SPDLOG_INFO("OrderSession login.");
	}
	if ( session_name == "market" ) {
		SPDLOG_INFO("MarketSession login.");
	}
}

/**
 * Slot when exchange logged out
 * 
 * @param const std::string session_name either order|market
 */
void DataContext::slotExchangeLogout(const std::string session_name) {
	if ( session_name == "order" ) {
		SPDLOG_INFO("OrderSession logout.");
	}
	if ( session_name == "market" ) {
		SPDLOG_INFO("MarketSession logout.");
	}
}

/**
 * Slot when exchange session is created
 * 
 * @param const std::string session_name either order|market
 */
void DataContext::slotExchangeSessionCreated(const std::string session_name) {
	SPDLOG_INFO("Exchange Session {} created.", session_name );
}

/**
 * Slot when exchange signals a warning message
 * 
 * @param const std::string msg 
 */
void DataContext::slotExchangeWarning(const std::string msg) {
	SPDLOG_WARN( "Exchange: {}", msg );
}

/**
 * Slot when exchange signals an error message
 * 
 * @param const std::string msg
 */
void DataContext::slotExchangeError(const std::string msg) {
	SPDLOG_ERROR( "Exchange: {}", msg );
}

/**
 * Slot when exchange signals if trading desk is open or not
 * 
 * @param const bool open 
 */
void DataContext::slotExchangeTradingDeskChange(const bool open) {
	m_trading_desk_open = open;

	if ( open ) {
		SPDLOG_INFO("Trading Desk is open.");
	} else {
		SPDLOG_INFO("Trading Desk is closed.");
	}
}

/**
 * Slot when exchange sends an account id
 * 
 * @param const std::string accountid 
 */
void DataContext::slotExchangeAccountID(const std::string accountid) {
	// add account to account list
	auto account = std::make_shared<Account>();
	account->setAccountID( accountid );
	account->setBalance( 0 );

	addAccount( std::move( account ) );
}

/**
 * Slot when exchange signals a change in balance
 * 
 * @param const std::string accountid
 * @param const double      balance
 */
void DataContext::slotExchangeBalanceChanged(const std::string accountid, const double balance) {
	std::lock_guard<std::mutex> lock( m_mutex );

	auto it = std::find_if( m_account_list.begin(), m_account_list.end(), [&](const std::shared_ptr<Account> lh){
		return lh->getAccountID() == accountid;
	});

	if ( it == m_account_list.end() ) {
		onError( fmt::format( "Account {} not found.", accountid ) );
		return;
	}

	(*it)->setBalance( balance );

	SPDLOG_INFO("Account: {} Balance: {:.2f}", accountid, balance );
}

/**
 * Account settings
 * 
 * @param const ExchangeCollateralSettingsMap map
 */
void DataContext::slotExchangeCollateralSettings(const ExchangeCollateralSettingsMap map) {
	// add to exchange settings
	for ( auto const &setting : map ) {
		m_exchange_settings.emplace( setting.first, setting.second );
	}
}

void DataContext::slotExchangePositionReport(const ExchangePositionReport report) {
	SPDLOG_INFO("PositionReport: \n AccountID: {}\nSymbol: {}\nPositionID: {}\nPosition OpenTime: {}", 
		report.account_id, report.symbol, report.position_id, report.pos_open_time);
}

void DataContext::slotExchangeMarketDataReject(const std::string reason) {
	SPDLOG_INFO("MarketDataRequestReject: {}", reason);
}

void DataContext::slotExchangeTick(const ExchangeTick tick) {
	auto instrument = getInstrument( tick.symbol );
	if ( instrument == nullptr ) {
		SPDLOG_WARN( "Slot called but no instrument found. {}", tick.symbol );
		return;
	}
	// add tick
	instrument->addTick( tick );
	// emit signal
	onTick( instrument );
}

/**
 * Slot when exchange signals a change on order
 * 
 * @param std::shared_ptr<Order> order
 */
void DataContext::slotExchangeOrder(std::shared_ptr<Order> order) {
	std::lock_guard<std::mutex> lock( m_mutex );

	if ( order->getStatus() == enums::OrderStatus::NEW ) {
		// New Order arrived at exchange
		// try to add this one to the active orders list
		auto it = std::find_if( m_order_list.begin(), m_order_list.end(), [&](const std::shared_ptr<Order> lh_order) {
			return ( lh_order->getOrderID() == order->getOrderID() || lh_order->getClientOrderID() == order->getClientOrderID() );
		});	

		if ( it == m_order_list.end() ) {
			// not found, add to list
			m_order_list.push_back( order );

			onSuccess( fmt::format( "Order {} changed to status new.", order->getOrderID() ) );
		}
	} else if ( order->getStatus() == enums::OrderStatus::REJECTED ) {
		// Order has been rejected
		// try to find the order and remove it from list
		
		onError( fmt::format( "Order {} has been rejected.", order->getOrderID() ) );
	} else if ( order->getStatus() == enums::OrderStatus::CANCELED ) {
		// Order has been CANCELED
		// try to find the order and remove it from list
		
		if ( removeOrder( order->getOrderID() ) ) {
			onSuccess( fmt::format( "Order {} has been canceled.", order->getOrderID() ) );	
		}
	} else if ( order->getStatus() == enums::OrderStatus::STOPPED ) {
		// Order has been STOPPED
		// try to find the order and remove it from list
		
		if ( removeOrder( order->getOrderID() ) ) {
			onSuccess( fmt::format( "Order {} has been stopped.", order->getOrderID() ) );
		}
	} else if ( order->getStatus() == enums::OrderStatus::FILLED || order->getStatus() == enums::OrderStatus::PARTIAL_FILLED ) {
		// Order has been FILLED || PARTIAL FILLED
		// try to find the order and update
		
		auto it = std::find_if( m_order_list.begin(), m_order_list.end(), [&](const std::shared_ptr<Order> lh_order){
			return ( lh_order->getOrderID() == order->getOrderID() );
		});

		if ( it == m_order_list.end() ) {
			// not found
			onError( fmt::format( "Order {} not found.", order->getOrderID() ) );
		} else {
			auto list_order = *it;

			list_order->setStatus( order->getStatus() );
			list_order->setStatusMsg( order->getStatusMsg() );
			list_order->setStopPrice( order->getStopPrice() );
			list_order->setLimitPrice( order->getLimitPrice() );
			list_order->setEntryPrice( order->getEntryPrice() );
			list_order->setLastFillPrice( order->getLastFillPrice() );
			list_order->setLastFillTime( order->getLastFillTime() );
			list_order->setFilled( order->getFilled() );

			if ( order->getStatus() == enums::OrderStatus::PARTIAL_FILLED ) {
				onSuccess( fmt::format( "Order {} has been partial filled.", order->getOrderID() ) );
			} else {
				onSuccess( fmt::format( "Order {} has been filled.", order->getOrderID() ) );	
			}
		}
	} else {
		// respond with error signal
		onError( fmt::format( "Order {} status unknown.", order->getOrderID() ) );
	}

	// Signal
	onOrderChange( order );
}

///
/// PRIVATE METHODS
/// 

/**
 * Connect network signals with data context slots
 */
void DataContext::connectNetworkSlots() {
	if ( m_network_ptr == nullptr ) return;

	// slots
	// Ready
	m_network_ptr->onExchangeReady.connect( std::bind( &DataContext::slotExchangeReady, this ) );
	// Logon
	m_network_ptr->onExchangeLogon.connect( std::bind( &DataContext::slotExchangeLogon, this, std::placeholders::_1 ) );
	// Logout
	m_network_ptr->onExchangeLogout.connect( std::bind( &DataContext::slotExchangeLogout, this, std::placeholders::_1 ) );
	// Connected
	m_network_ptr->onExchangeConnected.connect( std::bind( &DataContext::slotExchangeConnected, this ) );
	// Disconnected
	m_network_ptr->onExchangeDisconnected.connect( std::bind( &DataContext::slotExchangeDisconnected, this ) );
	// Instrument List
	m_network_ptr->onExchangeInstrumentList.connect( std::bind( &DataContext::slotExchangeInstrumentList, this, std::placeholders::_1 ) );
	// Settings
	m_network_ptr->onExchangeSettings.connect( std::bind( &DataContext::slotExchangeSettings, this, std::placeholders::_1 ) );
	// Session created
	m_network_ptr->onExchangeSessionCreated.connect( std::bind( &DataContext::slotExchangeSessionCreated, this, std::placeholders::_1 ) );
	// Warning
	m_network_ptr->onExchangeWarning.connect( std::bind( &DataContext::slotExchangeWarning, this, std::placeholders::_1 ) );
	// Error
	m_network_ptr->onExchangeError.connect( std::bind( &DataContext::slotExchangeError, this, std::placeholders::_1 ) );
	// Trading Desk Change
	m_network_ptr->onExchangeTradingDeskChange.connect( std::bind( &DataContext::slotExchangeTradingDeskChange, this, std::placeholders::_1 ) );
	// Account ID
	m_network_ptr->onExchangeAccountID.connect( std::bind( &DataContext::slotExchangeAccountID, this, std::placeholders::_1 ) );
	// Balance changed
	m_network_ptr->onExchangeBalanceChange.connect( std::bind( &DataContext::slotExchangeBalanceChanged, this, std::placeholders::_1, std::placeholders::_2 ) );
	// Collateral Settings
	m_network_ptr->onExchangeCollateralSettings.connect( std::bind( &DataContext::slotExchangeCollateralSettings, this, std::placeholders::_1 ) );
	// Position report
	m_network_ptr->onExchangePositionReport.connect( std::bind( &DataContext::slotExchangePositionReport, this, std::placeholders::_1 ) );
	// Market Data reject
	m_network_ptr->onExchangeMarketDataReject.connect( std::bind( &DataContext::slotExchangeMarketDataReject, this, std::placeholders::_1 ) );
	// Tick
	m_network_ptr->onExchangeTick.connect( std::bind( &DataContext::slotExchangeTick, this, std::placeholders::_1 ) );
	// Order change
	m_network_ptr->onExchangeOrder.connect( std::bind( &DataContext::slotExchangeOrder, this, std::placeholders::_1 ) );
}

/**
 * Gets status of connection
 * 
 * @return bool
 */
bool DataContext::isConnected() {
	return m_is_connected;
}

/**
 * Remove order from order list 
 * 
 * @param const std::string&  order_id
 * @return bool
 */
bool DataContext::removeOrder(const std::string& order_id) {
	std::lock_guard<std::mutex> lock( m_mutex );

	auto it = std::find_if( m_order_list.begin(), m_order_list.end(), [&](std::shared_ptr<Order> order) {
		return order->getOrderID() == order_id;
	});

	// not found
	if ( it == m_order_list.end() ) {
		onError( fmt::format( "removeOrder: order {} not found.", order_id ) );
		return false;
	}

	m_order_list.erase( it );

	return true;
}
};