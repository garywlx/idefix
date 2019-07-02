#pragma once

#include "logger.h"
#include "adapter.h"
#include "webcontext.h"
#include "datacontext.h"
#include "algo.h"

#include <thread>
#include <vector>
#include <memory>

namespace idefix {

typedef std::vector<Algo*> AlgoVector;

class MainApplication {
public:

	MainApplication();
	~MainApplication();

	// set datacontext pointer
	void setDataContext(std::unique_ptr<DataContext> ctx);
	// set webcontext pointer
	void setWebContext(std::unique_ptr<WebContext> ctx);
	// add algo pointer
	void addAlgo(Algo* algo);

	// start idefix, initiate api and connect to exchange
	void start();
	// disconnect from exchange, stop service
	void stop();
	// until the app is running return true
	bool isRunning();

	void slotExchangeReady();
	void slotExchangeTick(std::shared_ptr<Instrument> instrument);

private:
	// map data context signal callbacks
	void mapDataContextSignalCallbacks();

	// the WebContext
	std::unique_ptr<WebContext> m_webcontext_ptr;
	// the DataContext
	std::unique_ptr<DataContext> m_datacontext_ptr;
	// the algo container
	AlgoVector m_algo_list;

	// the thread for the websocket api
	std::thread m_api_thread;
	std::thread m_exchange_thread;
};
};

