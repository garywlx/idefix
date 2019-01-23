#pragma once

#include "logger.h"
#include "adapter.h"
#include "webcontext.h"

#include <thread>

namespace idefix {
class MainApplication {
public:

	MainApplication();
	~MainApplication();

	// set webcontext pointer
	void setWebContext(WebContext* ctx);
	// set adapter pointer
	void setAdapter(NetworkAdapter* adapter);

	// start idefix, initiate api and connect to exchange
	void start();
	// disconnect from exchange, stop service
	void stop();

private:
	// the WebContext
	WebContext* m_webcontext_ptr;
	// the Adapter
	NetworkAdapter* m_exchange_ptr;

	std::thread m_web_thread;
	std::thread m_exchange_thread;
};	
};

