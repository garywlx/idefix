#include "mainapplication.h"
#include "instrument.h"
#include "order.h"

#include <functional>
#include <vector>

namespace idefix {

	MainApplication::MainApplication() {}
	MainApplication::~MainApplication() {}

	/**
	 * set datacontext pointer
	 * 
	 * @param std::unique_ptr<DataContext> ctx
	 */
	void MainApplication::setDataContext(std::unique_ptr<DataContext> ctx) {
		m_datacontext_ptr = std::move( ctx );
	}

	/**
	 * Set WebContext pointer
	 * 
	 * @param std::unique_ptr<WebContext> ctx
	 */
	void MainApplication::setWebContext(std::unique_ptr<WebContext> ctx) {
		m_webcontext_ptr = std::move( ctx );
	}

	/**
	 * Add algorithm to algo list
	 * 
	 * @param Algo* algo
	 */
	void MainApplication::addAlgo(Algo* algo) {
		if ( algo == nullptr ) {
			SPDLOG_ERROR("Algo not found.");
			return;
		}

		// check if algo is already in list
		auto found_algo = std::find_if( m_algo_list.begin(), m_algo_list.end(), [&algo](Algo* rhAlgo) {
			return ( algo->name() == rhAlgo->name() ); 
		});

		if ( found_algo == m_algo_list.end() ) {
			// add algo
			m_algo_list.push_back( algo );
		}
	}

	/**
	 * start MainApplication, initiate api and connect to exchange
	 */
	void MainApplication::start() {
		// start thread with web api context
		if ( m_webcontext_ptr != nullptr ) {
			m_api_thread = std::thread( &WebContext::start, m_webcontext_ptr.get() );
		}

		if ( m_datacontext_ptr != nullptr ) {
			// connect signal/slots
			m_datacontext_ptr->onConnected.connect( std::bind( &MainApplication::slotExchangeConnected, this ) );
			m_datacontext_ptr->onDisconnected.connect( std::bind( &MainApplication::slotExchangeDisconnected, this ) );
			m_datacontext_ptr->onReady.connect( std::bind( &MainApplication::slotExchangeReady, this ) );
			m_datacontext_ptr->onTick.connect( std::bind( &MainApplication::slotExchangeTick, this, std::placeholders::_1 ) );

			// connect to exchange
			m_datacontext_ptr->connect();
		}
	}

	/**
	 * disconnect from exchange, stop service
	 */
	void MainApplication::stop() {
		if ( m_webcontext_ptr != nullptr ) {
			// stop web context
			m_webcontext_ptr->stop();

			// join thread
			if ( m_api_thread.joinable() ) {
				m_api_thread.join();	
			}
		}

		if ( m_datacontext_ptr != nullptr ) {
			m_datacontext_ptr->disconnect();
		}
	}

	/**
	 * Check if the exchange and web thread are joinable
	 * 
	 * @return bool
	 */
	bool MainApplication::isRunning() {
		bool is_running = true;

		if ( m_datacontext_ptr == nullptr || m_datacontext_ptr->isConnected() ) is_running = false;
		if ( m_webcontext_ptr == nullptr ) is_running = false;

		return is_running;
	}

	/**
	 * map data context signal callbacks
	 */
	void MainApplication::mapDataContextSignalCallbacks() {
		if ( m_datacontext_ptr == nullptr ) {
			return;
		}
	}

	/**
	 * Slot Exchange Connected
	 */
	void MainApplication::slotExchangeConnected() {
		SPDLOG_INFO( "slotExchangeConnected" );
	}

	/**
	 * Slot Exchange Disconnected
	 */
	void MainApplication::slotExchangeDisconnected() {
		SPDLOG_INFO( "slotExchangeDisconnected" );
	}

	/**
	 * Slot Exchange Ready
	 */
	void MainApplication::slotExchangeReady() {
		// initialze algos
		for ( auto& algo : m_algo_list ) {
			algo->initialize( *m_datacontext_ptr.get() );
		}
	}

	/**
	 * Slot exchange Tick
	 * 
	 * @param std::shared_ptr<Instrument> instrument 
	 */
	void MainApplication::slotExchangeTick(std::shared_ptr<Instrument> instrument) {		
		// update algos
		for ( auto& algo : m_algo_list ) {
			algo->onTick( *m_datacontext_ptr.get(), instrument );
		}
	}
};