/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */

#include <iostream>
#include <thread>
#include <chrono>

//#define SHOW_MARKETSNAPSHOT
#define IDEFIX_VERSION 1

#include "FIXManager.h"

using namespace std;
using namespace IDEFIX;

void show_help(){
    cout << "-------------------------------------------" << endl;
    cout << "Options: " << endl;
    cout << "  0=Exit" << endl;
    cout << "  1=Subscribe Marketdata" << endl;
    cout << "  2=Unsubscribe Marketdata" << endl;
    cout << "  3=Get Positions" << endl;
    cout << "  4=Send market order with stoploss" << endl;
    cout << "  5=Account status" << endl;
    cout << "  6=Manual Position Close" << endl;
    cout << "  7=Close all Positions" << endl;
    cout << "  8=Debug lists" << endl;
    cout << "  9=Show this help" << endl;
    cout << "-------------------------------------------" << endl;
}

int main(int argc, char** argv){

  try {

    if (argc != 2) {
      throw invalid_argument(std::string("Usage: ") + argv[0] + " cfgfile");
    }

    const string client_conf_file(argv[1]);

    if(client_conf_file.empty()){
      cout << "config file not found." << endl;
      return EXIT_FAILURE;
    }

    cout << "-------------- IDEFIX " << IDEFIX_VERSION << " -------------------" << endl;
    show_help();

    // init quickfix
    FIXManager fixmanager;
    // Start session and logon
    fixmanager.startSession(client_conf_file);

    IDEFIX::MarketSnapshot ms;
    IDEFIX::MarketOrder mo;
    string input_value;
    string buy_or_sell;

    while(true){
      int command = 0;
      bool exit = false;
      cin >> command;

      switch(command){
        case 0: // Exit fixmanagerlication
          cout << "--> Exiting..." << endl;
          exit = true;
          break;
        case 1: // Subscribe to MarketData
          cout << "--> Subscribe EUR/USD" << endl;
          fixmanager.subscribeMarketData(Symbol("EUR/USD"));
          break;
        case 2: // Unsubscribe from market data
          cout << "--> Unsubscribe EUR/USD" << endl;
          fixmanager.unsubscribeMarketData(Symbol("EUR/USD"));
          break;
        case 3: // Get positions
          cout << "--> query Positions +updates" << endl;
          fixmanager.queryPositionReport(PosReqType_POSITIONS);
          break;
        case 4: // Open Position with Stoploss
          cout << "--> query Position with Stoploss" << endl;
          ms = fixmanager.marketSnapshot(Symbol("EUR/USD"));

          // Buy Order
          mo = getNewMarketOrderStruct(Symbol("EUR/USD"));
          mo.qty = OrderQty(10000);
          mo.price = Price(ms.bid);
          mo.stopPrice = StopPx(ms.bid - 0.0020);
          mo.takePrice = Price(ms.bid + 0.0040);

          fixmanager.marketOrderWithStopLossTakeProfit(mo);
          fixmanager.queryPositionReport(PosReqType_POSITIONS);
          break;
        case 5: // Account/Collateral Report
          cout << "--> Get Account" << endl;
          fixmanager.queryAccounts();
          break;
        case 6:
          cout << "--> Position to close: ";
          cin >> input_value;
          cout << "BUY or SELL?: ";
          cin >> buy_or_sell;
          if( "" != input_value ) {
            fixmanager.queryClosePosition(input_value, Symbol("EUR/USD"), (buy_or_sell == "SELL" ? FIX::Side_SELL : FIX::Side_BUY), 10000);
          }
          break;
        case 7:
          cout << "--> Close all positions" << endl;
          fixmanager.closeAllPositions(Symbol("EUR/USD"));
          break;
        case 8:
          cout << "--> DEBUG" << endl;
          fixmanager.debug();
          break;
        case 9:
          // show help
          show_help();
          break;
      }

      if(exit){
        break;
      }
    }

    // End Session and logout
    fixmanager.endSession();

    // give another 10 seconds to clean up
    std::this_thread::sleep_for (std::chrono::seconds(1));

  } catch(exception& e){
    cout << e.what() << endl;
    return EXIT_FAILURE;
  } catch(...){
    cout << "Other exception." << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

