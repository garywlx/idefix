#pragma once

#include <iostream>
#include <map>
#include "Exceptions.h"

namespace IDEFIX {
class CFGParser {
public:
	CFGParser(const std::string cfg_file) throw( IDEFIX::file_not_found, out_of_range );
	std::map<std::string, std::string> values();
	bool has_error();
	std::string value(const std::string& key);

private:
	bool m_has_error;
	std::map<std::string, std::string> m_values;
};
};