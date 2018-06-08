#ifndef IDEFIX_DATACENTER_H
#define IDEFIX_DATACENTER_H

#include <iostream>
#include "datatypes.h"
#include <chrono>

namespace idefix {
	class Datacenter {
	private:
		// the current asset
		asset_struct m_asset;
		// the tick map 
		tick_map_t m_ticklist;
		// how many ticks in list?
		unsigned long m_tick_count;
		// the bar map
		bar_map_t m_barlist;
		// how many bars in list?
		unsigned long m_bar_count;
		// the account
		account_struct m_account;
		// last timepoint 
		std::chrono::steady_clock::time_point m_last_bar_tp;
		// lowest bar period in seconds
		unsigned int m_bar_period_seconds;

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
		void set(const asset_struct asset) noexcept;

		/*!
		 * Set current account
		 * @param account_struct
		 */
		void set(account_struct account) noexcept;

		/*!
		 * Set lowest bar period
		 * @param unsigned int period
		 */
		void set_lowest_bar_period(unsigned int period) noexcept;

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
		 * Get current equity
		 * @return double
		 */
		double equity() noexcept;

		/*!
		 * Get account information
		 * @return account_struct
		 */
		account_struct account();

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
		bool has_symbol(LIST_TYPE list, const std::string symbol);

		/*!
		 * Callback initializing 
		 */
		void on_init();

		/*!
		 * Callback for every new tick
		 * @param const tick_struct&
		 */
		void on_tick(const tick_struct& tick);

		/*!
		 * Callback if an error occours
		 * @param const std::string&
		 */
		void on_error(const std::string& error);

		/*!
		 * Callback if balance changes
		 */
		void on_balance();

		/*!
		 * Callback for every new candle
		 */
		void on_candle(const bar_struct& candle);

		/*!
		 * Callback when the datacenter exits
		 */
		void on_exit();

		/*!
		 * Returns true if the period for a new bar is over
		 * @return bool
		 */
		bool is_next_bar_time();

		/*!
		 * Returns the current date time stamp a string
		 * @return std::string
		 */
		std::string datetime();

		

		// DEBUG
		inline void debug_bar_list(){
			auto list = m_barlist[symbol()];
			auto it = list.begin();
			std::cout << "[BARLIST] " << symbol() << std::endl;
			while(it != list.end()){
				std::cout << *it << std::endl;
				it++;
			} 
		}

		inline void debug_tick_list(){
			auto list = m_ticklist[symbol()];
			auto it = list.begin();
			std::cout << "[TICKLIST] " << symbol() << std::endl;
			while(it != list.end()){
				std::cout << *it << std::endl;
				it++;
			} 
		}
	};	
};

#endif