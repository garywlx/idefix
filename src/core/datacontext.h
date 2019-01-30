#pragma once

#include <string>
#include <mutex>
#include <nod/nod.hpp>
#include <vector>
#include <memory>

#include "adapter.h"
#include "instrument.h"
#include "exchangetypes.h"

namespace idefix {
class DataContext {
public:
	DataContext(std::unique_ptr<NetworkAdapter> network_adapter);

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

	// signals
	nod::signal<void(std::shared_ptr<Instrument> instrument)> onTick;
	nod::signal<void()> onConnected;
	nod::signal<void()> onDisconnected;
	nod::signal<void()> onReady;

	// slots
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
	bool m_is_connected;
	std::vector< std::shared_ptr<Instrument> > m_instrument_list;
	std::mutex m_mutex;
	std::unique_ptr<NetworkAdapter> m_network_ptr;

	// add instrument
	void addInstrument(std::shared_ptr<Instrument> instrument);
	void addInstrument(const std::string& symbol);
	// connect signals to slots
	void connectNetworkSlots();
};
};