/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */

#include <iostream>
#include <thread>
#include <chrono>

//#define SHOW_MARKETSNAPSHOT

#include "FIXManager.h"

using namespace std;

int main(int argc, char** argv){

  try {

    if (argc != 2) {
      throw invalid_argument(std::string("Usage: ") + argv[0] + " cfgfile");
    }

    const string client_conf_file(argv[1]);

    cout << "Options: " << endl;
    cout << "  0=Exit" << endl;
    cout << "  1=Subscribe Marketdata" << endl;
    cout << "  2=Unsubscribe Marketdata" << endl;
    cout << "  3=Get Positions" << endl;
    cout << "  4=Send market order" << endl;
    cout << "  5=Close all positions" << endl;
    cout << "  6=Account Status" << endl;
    cout << "  7=Trade Capture Report" << endl;

    // init quickfix
    FIXManager fixmanager;
    // Start session and logon
    fixmanager.startSession(client_conf_file);

    IDEFIX::MarketSnapshot ms;
    string input_value;

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
          cout << "--> Subscribe EUR/USD, USD/CHF" << endl;
          fixmanager.subscribeMarketData(Symbol("EUR/USD"));
          // fixmanager.subscribeMarketData(Symbol("USD/CHF"));
          break;
        case 2: // Unsubscribe from market data
          cout << "--> Unsubscribe EUR/USD, USD/CHF" << endl;
          fixmanager.unsubscribeMarketData(Symbol("EUR/USD"));
          // fixmanager.unsubscribeMarketData(Symbol("USD/CHF"));
          break;
        case 3: // Get positions
          cout << "--> query Positions" << endl;
          fixmanager.queryPositionReport(PosReqType_POSITIONS);
          break;
        case 4: // Query trades
          cout << "--> query Trades" << endl;
          fixmanager.queryPositionReport(PosReqType_TRADES);
          break;
        case 6: // Account/Collateral Report
          cout << "--> Get Account" << endl;
          fixmanager.queryAccounts();
          break;
        case 7:
          cout << "--> Position to close: ";
          cin >> input_value;
          if( "" != input_value ) {
            fixmanager.queryClosePosition(input_value, Symbol("EUR/USD"), FIX::Side_BUY, 10000);
          }
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

