/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include "FIXApp.h"

using namespace std;

int main(int argc, char** argv){

  try {

    if (argc != 2) {
      throw invalid_argument(std::string("Usage: ") + argv[0] + " cfgfile");
    }

    const string client_conf_file(argv[1]);

    // init quickfix
    FIXApp app;
    // Start session and logon
    app.StartSession(client_conf_file);

    while(true){
      int command = 0;
      bool exit = false;
      cin >> command;

      switch(command){
        case 0: // Exit application
          exit = true;
          break;
        case 1: // Get positions
          app.GetPositions();
          break;
        case 2: // Subscribe to MarketData
          app.SubscribeMarketData();
          break;
        case 3: // Unsubscribe to market data
          app.UnsubscribeMarketData();
          break;
        case 4: // Send market order
          break;
      }

      if(exit){
        break;
      }
    }

    // End Session and logout
    app.EndSession();

    while(true){
      // Wait
    }

  } catch(exception& e){
    cout << e.what() << endl;
    return EXIT_FAILURE;
  } catch(...){
    cout << "Other exception." << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

