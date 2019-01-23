#include "mainapplication.h"
#include <functional>

namespace idefix {

	MainApplication::MainApplication() {

	}

	MainApplication::~MainApplication() {
		stop();
	}

	/**
	 * Set WebContext pointer
	 * 
	 * @param WebContext* ctx
	 */
	void MainApplication::setWebContext(WebContext* ctx) {
		m_webcontext_ptr = ctx;
	}

	/**
	 * Set exchange adapter pointer
	 * 
	 * @param NetworkAdapter* adapter
	 */
	void MainApplication::setAdapter(NetworkAdapter* adapter) {
		m_exchange_ptr = adapter;
	}

	/**
	 * start MainApplication, initiate api and connect to exchange
	 */
	void MainApplication::start() {
		if ( m_webcontext_ptr != nullptr ) {

			m_web_thread = std::thread([this](){
				m_webcontext_ptr->start();
			});
		}

		if ( m_exchange_ptr != nullptr  ) {

			m_exchange_thread = std::thread( [this]() {
				m_exchange_ptr->connect();
			});
		}
	}

	/**
	 * disconnect from exchange, stop service
	 */
	void MainApplication::stop() {
		if ( m_exchange_ptr != nullptr ) {
			m_exchange_ptr->disconnect();
			m_exchange_thread.join();
		}

		if ( m_webcontext_ptr != nullptr ) {
			m_webcontext_ptr->stop();
			m_web_thread.join();
		}
	}

};