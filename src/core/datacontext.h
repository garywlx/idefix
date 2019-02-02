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
#include "core/order.h"
#include "core/enums.h"
#include "core/account.h"

namespace idefix {
class DataContext {
public:
	DataContext(std::unique_ptr<NetworkAdapter> network_adapter);

	///
	/// DATA MANAGEMENT
	/// 
	
	// get account list
	std::vector< std::shared_ptr<Account> > getAccounts();
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

	// Send order to exchange
	// @param const std::string symbol
	// @param const enums::OrderAction action
	// @param const double qty
	// @param const double price = 0 == market | > 0 == pending order
	// @param const double sl = 0 == no sl | > 0 == stop order
	// @param const double tp = 0 == no tp | > 0 == limit order
	void createOrder(const std::string symbol, const enums::OrderAction action, const double qty, const double price = 0, const double sl = 0, const double tp = 0);
	// Cancels all of the open orders for this instrument
	void cancelOrders(std::shared_ptr<Instrument> instrument);
	// Use this method to cancel one or more existing orders.
	void cancelOrders(const std::vector<Order> orders);
	// Close order
	void closeOrder(const std::string& order_id);
	// request order status, leave orderid and symbol empty for status query for all orders on this accountid
	void queryOrderStatus(const std::string& accountid, const std::string& orderid = "", const std::string& symbol = "");
	// Gets the list of active orders
	std::vector< std::shared_ptr<Order> > getActiveOrders();

	///
	/// SIGNALS
	/// 
	nod::signal<void(std::shared_ptr<Instrument> instrument)> onTick;
	nod::signal<void()> onConnected;
	nod::signal<void()> onDisconnected;
	nod::signal<void()> onReady;
	nod::signal<void(std::shared_ptr<Order> order)> onOrderChange;
	nod::signal<void(const std::string msg)> onError;
	nod::signal<void(const std::string msg)> onSuccess;
	nod::signal<void(const std::string msg)> onWarning;

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
	void slotExchangeOrder(std::shared_ptr<Order> order);

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
	// stores accounts
	std::vector< std::shared_ptr<Account> > m_account_list;

	// add account
	void addAccount(std::shared_ptr<Account> account);
	// add instrument
	void addInstrument(std::shared_ptr<Instrument> instrument);
	void addInstrument(const std::string& symbol);
	// connect signals to slots
	void connectNetworkSlots();
	// remove order from order list
	bool removeOrder(const std::string& order_id);
};
};