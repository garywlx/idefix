#ifndef IDEFIX_BROKERADAPTER_H
#define IDEFIX_BROKERADAPTER_H

#include <iostream>
#include "../datatypes.h"
#include "../datacenter.h"
#include "../times.h"

/*!
 * Abstract Base Class
 */
namespace idefix {
	class BrokerAdapter {
	protected:
		Datacenter* m_datacenter_ptr;

	public:
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