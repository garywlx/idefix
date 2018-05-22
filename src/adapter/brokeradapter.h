#ifndef IDEFIX_BROKERADAPTER_H
#define IDEFIX_BROKERADAPTER_H

#include <iostream>
#include "../datatypes.h"
#include "../datacenter.h"

/*!
 * Abstract Base Class
 */
namespace idefix {
	class BrokerAdapter {
	protected:
		Datacenter* m_datacenter_ptr;

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
		 * Datacenter
		 * @return [description]
		 */
		Datacenter* datacenter(){
			return m_datacenter_ptr;
		}
	};
};

#endif