#ifndef IDEFIX_RENKOBRICK_H
#define IDEFIX_RENKOBRICK_H

#include <string>
#include <iomanip>
#include "Bar.h"

namespace IDEFIX {
	struct RenkoBrick: public Bar {
		enum STATUS {
			NOSTATUS = -1,
			LONG = 0,
			SHORT = 1
		};

		std::string symbol;
		std::string open_time;
		std::string close_time;
		double open_price;
		double close_price;
		double high_price;
		double low_price;
		STATUS status;
		int volume;
		double diff;
		double period;
		double point_size;

		inline void clear(){
			symbol      = "";
			open_time   = "";
			close_time  = "";
			open_price  = 0;
			close_price = 0;
			high_price  = 0;
			low_price   = 0;
			status      = STATUS::NOSTATUS;
			volume      = 0;
			diff        = 0;
			period      = 0;
			point_size  = 0;
		}
	};

	inline std::ostream& operator<<(std::ostream& out, const IDEFIX::RenkoBrick& brick) {
		std::string status = "NOSTATUS";
		bool is_long = true;
		if ( brick.status == RenkoBrick::STATUS::LONG ) {
			status = "L";
		} else if ( brick.status == RenkoBrick::STATUS::SHORT ) {
			status = "S";
			is_long = false;
		}

		out << "\033[" << ( brick.status == RenkoBrick::STATUS::SHORT ? "31" : "32" ) << "m";
		out << "Brick " << status << " " << brick.open_time << " ";
		out << std::setprecision(5) << std::fixed << brick.open_price << " ";
		out << std::setprecision(5) << std::fixed << brick.high_price << " ";
		out << std::setprecision(5) << std::fixed << brick.low_price << " ";
		out << std::setprecision(5) << std::fixed << brick.close_price << " ";
		out << brick.close_time << " ";
		out << brick.diff << " " << brick.volume;
		out << "\033[0m";
		return out;
	};

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