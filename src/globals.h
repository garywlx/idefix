#ifndef GLOBALS_H
#define GLOBALS_H

#include <map>
#include "ide.h"

namespace idefix {
	extern std::string g_symbol;
	extern std::map<std::string, idefix::tick_deq_t> g_ticklist;
	extern std::map<std::string, idefix::bar_deq_t> g_barlist;
};

#endif