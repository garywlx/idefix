#include "globals.h"

namespace idefix {

	/*!
	 * Global active symbol
	 */
	std::string g_symbol;

	/*!
	 * Global tick list
	 */
	std::map<std::string, idefix::tick_vect_t> g_ticklist;

	/*!
	 * Global bar list
	 */
	std::map<std::string, idefix::bar_vec_t> g_barlist;
	
};