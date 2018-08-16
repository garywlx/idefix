#ifndef IDEFIX_RENKOBRICK_H
#define IDEFIX_RENKOBRICK_H

#include <string>
#include <iomanip>

namespace IDEFIX {
	struct RenkoBrick {
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
};

#endif