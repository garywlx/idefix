/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 */

#include <iostream>
#include <thread>
#include <chrono>

#define IDEFIX_VERSION "1.0.1"

#include "FIXManager.h"

using namespace std;
using namespace IDEFIX;

void show_help(){
  cout << "Options: " << endl;
  cout << "  0=Exit" << endl;
  cout << "  1=Toggle SELL/BUY" << endl;
  cout << "  2=Open Position with SL" << endl;
  cout << "  3=Close all positions no matter what" << endl;
  //cout << "  3=Open Position with SL & TP" << endl;
  cout << "  4=Get Positions" << endl;
  cout << "  5=Account status" << endl;
  cout << "  6=Close position" << endl;
  cout << "  7=Close all positions" << endl;
  cout << "    > l=loosers" << endl;
  cout << "    > w=winners" << endl;
  cout << "    > a=all" << endl;
  cout << "  8=Toggle price output" << endl;
  cout << "  9=Show this help" << endl;
  cout << " 10=Show sys params" << endl;
  cout << " 11=Show available markets" << endl;
  cout << " 12=Show market detail" << endl;
  cout << " 13=Toggle PnL output" << endl;
  cout << " 14=Subscribe to Symbol" << endl;
  cout << " 15=Unsubscribe from Symbol" << endl;

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
    mo.setSide( FIX::Side_BUY );

    IDEFIX::MarketDetail marketDetail;

    string input_value;
    FIX::Side side( FIX::Side_BUY );

    // pips for stop loss || take profit
    int pips = 20;
    double qty = 100000;

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
        case 2: // Open Position with StopLoss
          cout << "--> open position with stop loss" << endl;
          ms = fixmanager.getLatestSnapshot( SUBSCRIBE_PAIR );

          if( ! ms.isValid() ) {
            cout << "MarketSnapshot is invalid: " << ms << endl;
            break;
          }

          marketDetail = fixmanager.getMarketDetails( SUBSCRIBE_PAIR );
          mo.setPrecision( marketDetail.getSymPrecision() );
          mo.setPointSize( marketDetail.getSymPointsize() );
          mo.setSide( side.getValue() );
          mo.setQty( qty );
          mo.setPrice( 0 ); // market order
          mo.setAccountID( fixmanager.getAccountID() );

          // SELL
          if ( side == FIX::Side_SELL ) {
            mo.setStopPrice( ms.getBid() + ( mo.getPointSize() * pips ) );
          } 
          // BUY
          else if ( side == FIX::Side_BUY ) {
            mo.setStopPrice( ms.getAsk() - ( mo.getPointSize() * pips ) );
          }

          fixmanager.marketOrder( mo, FIXFactory::SingleOrderType::MARKET_ORDER_SL );
          fixmanager.queryPositionReport();
          break;
        case 3: // Close all positions
          fixmanager.closeAllPositions( SUBSCRIBE_PAIR );
          fixmanager.queryAccounts();
          break;
        /*case 3: // Open Position with Stoploss and Take Profit
          cout << "--> query position with stop loss and take profit" << endl;
          ms = fixmanager.getLatestSnapshot( SUBSCRIBE_PAIR );

          if( ! ms.isValid() ){
            cout << "MarketSnapshot is invalid: " << ms << endl;
            break;
          }

          marketDetail = fixmanager.getMarketDetails( SUBSCRIBE_PAIR );
          mo.setPrecision( marketDetail.getSymPrecision() );
          mo.setPointSize( marketDetail.getSymPointsize() );
          mo.setSide( side.getValue() );
          mo.setQty( qty );
          mo.setPrice( 0 ); // market order
          mo.setAccountID( fixmanager.getAccountID() );

          // SELL
          if( side == FIX::Side_SELL ) {
            mo.setStopPrice(ms.getBid() + (mo.getPointSize() * pips));
            mo.setTakePrice(ms.getBid() - (mo.getPointSize() * pips));
          } 
          // BUY
          else {
            mo.setStopPrice(ms.getAsk() - (mo.getPointSize() * pips));
            mo.setTakePrice(ms.getAsk() + (mo.getPointSize() * pips));
          }
          
          fixmanager.marketOrder( mo, FIXFactory::SingleOrderType::MARKET_ORDER_SL_TP );
          fixmanager.queryPositionReport();
          break;*/
        case 4: // Get positions
          cout << "--> query positions" << endl;
          // output array
          fixmanager.debug();
          break;
        case 5: // Account/Collateral Report
          cout << "--> Get account status" << endl;
          fixmanager.queryAccounts();
          break;
        case 6:
          cout << "--> Position ID to close: ";
          input_value.clear();
          cin >> input_value;
          if( ! input_value.empty() ) {
            mo.setSide( side.getValue() );
            mo.setPosID(input_value);
            mo.setSymbol( SUBSCRIBE_PAIR );
            mo.setQty( qty );
            mo.setAccountID( fixmanager.getAccountID() );
            fixmanager.closePosition( mo );
          }
          break;
        case 7:
          cout << "--> Close all positions" << endl;
          cout << " > a=all" << endl;
          cout << " > w=all winners" << endl;
          cout << " > l=all loosers" << endl;
          input_value.clear();
          cin >> input_value;

          if ( "a" == input_value ) {
            fixmanager.closeAllPositions( SUBSCRIBE_PAIR );  
          } else if ( "w" == input_value ) {
            fixmanager.closeWinners( SUBSCRIBE_PAIR );
          } else if ( "l" == input_value ) {
            fixmanager.closeLoosers( SUBSCRIBE_PAIR );
          } else {
            cout << "-- unknown option --" << endl;
          }
          break;
        case 8:
          cout << "--> Toggle Price Output" << endl;
          fixmanager.toggleSnapshotOutput();
          break;
        case 9:
          // show help
          show_help();
          break;
        case 10:
          // show sys param list
          fixmanager.showSysParamList();
          break;
        case 11:
          // show available market list
          fixmanager.showAvailableMarketList();
          break;
        case 12:
          // show market detail
          cout << "--> Show market detail for: ";
          input_value.clear();
          cin >> input_value;
          if ( ! input_value.empty() ) {
            fixmanager.showMarketDetail( input_value );  
          }
          break;
        case 13:
          // toggle profit loss output
          cout << "--> Toggle PnL output" << endl;
          fixmanager.togglePNLOutput();
          break;
        case 14:
          // subscribe to symbol
          cout << "--> Subscribe to: ";
          input_value.clear();
          cin >> input_value;

          if( input_value.empty() ) {
            cout << "-- no symbol found --" << endl;
            break;
          }

          fixmanager.subscribeMarketData( input_value );
          break;
        case 15:
          // unsubscribe from symbol
          cout << "--> Unsubscribe from: ";
          input_value.clear();
          cin >> input_value;

          if( input_value.empty() ) {
            cout << "-- no symbol found--" << endl;
            break;
          }

          fixmanager.unsubscribeMarketData( input_value );
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

