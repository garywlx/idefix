#ifndef IDEFIX_SIGNAL_TYPE_H
#define IDEFIX_SIGNAL_TYPE_H

// Signal Types for nod signal identifier

namespace IDEFIX {
	enum SignalType {
	    MARKET_ORDER_SET_SL,
	    MARKET_ORDER_SET_TP,
	    MARKET_ORDER_CANCELED,
	    MARKET_ORDER_REJECT,
	    MARKET_ORDER_NEW,
	    MARKET_ORDER_TP_HIT,
	    MARKET_ORDER_SL_HIT
  	};
};

#endif