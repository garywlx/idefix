#ifndef IDEFIX_CSVHANDLER_H
#define IDEFIX_CSVHANDLER_H

#include <string>
#include <fstream>
#include <ostream>

namespace IDEFIX {
	class CSVHandler {
	public:
		CSVHandler();
		~CSVHandler();

		void set_path(const std::string& path);
		void set_filename(const std::string& name);
		void add_line(const std::string& line, const bool add_endl = true);

	private:
		std::string m_path;
		std::string m_filename;
		std::ofstream m_file;
	};
};

#endif