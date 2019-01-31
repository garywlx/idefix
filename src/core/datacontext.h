#pragma once

#include <string>
#include <mutex>
#include <nod/nod.hpp>
#include <vector>
#include <memory>
#include <unordered_map>

#include "core/adapter.h"
#include "core/instrument.h"
#include "core/exchangetypes.h"
#include "core/utility.h"
#include "core/execution.h"
#include "core/order.h"
#include "core/enums.h"

namespace idefix {
class DataContext {
public:
	DataContext(std::unique_ptr<NetworkAdapter> network_adapter);

	///
	/// DATA MANAGEMENT
	/// 
	
	// get instrument list
	std::vector< std::shared_ptr<Instrument> > getInstruments();
	// get instrument
	std::shared_ptr<Instrument> getInstrument(const std::string& symbol);
	// check wether the instrument is available
	bool hasInstrument(const std::string& symbol);
	// connect to exchange
	void connect();
	// disconnect from exchange
	void disconnect();
	// return if connected to exchange
	bool isConnected();
	// subscribe to price feed
	void subscribe(const std::string& symbol);
	// unsubscribe from price feed
	void unsubscribe(const std::string& symbol);
	// Determines if this account (broker) supports hedging.
	bool supportsHedging();
	// Gets value of exchange setting
	std::string getExchangeSetting(const std::string& key);
	// Determines if the trading desk is open for trading
	bool isTradingDeskOpen();

	///
	/// ORDER MANAGEMENT
	/// 

	// Cancels all of the open orders for this instrument
	void cancelOrders(std::shared_ptr<Instrument> instrument);
	// Use this method to cancel one or more existing orders.
	void cancelOrders(const std::vector<Order> orders);
	// Creates a new 'Market' order.
	std::shared_ptr<Order> createMarketOrder(std::shared_ptr<Instrument> instrument, enums::OrderAction action, double qty);
	// Creates a new 'Market' order with reference id.
	std::shared_ptr<Order> createMarketOrder(std::shared_ptr<Instrument> instrument, const std::string ref_id, enums::OrderAction action, double qty);
	// Creates a new 'Stop' order.
	std::shared_ptr<Order> createStopOrder(std::shared_ptr<Instrument> instrument, enums::OrderAction action, enums::TIF tif, double qty, double stop_price);
	// Creates a new 'Stop' order with reference id.
	std::shared_ptr<Order> createStopOrder(std::shared_ptr<Instrument> instrument, const std::string ref_id, enums::OrderAction action, enums::TIF tif, double qty, double stop_price);
	// Creates a new 'Limit' order.
	std::shared_ptr<Order> createLimitOrder(std::shared_ptr<Instrument> instrument, enums::OrderAction action, enums::TIF tif, double qty, double limit_price);
	// Creates a new 'Limit' order with reference id.
	std::shared_ptr<Order> createLimitOrder(std::shared_ptr<Instrument> instrument, const std::string ref_id, enums::OrderAction action, enums::TIF tif, double qty, double limit_price);
	// Submit orders
	void submitOrders(const std::vector<Order> orders);
	// Gets the list of active orders
	std::vector< std::shared_ptr<Order> > getActiveOrders();
	// Gets the list of active executions
	std::vector< std::shared_ptr<Execution> > getActiveExecutions();

	///
	/// SIGNALS
	/// 
	nod::signal<void(std::shared_ptr<Instrument> instrument)> onTick;
	nod::signal<void()> onConnected;
	nod::signal<void()> onDisconnected;
	nod::signal<void()> onReady;

	/// 
	/// SLOTS
	/// 
	void slotExchangeInstrumentList(const std::vector<Instrument> instruments);
	void slotExchangeSettings(const ExchangeSettingsMap settings);
	void slotExchangeReady();
	void slotExchangeConnected();
	void slotExchangeDisconnected();
	void slotExchangeLogon(const std::string session_name);
	void slotExchangeLogout(const std::string session_name);
	void slotExchangeSessionCreated(const std::string session_name);
	void slotExchangeWarning(const std::string msg);
	void slotExchangeError(const std::string msg);
	void slotExchangeTradingDeskChange(const bool open);
	void slotExchangeAccountID(const std::string accountid);
	void slotExchangeBalanceChanged(const std::string accountid, const double balance);
	void slotExchangeCollateralSettings(const ExchangeCollateralSettingsMap map);
	void slotExchangePositionReport(const ExchangePositionReport report);
	void slotExchangeMarketDataReject(const std::string reason);
	void slotExchangeTick(const ExchangeTick tick);

private:
	// is the exchange connected?
	bool m_is_connected;
	// is the tradign desk open for trading?
	bool m_trading_desk_open;
	// stores all available instruments
	std::vector< std::shared_ptr<Instrument> > m_instrument_list;
	// the mutex
	std::mutex m_mutex;
	// stores pointer to network adapter
	std::unique_ptr<NetworkAdapter> m_network_ptr;
	// stores exchange settings
	UnorderedStrMap m_exchange_settings;
	// stores data context pointer
	std::shared_ptr<DataContext> m_datacontext_ptr;
	// stores orders
	std::vector< std::shared_ptr<Order> > m_order_list;
	// stores executed orders
	std::vector< std::shared_ptr<Execution> > m_execution_list;

	// add instrument
	void addInstrument(std::shared_ptr<Instrument> instrument);
	void addInstrument(const std::string& symbol);
	// connect signals to slots
	void connectNetworkSlots();
};
};