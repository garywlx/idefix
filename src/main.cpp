/*!
 * Copyright(c)2018, Arne Gockeln. All rights reserved.
 * https://arnegockeln.com
 */
#include <iostream>
#include <string>
#include "FIXManager.h"
#include "AwesomeStrategy.h"
#include <functional>
#include "MathHelper.h"
#include <sstream>
#include <cstdlib>
#include "CFGParser.h"
#include "StringHelper.h"

namespace IDEFIX {
	void connect(FIXManager& fixmanager, AwesomeStrategy& strategy);	
};

/*!
 * Check argument with option if option value exists.
 * If not show cerr message
 * 
 * @param const int         argc        
 * @param const int         i           
 * @param const std::string arg
 * @param const std::string failure_msg 
 * @return bool
 */
inline bool check_argument_option(const int argc, const int i, const std::string arg, const std::string failure_msg = " option requires one argument.") {
	if ( i + 1 < argc ) {
		return true;
	} 

	cerr << arg << failure_msg << endl;
	return false;
}

using namespace std;
using namespace IDEFIX;

int main(int argc, char** argv) {

#ifdef CMAKE_PROJECT_VERSION
	const std::string project_version = CMAKE_PROJECT_VERSION;
#else
	const std::string project_version = "not set";
#endif

	try {
		if ( argc < 2 ) {
			cout << "IDEFIX v" << project_version << endl;
			cout << "Usage:" << endl;
			cout << "   idefix <options> <configfile>" << endl;
			cout << "Options:" << endl;
			cout << "   -v       \t Show version." << endl;
			cout << "   -s file  \t Load strategy cfg file." << endl;
			cout << endl;
			
			return EXIT_SUCCESS;
		}

		// defaults
		// config file
		std::string config_file;
		std::string strategy_cfg_file;

		// parse arguments
		for ( int i = 0; i < argc; i++ ) {
			std::string arg = argv[ i ];

			// Version
			if ( arg == "-v" ) {
				cout << "Version " << project_version << endl;
				return EXIT_SUCCESS;
			} 
			// strategy config file
			else if ( arg == "-s" ) {
				if ( ! check_argument_option( argc, i, arg ) ) {
					return EXIT_FAILURE;
				}

				strategy_cfg_file = argv[ i + 1 ];
			}
			// Config File
			else {
				config_file = arg;
			}
		}

#ifdef CMAKE_USE_HTML_CHARTS
		// purge csv files in public_html folder
		std::system( "if [ \"$(ls -A public_html/*_bars.csv)\" ]; then rm public_html/*_bars.csv; fi" );
#endif
		
		// init fix manager
		FIXManager fixmanager;
		
		// Strategy Configuration
		AwesomeStrategyConfig strategy_config;

		// Parse strategy config file
		try {
			auto scfg = CFGParser( strategy_cfg_file );
			if ( ! scfg.has_error() ) {
				// overwrite strategy config
				strategy_config.max_short_pos = atoi( scfg.value( "max_short_pos" ).c_str() );
				strategy_config.max_long_pos  = atoi( scfg.value( "max_long_pos" ).c_str() );
				strategy_config.max_pip_risk  = atoi( scfg.value( "max_pip_risk" ).c_str() );
				strategy_config.max_risk      = atof( scfg.value( "max_risk" ).c_str() );
				strategy_config.max_qty       = atof( scfg.value( "max_qty" ).c_str() );
				strategy_config.max_spread    = atof( scfg.value( "max_spread" ).c_str() );
				strategy_config.renko_size    = atof( scfg.value( "renko_size" ).c_str() );
				strategy_config.sma_size      = atoi( scfg.value( "sma_size" ).c_str() );
				strategy_config.wait_bricks   = atoi( scfg.value( "wait_bricks" ).c_str() );
			} 
		} catch(...) {
			return EXIT_FAILURE;
		}

		// EUR/USD
		AwesomeStrategy eurusd( "EUR/USD", strategy_config );
		connect( fixmanager, eurusd );
		
		// GBP/USD
		AwesomeStrategy gbpusd( "GBP/USD", strategy_config );
		connect( fixmanager, gbpusd );

		// Add chart with strategy
		// AUD/USD
		// AwesomeStrategy audusd( "AUD/USD" );
		// connect( fixmanager, audusd );

		// // NZD/USD
		// AwesomeStrategy nzdusd( "NZD/USD" );
		// connect( fixmanager, nzdusd );

		// // USD/CAD
		// AwesomeStrategy usdcad( "USD/CAD" );
		// connect( fixmanager, usdcad );

		// // USD/CHF
		// AwesomeStrategy usdchf( "USD/CHF" );
		// connect( fixmanager, usdchf );

		// connect 
		fixmanager.connect( config_file );

		// start loop
		while ( ! fixmanager.isExiting() ) {
			int command = 0;
			cin >> command;

			if ( command == 0 && ! fixmanager.isExiting() ) {
				fixmanager.setExiting( true );
				fixmanager.disconnect();
				break;
			}
		}

	} catch(std::exception& e) {
		cout << "Damn: " << e.what() << endl;
		return EXIT_FAILURE;
	} catch(...) {
		cout << "Unknonwn exception!" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*!
 * Connect fixmanager signals to strategy slots
 * 
 * @param FIXManager& fixmanager
 * @param const AwesomeStrategy& strategy
 */
void IDEFIX::connect(FIXManager& fixmanager, AwesomeStrategy& strategy) {
	// FIXManager signals
	// On Init
	fixmanager.on_init.connect( [&]() { 
		FIX::Locker lock( fixmanager.m_mutex );

		fixmanager.subscribeMarketData( strategy.get_symbol() );
		fixmanager.closeAllPositions( strategy.get_symbol() );
		fixmanager.queryAccounts();

		strategy.on_init();		
	});

	// On Exit
	fixmanager.on_exit.connect( [&]() { 
		FIX::Locker lock( fixmanager.m_mutex );

		fixmanager.closeAllPositions( strategy.get_symbol() );
		fixmanager.queryAccounts();

		fixmanager.unsubscribeMarketData( strategy.get_symbol() ); 
		
		strategy.on_exit();
	});

	// On tick
	fixmanager.on_tick.connect( [&](const MarketSnapshot& tick){
		FIX::Locker lock( fixmanager.m_mutex );

		if ( fixmanager.isExiting() ) return;

		if ( tick.getSymbol() == strategy.get_symbol() ) {
			strategy.on_tick( tick );
		}
	});
	
	// on entry signal open new market order
	strategy.on_entry_signal.connect( [&](const MarketSide side) {
		FIX::Locker lock( fixmanager.m_mutex );

		// close all opposite trades in this symbol
		FIX::Side opposide( ( side == MarketSide::Side_SELL ? FIX::Side_BUY : FIX::Side_SELL ) );
		fixmanager.closeAllPositions( strategy.get_symbol(), opposide.getValue() );

		// open new trade
		double conversion_price = 0;
		double free_margin      = fixmanager.getAccount()->getFreeMargin();
		double pip_risk         = strategy.get_config()->max_pip_risk;
		double percent_risk     = strategy.get_config()->max_risk;

		// get latest data
		// latest market snapshot
		auto ms = fixmanager.getLatestSnapshot( strategy.get_symbol() );
		// latest market details
		auto md = fixmanager.getMarketDetails( strategy.get_symbol() );

		// create market order
		MarketOrder mo;
		// set account id
		mo.setAccountID( fixmanager.getAccountID() );
		// set symbol
		mo.setSymbol( strategy.get_symbol() );
		// set side
		FIX::Side fix_side( ( side == MarketSide::Side_SELL ? FIX::Side_SELL : FIX::Side_BUY ) );
		mo.setSide( fix_side.getValue() );
		// set qty
		mo.setQty( Math::get_unit_size( free_margin, percent_risk, pip_risk, conversion_price ) );
		
		cout << "[on_entry_signal] " << mo.getSymbol() << " qty " << mo.getQty() << endl;

		//fixmanager.console()->info("[on_entry_signal] {} {:d} qty", mo.getSymbol(), mo.getQty() );

		if ( mo.getQty() > strategy.get_config()->max_qty ) {
			mo.setQty( strategy.get_config()->max_qty );
		}

		// fixmanager.console()->info("[on_entry_signal] {} {:d} qty", mo.getSymbol(), mo.getQty() );
		cout << "[on_entry_signal] " << mo.getSymbol() << " qty " << mo.getQty() << endl;

		// set entry price
		mo.setPrice( 0 ); // MarketOrder
		// set market detail precision and pointsize
	    mo.setPrecision( md->getSymPrecision() );
	    mo.setPointSize( md->getSymPointsize() );
		// set stoploss
		// for sell
		if ( side == MarketSide::Side_SELL ) {
			mo.setStopPrice( ms->getBid() + ( mo.getPointSize() * pip_risk ) );
		} 
		// for buy
		else if ( side == MarketSide::Side_BUY ) {
			mo.setStopPrice( ms->getAsk() - ( mo.getPointSize() * pip_risk ) );
		}
		
		//fixmanager.console()->info("[on_entry_signal] Open Position in {} on {} with size {:d}", mo.getSymbol(), mo.getSideStr(), mo.getQty() );
		cout << "[on_entry_signal] Open Position in " << mo.getSymbol() << " on " << mo.getSideStr() << " with size " << mo.getQty() << endl;

		// fixmanager.marketOrder( mo, FIXFactory::SingleOrderType::MARKET_ORDER_SL );
		// fixmanager.marketOrder( mo );
		fixmanager.queryPositionReport();
	});

	// on close all signal close all positions for symbol
	strategy.on_close_all_signal.connect( [&](const std::string& symbol) {
		FIX::Locker lock( fixmanager.m_mutex );
		fixmanager.closeAllPositions( symbol );
		fixmanager.queryAccounts();
	});
	
}