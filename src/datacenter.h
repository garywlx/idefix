#ifndef IDEFIX_DATACENTER_H
#define IDEFIX_DATACENTER_H

#include <iostream>
#include "datatypes.h"

namespace idefix {
	class Datacenter {
	private:
		asset_struct m_asset;
		tick_map_t m_ticklist;
		bar_map_t m_barlist;

	public:
		/*!
		 * Constructor
		 */
		Datacenter(asset_struct asset);

		/*!
		 * Deconstructor
		 */
		~Datacenter();

		/*!
		 * Set current asset
		 * @param asset_struct asset 
		 */
		void set_asset(const asset_struct asset) noexcept;

		/*!
		 * Get current active asset
		 * @return asset_struct
		 */
		asset_struct asset() noexcept;

		/*!
		 * Get current active symbol
		 * @return std::string
		 */
		std::string symbol() noexcept;

		/*!
		 * Get bar open value of position pos
		 * @param int pos current bar is 0, previous bar is 1...
		 * @return double 
		 */
		double open(const int pos) noexcept;

		/*!
		 * Get bar high value of position pos
		 * @param int pos current bar is 0, previous bar is 1...
		 * @return double
		 */
		double high(const int pos) noexcept;

		/*!
		 * Get bar low value of position pos
		 * @param int pos current bar is 0, previous bar is 1...
		 * @return double
		 */
		double low(const int pos) noexcept;

		/*!
		 * Get bar close value of position pos
		 * @param int pos current bar is 0, previous bar is 1...
		 * @return double
		 */
		double close(const int pos) noexcept;

		/*!
		 * Get current ask price
		 * @return double
		 */
		double ask() noexcept;

		/*!
		 * Get current bid price
		 * @return double
		 */
		double bid() noexcept;

		/*!
		 * Get current spread
		 * @return double
		 */
		double spread() noexcept;

		/*!
		 * Get current balance
		 * @return double
		 */
		double balance() noexcept;

		/*!
		 * Get tick value of position pos
		 * @param int	pos current tick is 0, previous tick is 1...
		 * @return tick_struct
		 */
		tick_struct tick(const int pos);

		/*!
		 * Get tick list size of current symbol
		 * @return unsigned int
		 */
		unsigned int ticks();

		/*!
		 * Get bar value of position pos
		 * @param  int	pos current bar is 0, previous bar is 1...
		 * @return bar_struct
		 */
		bar_struct bar(const int pos);

		/*!
		 * Get bar list size of current symbol
		 * @return unsigned int
		 */
		unsigned int bars();

		/*!
		 * Add new tick to global tick list
		 * @param tick_struct tick
		 */
		 void add_tick(idefix::tick_struct tick);

		 /*!
		  * Add new bar to global bar list
		  * @param bar_struct bar
		  */
		 void add_bar(idefix::bar_struct bar);

		 /*!
		  * Check if list contains the symbol
		  */
		 template <typename LIST_TYPE> 
		 bool hasSymbol(LIST_TYPE list, const std::string symbol);

		 void debug_bar_list();
		 void debug_tick_list();
	};	
};

#endif