#ifndef BROKERADAPTER_H
#define BROKERADAPTER_H

#include <iostream>
#include "../ide.h"

/*!
 * Abstract Base Class
 */
namespace idefix {
	class BrokerAdapter {
	private:
		std::string m_symbol;
		tick_map_t m_tick_map;

	public:
		void init() {
			std::cout << "init not yet implemented." << std::endl;
		};
		void onInit() {
			std::cout << "onInit not yet implemented." << std::endl;
		};
		void onTick(const tick_struct& tick){
			std::cout << "onTick not yet implemented." << std::endl;
		};
		void onError(const std::string& error){
			std::cout << "onError not yet implemented." << std::endl;
		};
		void onBalance(){
			std::cout << "onBalance not yet implemented." << std::endl;
		};
		void onCandle(){
			std::cout << "onCandle not yet implemented." << std::endl;
		};
		void onExit(){
			std::cout << "onExit not yet implemented." << std::endl;
		}

		/*!
		 * Add tick to tick map
		 * @param const tick_struct
		 */
		void add_tick(const tick_struct tick){
			
		}
	};
};

#endif