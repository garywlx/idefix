#ifndef IDEFIX_OPERATORS_H
#define IDEFIX_OPERATORS_H

#include "RenkoBrick.h"
#include "Exceptions.h"

namespace IDEFIX {
	/*!
	 * Check if value is above the line value
	 * 
	 * @param const double  value      
	 * @param const double  line_value 
	 * @return bool if value > line_value
	 */
	bool above(const double value, const double line_value) {
		return value > line_value;
	}

	/*!
	 * Check if brick.open_price and brick.close_price are above line_value
	 * @param const RenkoBrick&  brick
	 * @param const double       line_value 
	 * @return bool if brick.open_price > line_value && brick.close_price > line_value
	 */
	bool above(const RenkoBrick& brick, const double line_value) {
		return ( brick.open_price > line_value && brick.close_price > line_value );
	}

	/*!
	 * Check if value is below the line value
	 * 
	 * @param const double  value      
	 * @param const double  line_value 
	 * @return bool if value < line_value
	 */
	bool below(const double value, const double line_value) {
		return value < line_value;
	}

	/*!
	 * Check if brick.open_price and brick.close_price are below line_value
	 * @param  brick      [description]
	 * @param  line_value [description]
	 * @return            [description]
	 */
	bool below(const RenkoBrick& brick, const double line_value) {
		return ( brick.open_price < line_value && brick.close_price < line_value );
	}

	/*!
	 * Check if lvalue equals rvalue
	 * @param const double  left_value 
	 * @param const double  right_value
	 * @return bool if rvalue == rvalue
	 */
	bool equals(const double lvalue, const double rvalue) {
		return lvalue == rvalue;
	}
};

#endif