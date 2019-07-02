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
	 * blocking
	 */
	void MainApplication::start() {
		// start thread with web api context
		if ( m_webcontext_ptr != nullptr ) {
			m_api_thread = std::thread( &WebContext::start, m_webcontext_ptr.get() );
		}

		if ( m_datacontext_ptr != nullptr ) {
			// connect signal/slots
			mapDataContextSignalCallbacks();

			// connect to exchange
			m_datacontext_ptr->connect();
		}

		// block thread
		while( true ) {
			SPDLOG_INFO( "MENU" );
			SPDLOG_INFO( " 0) Quit");
			SPDLOG_INFO( " 1) Trade Market");
			SPDLOG_INFO( " 2) Trade Market with SL");
			SPDLOG_INFO( " 3) Trade Market with SL and TP");
			SPDLOG_INFO( " 4) Close Trade");
			SPDLOG_INFO( " 5) Query Order Status");
			SPDLOG_INFO( " 6) Query All Order Status");
			SPDLOG_INFO( " 7) Subscribe MarketData");
			SPDLOG_INFO( " 8) Unsubscribe MarketData");
			SPDLOG_INFO( " 9) Query Position Reports" );
			SPDLOG_INFO( "10) Query Trade Reports" );

			std::string cmd;
			std::cin >> cmd;

			if ( cmd == "0" ) {
				break;
			} else if ( cmd == "1" ) {
				SPDLOG_INFO( "Trade market.");
				auto instrument = m_datacontext_ptr->getInstrument("EUR/USD");
				if ( instrument == nullptr ) continue;

				double qty = 10000;
				SPDLOG_INFO( "Trade EUR/USD Market BUY QTY {:.1f}", qty );

				m_datacontext_ptr->createOrder( "EUR/USD", enums::OrderAction::BUY, qty );

			} else if ( cmd == "2" ) {
				SPDLOG_INFO( "Trade Market with SL");

				auto instrument = m_datacontext_ptr->getInstrument("EUR/USD");
				if ( instrument == nullptr ) continue;

				double qty = 10000;
				double sl  = instrument->getBidPrice() - ( instrument->getPointSize() * 10 );

				SPDLOG_INFO( "Trade EUR/USD Market with SL BUY QTY {:.1f} SL {}", qty, instrument->format( sl ) );

				m_datacontext_ptr->createOrder( "EUR/USD", enums::OrderAction::BUY, qty, 0, sl );

			} else if ( cmd == "3" ) {
				SPDLOG_INFO( "Trade Market with SL and TP");
				auto instrument = m_datacontext_ptr->getInstrument("EUR/USD");
				if ( instrument == nullptr ) continue;

				SPDLOG_INFO( "Instrument EUR/USD found." );

				double qty = 10000;
				double ask = instrument->getAskPrice();
				double bid = instrument->getBidPrice();
				double sl  = bid - ( instrument->getPointSize() * 10 );
				double tp  = ask + ( instrument->getPointSize() * 20 );   

				SPDLOG_INFO( "Trade EUR/USD BID {} ASK {} SL {} TP {} QTY {:.1f}", instrument->format( bid ), instrument->format( ask ), instrument->format( sl ), instrument->format( tp ), qty );

				m_datacontext_ptr->createOrder( "EUR/USD", enums::OrderAction::BUY, qty, 0, sl, tp );
			} else if ( cmd == "4" ) {
				std::string orderid;
				std::cout << "Please enter order id: ";
				std::cin >> orderid;

				if ( ! orderid.empty() ) {
					SPDLOG_INFO( "Close {}", orderid );
				}
			} else if ( cmd == "5" ) {
				SPDLOG_INFO( "Query Order Status" );

				std::string orderid;
				std::cout << "Please enter order id: ";
				std::cin >> orderid;

				auto account = m_datacontext_ptr->getAccounts()[0];

				if ( ! orderid.empty() ) {
					m_datacontext_ptr->queryOrderStatus( account->getAccountID(), orderid, "EUR/USD" );
				}
			} else if ( cmd == "6" ) {
				SPDLOG_INFO( "Query All Order Status" );

				auto account = m_datacontext_ptr->getAccounts()[0];
				m_datacontext_ptr->queryOrderStatus( account->getAccountID() );
			} else if ( cmd == "7" ) {
				SPDLOG_INFO( "Subscribe MarketData EUR/USD");

				m_datacontext_ptr->subscribe( "EUR/USD" );
			} else if ( cmd == "8" ) {
				SPDLOG_INFO( "Unsubscribe MarketData EUR/USD" );
				m_datacontext_ptr->unsubscribe( "EUR/USD" );
			} else if ( cmd == "9" ) {
				SPDLOG_INFO( "Query Position Reports" );
				auto account = m_datacontext_ptr->getAccounts()[0];

				m_datacontext_ptr->queryPositionReport( account->getAccountID(), enums::ExecutionType::POSITIONS );
			} else if ( cmd == "10" ) {
				SPDLOG_INFO( "Query Trade Reports" );
				auto account = m_datacontext_ptr->getAccounts()[0];

				m_datacontext_ptr->queryPositionReport( account->getAccountID(), enums::ExecutionType::TRADES );
			}
		}

		stop();
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

		m_datacontext_ptr->onReady.connect( std::bind( &MainApplication::slotExchangeReady, this ) );
		m_datacontext_ptr->onTick.connect( std::bind( &MainApplication::slotExchangeTick, this, std::placeholders::_1 ) );
		m_datacontext_ptr->onError.connect( [&](const std::string msg) {
			SPDLOG_ERROR( "{}", msg );
		});
		m_datacontext_ptr->onWarning.connect( [&](const std::string msg) {
			SPDLOG_WARN( "{}", msg );
		});
		m_datacontext_ptr->onSuccess.connect( [&](const std::string msg) {
			SPDLOG_INFO( "{}", msg );
		});
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
		// SPDLOG_INFO( "Tick {} Bid {} Ask {}", instrument->getSymbol(), instrument->format( instrument->getBidPrice() ), instrument->format( instrument->getAskPrice() ) );

		// update algos
		for ( auto& algo : m_algo_list ) {
			algo->onTick( *m_datacontext_ptr.get(), instrument );
		}
	}
};