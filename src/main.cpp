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
    // cout << "  1=Subscribe Marketdata" << endl;
    // cout << "  2=Unsubscribe Marketdata" << endl;
    cout << "  3=Get Positions" << endl;
    cout << "  4=Send market order with SL & TP" << endl;
    cout << "  5=Account status" << endl;
    cout << "  6=Manual Position Close" << endl;
    cout << "  7=Close all Positions" << endl;
    cout << "  8=Toggle Price Output" << endl;
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

    // init FIXManager and start new session
    FIXManager fixmanager(client_conf_file);

    IDEFIX::MarketSnapshot ms;
    IDEFIX::MarketOrder mo( SUBSCRIBE_PAIR );
    mo.setSide(FIX::Side_BUY);

    IDEFIX::MarketDetail marketDetail;

    string input_value;
    FIX::Side side( FIX::Side_BUY );

    int pips = 20;

    while(true){
      int command = 0;
      bool exit = false;
      cin >> command;

      switch(command){
        case 0: // Exit fixmanagerlication
          exit = true;
          break;
        case 1:
          // toggle BUY/SELL positions to execute
          if( side == FIX::Side_BUY ) {
            side = FIX::Side_SELL;
            cout << "--> toggle Side: next order is SELL" << endl;
          } else {
            side = FIX::Side_BUY;
            cout << "--> toggle Side: next order is BUY" << endl;
          }
          break;
        // case 1: // Subscribe to MarketData
        //   cout << "--> Subscribe " << SUBSCRIBE_PAIR << endl;
        //   fixmanager.subscribeMarketData( SUBSCRIBE_PAIR );
        //   break;
        // case 2: // Unsubscribe from market data
        //   cout << "--> Unsubscribe " << SUBSCRIBE_PAIR << endl;
        //   fixmanager.unsubscribeMarketData( SUBSCRIBE_PAIR );
        //   break;
        case 3: // Get positions
          cout << "--> query Positions" << endl;
          //fixmanager.queryPositionReport(PosReqType_POSITIONS);
          /*cout << "--> query Trades" << endl;
          fixmanager.queryPositionReport(PosReqType_TRADES);
          cout << "--> query Assignments" << endl;
          fixmanager.queryPositionReport(PosReqType_ASSIGNMENTS);
          cout << "--> query Exercises" << endl;
          fixmanager.queryPositionReport(PosReqType_EXERCISES);*/
          // output array
          fixmanager.debug();
          break;
        case 4: // Open Position with Stoploss
          cout << "--> query Position with Stoploss" << endl;
          ms = fixmanager.getLatestSnapshot( SUBSCRIBE_PAIR );

          if( ! ms.isValid() ){
            cout << "MarketSnapshot is invalid: " << ms << endl;
            break;
          }

          marketDetail = fixmanager.getMarketDetails( SUBSCRIBE_PAIR );
          mo.setPrecision( marketDetail.getSymPrecision() );
          mo.setPointSize( marketDetail.getSymPointsize() );
          mo.setSide( side.getValue() );
          mo.setQty(10000);
          mo.setPrice( 0 ); // market order
          mo.setAccountID( fixmanager.getAccountID() );

          // SELL
          if( side == FIX::Side_SELL ) {
            mo.setStopPrice(ms.getBid() + (marketDetail.getSymPointsize() * pips));
            mo.setTakePrice(ms.getBid() - (marketDetail.getSymPointsize() * pips));
          } 
          // BUY
          else {
            mo.setStopPrice(ms.getAsk() - (marketDetail.getSymPointsize() * pips));
            mo.setTakePrice(ms.getAsk() + (marketDetail.getSymPointsize() * pips));
          }
          
          fixmanager.marketOrder( mo, FIXFactory::SingleOrderType::MARKET_ORDER_SL_TP );
          fixmanager.queryPositionReport(PosReqType_POSITIONS);
          break;
        case 5: // Account/Collateral Report
          cout << "--> Get Account" << endl;
          fixmanager.queryAccounts();
          break;
        case 6:
          cout << "--> Position to close: ";
          cin >> input_value;
          if( "" != input_value ) {
            mo.setSide( side.getValue() );
            mo.setPosID(input_value);
            mo.setSymbol( SUBSCRIBE_PAIR );
            mo.setQty(10000);
            mo.setAccountID( fixmanager.getAccountID() );
            fixmanager.closePosition(mo);
          }
          break;
        case 7:
          cout << "--> Close all positions" << endl;
          fixmanager.closeAllPositions( SUBSCRIBE_PAIR );
          break;
        case 8:
          cout << "--> Toggle Price Output" << endl;
          fixmanager.toggleSnapshotOutput();
          break;
        case 9:
          // show help
          show_help();
          break;
        default:
          cout << "-- unknown option --" << endl;
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

