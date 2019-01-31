#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

#include <nod/nod.hpp>

#include "enums.h"
#include "exchangetypes.h"
#include "instrument.h"
#include "order.h"
#include "execution.h"

namespace idefix { 
class Adapter {
public:
	virtual ~Adapter() = default;

	typedef Adapter* (*CFunc)();
	typedef std::function<Adapter*()> Func;

	Adapter* clone() {
		auto inst = m_create_func();
		inst->setConfigFile( getConfigFile() );
		return inst;
	}

	static Adapter* load(const std::string& sofile);

	void setConfigFile(const std::string& filepath) {
		m_config_file = filepath;
	}

	std::string getConfigFile() {
		return m_config_file;
	}

protected:
	Func m_create_func;
	std::string m_config_file;
};

/**
 * Base Class for NetworkAdapter
 */
class NetworkAdapter: public Adapter {
public:
	virtual void connect() noexcept {}
	virtual void disconnect() noexcept {}
	virtual void subscribeMarketData(const std::string& symbol) {}
	virtual void unsubscribeMarketData(const std::string& symbol) {}

	// if a new instrument list is available
	nod::signal<void(const std::vector<Instrument> instruments)> onExchangeInstrumentList;
	// if a new list of exchange system parameters arrives
	nod::signal<void(const ExchangeSettingsMap settings)> onExchangeSettings;
	// if the exchange is ready
	nod::signal<void()> onExchangeReady;
	// if the exchange is connected
	nod::signal<void()> onExchangeConnected;
	// if the exchange is disconnected
	nod::signal<void()> onExchangeDisconnected;
	// if the exchange session is created
	nod::signal<void(const std::string msg)> onExchangeSessionCreated;
	// if the exchange is logged in
	nod::signal<void(const std::string msg)> onExchangeLogon;
	// if the exchange is logged out
	nod::signal<void(const std::string msg)> onExchangeLogout;
	// if the exchange sends a warning message
	nod::signal<void(const std::string msg)> onExchangeWarning;
	// if the exchange sends an error message
	nod::signal<void(const std::string msg)> onExchangeError;
	// if the exchange trading desk status is changed
	nod::signal<void(const bool open)> onExchangeTradingDeskChange;
	// if the exchange account is changed
	nod::signal<void(const std::string accoundid)> onExchangeAccountID;
	// if the exchange account balance is changed
	nod::signal<void(const std::string accountid, const double balance)> onExchangeBalanceChange;
	// if the exchange account collateral parties settings
	nod::signal<void(const ExchangeCollateralSettingsMap map)> onExchangeCollateralSettings;
	// if the exchange responds with a detailed position report
	nod::signal<void(const ExchangePositionReport report)> onExchangePositionReport;
	// if the exchange responds with marketDataRequestReject
	nod::signal<void(const std::string reason)> onExchangeMarketDataReject;
	// if the exchange responds with a MarketDataSnapshot
	nod::signal<void(const ExchangeTick tick)> onExchangeTick;
	// if the exchange responds with an ExecutionReport
	//nod::signal<void(const enums::ExchangeOrderEvent event, const Order order)> onExchangeOrder;
	// if the exchange responds with a new order
	nod::signal<void(std::shared_ptr<Order> order)> onExchangeOrderNew;
	// if the exchange responds with a filled order
	nod::signal<void(std::shared_ptr<Execution> execution)> onExchangeOrderFilled;
	// if the exchange responds with a cancelled order
	nod::signal<void(std::shared_ptr<Order> order)> onExchangeOrderCancelled;
	// if the exchange responds with a rejected order
	nod::signal<void(std::shared_ptr<Order> order)> onExchangeOrderRejected;
};

};