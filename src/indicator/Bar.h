#ifndef IDEFIX_BAR_H
#define IDEFIX_BAR_H

#include <iomanip>

namespace IDEFIX {
	struct Bar {
		friend inline std::ostream& operator<<(std::ostream& out, const IDEFIX::Bar b) {
			return out;
		}
	};
};

#endif