#ifndef IDEFIX_TIMEMACHINE_H
#define IDEFIX_TIMEMACHINE_H

#include <iostream>
#include <chrono>
#include <date/date.h>

namespace idefix {
	namespace convert {
		/*!
		 * Convert datetime string to time_point
		 * @param const std::string datetime_str 
		 * @return std::chrono::system_clock::time_point
		 */
		inline std::chrono::system_clock::time_point to_time_point(const std::string& datetime_str){
			std::istringstream in(datetime_str);
			date::sys_seconds tp;
			in >> date::parse("%D %T", tp);
			if( in.fail() ){
				in.clear();
				in.str(datetime_str);
				in >> date::parse("%FT%T%z", tp);
			}

			return tp;
		}

		/*!
		* Convert seconds to milliseconds
		*
		* @param const std::chrono::seconds seconds
		* @return std::chrono::milliseconds
		*/
		inline std::chrono::milliseconds to_milliseconds(const std::chrono::seconds seconds){
			return std::chrono::duration_cast<std::chrono::milliseconds>(seconds);
		}

		/*!
		 * Convert timepoint to string date format %F %T
		 * 
		 * @param  std::chrono::system_close::time_point tp
		 * @return std::string
		 */
		inline std::string to_string(const std::chrono::system_clock::time_point tp){
			return date::format("%F %T", tp);
		}
	}; // - ns convert

	namespace times {
		/*!
		* Returns the current date time stamp a string
		* 
		* @return std::string
		*/
		inline std::string timestamp_str(){
			return date::format("%F %T", std::chrono::system_clock::now());;
		}	
	}; // - ns times
}; // - ns idefix

#endif