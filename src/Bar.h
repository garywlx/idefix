#ifndef IDEFIX_BAR_H
#define IDEFIX_BAR_H

#include <iomanip>
#include <string>

namespace IDEFIX {
	struct Bar {
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

	inline std::ostream& operator<<(std::ostream& out, const IDEFIX::Bar& brick) {
		std::string status = "NOSTATUS";
		bool is_long = true;
		if ( brick.status == Bar::STATUS::LONG ) {
			status = "L";
		} else if ( brick.status == Bar::STATUS::SHORT ) {
			status = "S";
			is_long = false;
		}

		//out << "\033[" << ( brick.status == Bar::STATUS::SHORT ? "31" : "32" ) << "m";
		out << "Brick " << status << " " << brick.open_time << " ";
		out << std::setprecision(5) << std::fixed; 
		out << brick.open_price << " ";
		out << std::setprecision(5) << std::fixed; 
		out << brick.high_price << " ";
		out << std::setprecision(5) << std::fixed; 
		out << brick.low_price << " ";
		out << std::setprecision(5) << std::fixed; 
		out << brick.close_price << " ";
		out << brick.close_time << " ";
		out << brick.diff << " " << brick.volume;
		//out << "\033[0m";
		return out;
	};
};

#endif
