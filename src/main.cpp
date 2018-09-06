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

		// config file
		const std::string config_file = argv[1];

		// init fix manager
		FIXManager fixmanager;
		
		AwesomeStrategy test( "EUR/USD" );
		connect( fixmanager, test );

		// Add chart with strategy
		// AUD/USD
		// AwesomeStrategy audusd( "AUD/USD" );
		// connect( fixmanager, audusd );

		// EUR/USD		
		// AwesomeStrategy eurusd( "EUR/USD" );
		// connect( fixmanager, eurusd );
		
		// GBP/USD
		// AwesomeStrategy gbpusd( "GBP/USD" );
		// connect( fixmanager, gbpusd );

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
			} else if ( command == 1 && ! fixmanager.isExiting() ) {
				fixmanager.closeAllPositions( "GBP/USD" );
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
		fixmanager.unsubscribeMarketData( strategy.get_symbol() ); 
		strategy.on_exit();
	});

	// On tick
	fixmanager.on_tick.connect( [&](const MarketSnapshot& tick){
		FIX::Locker lock( fixmanager.m_mutex );

		if ( tick.getSymbol() == strategy.get_symbol() ) {
			strategy.on_tick( tick );
		}
	});
	
	// On Market Order Change
	// fixmanager.on_market_order.connect( [&](const SignalType type, const MarketOrder& mo) {
	// 	FIX::Locker lock( fixmanager.m_mutex );

	// 	if ( mo.getSymbol() == strategy.get_symbol() ) {
	// 		strategy.on_market_order( type, mo );
	// 	}
	// });

	// Strategy signals
	// on bar signal save bar values as json to a file for charting
	// strategy.on_bar_signal.connect( [&](const Bar& bar){
	// 	// parse dates
	// 	auto open_dt = FIX::UtcTimeStampConvertor::convert( bar.open_time );
	// 	std::stringstream open_ss;
	// 	open_ss << open_dt.getYear() << "-" 
	// 			 << setfill('0') << setw(2) << open_dt.getMonth() << "-" 
	// 			 << setfill('0') << setw(2) << open_dt.getDay() << " " 
	// 			 << setfill('0') << setw(2) << open_dt.getHour() << ":" 
	// 			 << setfill('0') << setw(2) << open_dt.getMinute() << ":" 
	// 			 << setfill('0') << setw(2) << open_dt.getSecond() << "." << open_dt.getMillisecond();

	// 	auto close_dt = FIX::UtcTimeStampConvertor::convert( bar.close_time );
	// 	std::stringstream close_ss;
	// 	close_ss << close_dt.getYear() << "-" 
	// 			 << setfill('0') << setw(2) << close_dt.getMonth() << "-" 
	// 			 << setfill('0') << setw(2) << close_dt.getDay() << " " 
	// 			 << setfill('0') << setw(2) << close_dt.getHour() << ":" 
	// 			 << setfill('0') << setw(2) << close_dt.getMinute() << ":" 
	// 			 << setfill('0') << setw(2) << close_dt.getSecond() << "." << close_dt.getMillisecond();

	// 	 // make filename for symbol bars
	// 	std::stringstream symbol_filename_ss;
	// 	symbol_filename_ss << strategy.get_symbol().c_str() << "_bars.csv";

	// 	std::string filename = symbol_filename_ss.str();
	// 	str::replace( filename, "/", "" );

	// 	std::ofstream barfile;
	// 	barfile.open( filename, ios::app | ios::out );
	// 	barfile << open_ss.str() << ",";
	// 	barfile << bar.open_price << ",";
	// 	barfile << bar.high_price << ",";
	// 	barfile << bar.low_price << ",";
	// 	barfile << bar.close_price << ",";
	// 	barfile << close_ss.str() << ",";
	// 	barfile << bar.volume << ",";
	// 	barfile << bar.point_size << endl;
	// 	barfile.close();

	// });

	// on entry signal open new market order
	strategy.on_entry_signal.connect( [&](const MarketSide side) {
		FIX::Locker lock( fixmanager.m_mutex );

		// close all opposite trades in this symbol
		FIX::Side opposide( ( side == MarketSide::Side_SELL ? FIX::Side_BUY : FIX::Side_SELL ) );
		fixmanager.closeAllPositions( strategy.get_symbol(), opposide.getValue() );

		// open new trade
		double conversion_price = 0;
		double free_margin = fixmanager.getAccount().getFreeMargin();

		MarketOrder mo;
		mo.setAccountID( fixmanager.getAccountID() );
		mo.setSymbol( strategy.get_symbol() );
		FIX::Side fix_side( ( side == MarketSide::Side_SELL ? FIX::Side_SELL : FIX::Side_BUY ) );
		mo.setSide( fix_side.getValue() );
		mo.setQty( Math::get_unit_size( free_margin, 1, 1, conversion_price ) );
		if ( mo.getQty() > strategy.get_max_qty() ) {
			mo.setQty( strategy.get_max_qty() );
		}
		mo.setPrice( 0 ); // MarketOrder

		// set market detail precision and pointsize
	    auto marketDetail = fixmanager.getMarketDetails( mo.getSymbol() );

	    mo.setPrecision( marketDetail.getSymPrecision() );
	    mo.setPointSize( marketDetail.getSymPointsize() );
		
		console()->info("[on_entry_signal] Side: {}", mo.getSideStr() );

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