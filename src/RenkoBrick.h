#ifndef IDEFIX_RENKOBRICK_H
#define IDEFIX_RENKOBRICK_H

#include <string>
#include <iomanip>
#include "Bar.h"

namespace IDEFIX {
	struct RenkoBrick: public Bar {};

	inline bool operator==(const RenkoBrick& l, const RenkoBrick& r) {
		if ( l.symbol != r.symbol ) return false;
		if ( l.open_time != r.open_time ) return false;
		if ( l.close_time != r.close_time ) return false;
		if ( l.open_price != r.open_price ) return false;
		if ( l.close_price != r.close_price ) return false;
		if ( l.high_price != r.high_price ) return false;
		if ( l.low_price != r.low_price ) return false;
		if ( l.status != r.status ) return false;
		if ( l.volume != r.volume ) return false;
		if ( l.diff != r.diff ) return false;
		if ( l.period != r.period ) return false;
		if ( l.point_size != r.point_size ) return false;

		return true;
	}

	inline bool operator!=(const RenkoBrick& l, const RenkoBrick& r) {
		return ! ( l == r );
	}
};

#endif