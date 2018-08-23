#ifndef IDEFIX_EXCEPTIONS_H
#define IDEFIX_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace IDEFIX {

	struct idefix_exception: public std::logic_error {
		idefix_exception(const std::string& msg, const std::string& f, const int l): 
			std::logic_error(msg), file(f), line(l) {}
		~idefix_exception() throw() {}

		const std::string file;
		int line;
	};

	struct element_not_found: public idefix_exception {
		element_not_found(const std::string& file, const int line): 
			idefix_exception("Element not found.", file, line) {}
		~element_not_found() throw() {}
	};

	struct out_of_range: public idefix_exception {
		out_of_range(const std::string& file, const int line):
			idefix_exception("Out of range.", file, line) {}
		~out_of_range() throw() {}
	};
};

#endif