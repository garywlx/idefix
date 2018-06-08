#ifndef IDEFIX_EURUSD_H
#define IDEFIX_EURUSD_H
#include "../datatypes.h"

namespace idefix {	
	namespace assets {
		class EurUsd: public asset_struct {
		public:
			EurUsd(){
				name = "EUR/USD";
				contract_size = 100000;
				decimal_places = 5;
				market_open_dt = "00:00";
				market_close_dt = "23:59";
			}
		};
	}
}

#endif // - IDEFIX_EURUSD_H