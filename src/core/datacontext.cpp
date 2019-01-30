#include "datacontext.h"
#include "utility.h"
#include "logger.h"
#include <functional>

namespace idefix {
DataContext::DataContext(std::unique_ptr<NetworkAdapter> network_adapter): m_network_ptr( std::move( network_adapter ) ), m_is_connected(false) {

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

// ----------------------------------------------------------------------------------------------
// SLOTS
// ----------------------------------------------------------------------------------------------
void DataContext::slotExchangeInstrumentList(const std::vector<Instrument> instruments) {
	for ( const auto& instrument : instruments ) {
		addInstrument( instrument.getSymbol() );
	}
}

void DataContext::slotExchangeSettings(const ExchangeSettingsMap settings) {
	for ( const auto& setting : settings ) {
		SPDLOG_INFO("onExchangeSettings {} = {}", setting.first, setting.second );
	}
}

void DataContext::slotExchangeReady() {
	SPDLOG_INFO("Registered instruments: {:d}", m_instrument_list.size() );
	SPDLOG_INFO("Exchange ready.");

	// emit signal
	onReady();
}

void DataContext::slotExchangeConnected() {
	m_is_connected = true;

	SPDLOG_INFO("Exchange connected.");

	// emit signal
	onConnected();
}

void DataContext::slotExchangeDisconnected() {
	m_is_connected = false;

	SPDLOG_INFO("Exchange disconnected.");

	// emit signal
	onDisconnected();
}

void DataContext::slotExchangeLogon(const std::string session_name) {
	if ( session_name == "order" ) {
		SPDLOG_INFO("OrderSession login.");
	}
	if ( session_name == "market" ) {
		SPDLOG_INFO("MarketSession login.");
	}
}

void DataContext::slotExchangeLogout(const std::string session_name) {
	if ( session_name == "order" ) {
		SPDLOG_INFO("OrderSession logout.");
	}
	if ( session_name == "market" ) {
		SPDLOG_INFO("MarketSession logout.");
	}
}

void DataContext::slotExchangeSessionCreated(const std::string session_name) {
	SPDLOG_INFO("Exchange Session {} created.", session_name );
}

void DataContext::slotExchangeWarning(const std::string msg) {
	SPDLOG_WARN( "Exchange: {}", msg );
}

void DataContext::slotExchangeError(const std::string msg) {
	SPDLOG_ERROR( "Exchange: {}", msg );
}

void DataContext::slotExchangeTradingDeskChange(const bool open) {
	if ( open ) {
		SPDLOG_INFO("Trading Desk is open.");
	} else {
		SPDLOG_INFO("Trading Desk is closed.");
	}
}

void DataContext::slotExchangeAccountID(const std::string accountid) {
	SPDLOG_INFO("AccountID: {}", accountid );
}

void DataContext::slotExchangeBalanceChanged(const std::string accountid, const double balance) {
	SPDLOG_INFO("Account: {} Balance: {:.2f}", accountid, balance );
}

void DataContext::slotExchangeCollateralSettings(const ExchangeCollateralSettingsMap map) {
	SPDLOG_INFO("AccountSettings:");
	for ( auto const &setting : map ) {
		SPDLOG_INFO("{} = {}", setting.first, setting.second);
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

}

bool DataContext::isConnected() {
	return m_is_connected;
}

};