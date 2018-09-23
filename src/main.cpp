/*!
 * Copyright(c)2018, Arne Gockeln. All rights reserved.
 * https://arnegockeln.com
 */

// Semantic versioning. idefix version can be printed with IDEFIX_VERSION();
#define IDEFIX_VERSION_MAJOR 0
#define IDEFIX_VERSION_MINOR 1
#define IDEFIX_VERSION_PATCH 9

#define IDEFIX_VERSION() printf("%d.%d.%d", IDEFIX_VERSION_MAJOR, IDEFIX_VERSION_MINOR, IDEFIX_VERSION_PATCH)

#include <iostream>
#include <string>
#include "FIXManager.h"
#include "AwesomeStrategy.h"
#include <functional>
#include "Console.h"
#include "MathHelper.h"
#include <sstream>
#include <cstdlib>

namespace IDEFIX {
	void connect(FIXManager& fixmanager, AwesomeStrategy& strategy);	
};

using namespace std;
using namespace IDEFIX;

int main(int argc, char** argv) {
	try {
		if ( argc != 2 ) {
			cout << "IDEFIX "; IDEFIX_VERSION(); cout << endl;
			cout << "Usage:" << endl;
			cout << "   idefix <configfile>" << endl;
			cout << endl;
			
			return EXIT_SUCCESS;
		}

		// purge csv files in public_html folder
		std::system( "if [ \"$(ls -A public_html/*_bars.csv)\" ]; then rm public_html/*_bars.csv; fi" );

		// config file
		const std::string config_file = argv[1];

		// init fix manager
		FIXManager fixmanager;
		
		// Add chart with strategy
		// AUD/USD
		// AwesomeStrategy audusd( "AUD/USD" );
		// connect( fixmanager, audusd );

		// Strategy Configuration
		AwesomeStrategyConfig strategy_config;
		// how many parallel short positions are allowed?
		strategy_config.max_short_pos = 1;
		// how many parallel long positions are allowed?
		strategy_config.max_long_pos  = 1;
		// maximum risk per trade in percent
		strategy_config.max_risk      = 1;
		// maximum quantity size
		strategy_config.max_qty       = 1 * 100000;
		// maximum spread to open a position
		strategy_config.max_spread    = 1;
		// Renko brick size
		strategy_config.renko_size    = 5;
		// SMA size
		strategy_config.sma_size      = 5;
		// wait for at least 5 bricks before entering the markets
		strategy_config.wait_bricks   = strategy_config.sma_size + 1;

		// EUR/USD
		AwesomeStrategy eurusd( "EUR/USD", strategy_config );
		connect( fixmanager, eurusd );
		
		// GBP/USD
		AwesomeStrategy gbpusd( "GBP/USD", strategy_config );
		connect( fixmanager, gbpusd );

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
		cout << "something horrible went wrong: " << e.what() << endl;
		return EXIT_FAILURE;
	} catch(...) {
		cout << "Unknonwn exception" << endl;
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
		double pip_risk         = 30; // 10 = 1 pip 
		double percent_risk     = 1;

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
		if ( mo.getQty() > strategy.get_max_qty() ) {
			mo.setQty( strategy.get_max_qty() );
		}
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
		
		console()->info("[on_entry_signal] Open Position for Side: {}", mo.getSideStr() );

		fixmanager.marketOrder( mo, FIXFactory::SingleOrderType::MARKET_ORDER_SL );
		fixmanager.queryPositionReport();
	});

	// on close all signal close all positions for symbol
	strategy.on_close_all_signal.connect( [&](const std::string& symbol) {
		FIX::Locker lock( fixmanager.m_mutex );
		fixmanager.closeAllPositions( symbol );
		fixmanager.queryAccounts();
	});
	
}