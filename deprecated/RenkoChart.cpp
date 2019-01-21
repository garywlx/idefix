#include "RenkoChart.h"
#include "MathHelper.h"
#include "StringHelper.h"
#include <stdexcept>

#ifdef CMAKE_SHOW_DEBUG_OUTPUT
#include "Console.h"
#include <sstream>
#endif

namespace IDEFIX {
	RenkoChart::RenkoChart(): m_period(5) {

	}

	RenkoChart::RenkoChart(const double period): m_period( period ) {
		m_init_brick.clear();
		m_last_brick.clear();
		m_current_brick.clear();
	}

	RenkoChart::~RenkoChart() {}

	/*!
	 * React to on_tick call
	 * 
	 * @param const MarktSnapshot&  tick
	 */
	void RenkoChart::on_tick(const MarketSnapshot& tick) {
		FIX::Locker lock( m_mutex );

		// is this the first brick?
		if ( m_bricks.size() == 0 ) {
			if ( init_brick( tick ) ) {
				return;
			}
		} 
		// get last brick
		else {
			m_last_brick = m_bricks.back();
		}

		// shortcuts
		auto tick_bid          = tick.getBid();
		auto tick_sending_time = tick.getSendingTime();
		auto tick_point_size   = tick.getPointSize();

		// renko calculation
		if ( m_current_brick.status == RenkoBrick::STATUS::NOSTATUS && m_current_brick.volume == 0 ) {
			m_current_brick.symbol     = tick.getSymbol();
			m_current_brick.period     = m_period;
			m_current_brick.open_time  = m_last_brick.close_time;
			m_current_brick.open_price = m_last_brick.close_price;
			m_current_brick.low_price  = m_last_brick.close_price;
			m_current_brick.volume     = 1;
			m_current_brick.point_size = tick_point_size;
		} 

		// increase volume
		m_current_brick.volume++;

#ifdef CMAKE_SHOW_DEBUG_OUTPUT
		// for console output
		std::stringstream ss_console;
#endif

		// make brick
		switch( m_last_brick.status ) {
			// last brick is LONG 
			case RenkoBrick::STATUS::LONG:
				// open new long brick
				if ( tick_bid > m_last_brick.close_price && Math::get_spread( tick_bid, m_last_brick.close_price, tick_point_size ) >= m_period ) {
					m_current_brick.open_time   = m_last_brick.close_time;
					m_current_brick.open_price  = m_last_brick.close_price;
					m_current_brick.diff        = Math::get_spread( tick_bid, m_last_brick.close_price, tick_point_size );
					m_current_brick.close_price = tick_bid;
					m_current_brick.close_time  = tick_sending_time;
					m_current_brick.status      = RenkoBrick::STATUS::LONG;

					// correct price and diff if it is != m_period
					if ( m_current_brick.diff > m_period ) {
						m_current_brick.close_price = m_last_brick.close_price + ( m_period * tick_point_size );
						m_current_brick.diff        = m_period;
					}

					m_current_brick.low_price  = m_current_brick.open_price;
					m_current_brick.high_price = m_current_brick.close_price;
					
					// add brick to stack
					m_bricks.push_back( m_current_brick );

#ifdef CMAKE_SHOW_DEBUG_OUTPUT
					ss_console.clear();
					ss_console.str("");
					ss_console << m_current_brick.symbol << " L+LONG  " << m_current_brick;
					console()->info("[RenkoChart] {}", ss_console.str() );
#endif
					
					// signal
					on_brick( m_current_brick );

					// reset current and last brick
					m_current_brick.clear();
					m_last_brick.clear();
					return;
				}
				// open new short brick
				else if ( tick_bid < m_last_brick.open_price && Math::get_spread( tick_bid, m_last_brick.open_price, tick_point_size ) >= m_period ) {
					m_current_brick.open_time   = m_last_brick.open_time;
					m_current_brick.open_price  = m_last_brick.open_price;
					m_current_brick.diff        = Math::get_spread( tick_bid, m_last_brick.open_price, tick_point_size );
					m_current_brick.close_price = tick_bid;
					m_current_brick.close_time  = tick_sending_time;
					m_current_brick.status      = RenkoBrick::STATUS::SHORT;

					if ( m_current_brick.diff > m_period ) {
						m_current_brick.close_price = m_last_brick.open_price - ( m_period * tick_point_size );
						m_current_brick.diff        = m_period;
					}

					m_current_brick.low_price  = m_current_brick.close_price;
					m_current_brick.high_price = m_current_brick.open_price;

					// add brick to stack
					m_bricks.push_back( m_current_brick );

#ifdef CMAKE_SHOW_DEBUG_OUTPUT
					ss_console.clear();
					ss_console.str("");
					ss_console << m_current_brick.symbol << " L+SHORT " << m_current_brick;
					console()->info("[RenkoChart] {}", ss_console.str() );
#endif
					
					// signal
					on_brick( m_current_brick );

					// reset current and last brick
					m_current_brick.clear();
					m_last_brick.clear();
					return;
				}
				break;
			// last brick is SHORT
			case RenkoBrick::STATUS::SHORT:
				// open new short brick
				if ( tick_bid < m_last_brick.close_price && Math::get_spread( tick_bid, m_last_brick.close_price, tick_point_size ) >= m_period ) {
					m_current_brick.open_time   = m_last_brick.close_time;
					m_current_brick.open_price  = m_last_brick.close_price;
					m_current_brick.diff        = Math::get_spread( tick_bid, m_last_brick.close_price, tick_point_size );
					m_current_brick.close_price = tick_bid;
					m_current_brick.close_time  = tick_sending_time;
					m_current_brick.status      = RenkoBrick::STATUS::SHORT;

					if ( m_current_brick.diff > m_period ) {
						m_current_brick.close_price = m_last_brick.close_price - ( m_period * tick_point_size );
						m_current_brick.diff        = m_period;
					}

					m_current_brick.low_price  = m_current_brick.close_price;
					m_current_brick.high_price = m_current_brick.open_price;

					// add brick to stack
					m_bricks.push_back( m_current_brick );
					
#ifdef CMAKE_SHOW_DEBUG_OUTPUT
					ss_console.clear();
					ss_console.str("");
					ss_console << m_current_brick.symbol << " S+SHORT " << m_current_brick;
					console()->info("[RenkoChart] {}", ss_console.str() );
#endif

					// signal
					on_brick( m_current_brick );

					// reset current and last brick
					m_current_brick.clear();
					m_last_brick.clear();
					return;
				}
				// open new long brick
				else if ( tick_bid > m_last_brick.open_price && Math::get_spread( tick_bid, m_last_brick.open_price, tick_point_size ) >= m_period ) {
					m_current_brick.open_time   = m_last_brick.open_time;
					m_current_brick.open_price  = m_last_brick.open_price;
					m_current_brick.diff        = Math::get_spread( tick_bid, m_last_brick.open_price, tick_point_size );
					m_current_brick.close_price = tick_bid;
					m_current_brick.close_time  = tick_sending_time;
					m_current_brick.status      = RenkoBrick::STATUS::LONG;

					if ( m_current_brick.diff > m_period ) {
						m_current_brick.close_price = m_last_brick.open_price + ( m_period * tick_point_size );
						m_current_brick.diff        = m_period;
					}

					m_current_brick.low_price = m_current_brick.open_price;
					m_current_brick.high_price = m_current_brick.close_price;

					// add brick to stack
					m_bricks.push_back( m_current_brick );

#ifdef CMAKE_SHOW_DEBUG_OUTPUT
					ss_console.clear();
					ss_console.str("");
					ss_console << m_current_brick.symbol << " S+LONG  " << m_current_brick;
					console()->info("[RenkoChart] {}", ss_console.str() );
#endif

					// signal
					on_brick( m_current_brick );

					// reset current and last brick
					m_current_brick.clear();
					m_last_brick.clear();
					return;
				}
				break;
			case RenkoBrick::STATUS::NOSTATUS:
				// do nothing
				break;
		}
	}

	/*!
	 * Add initial brick, the very first
	 * 
	 * @param const MarktSnapshot& tick
	 * @return bool True if a brick was added
	 */
	bool RenkoChart::init_brick(const MarketSnapshot& tick) {
		FIX::Locker lock( m_mutex );
	
		// shortcuts
		auto tick_bid          = tick.getBid();
		auto tick_point_size   = tick.getPointSize();
		auto tick_sending_time = tick.getSendingTime();

		// init first brick
		if ( m_init_brick.status == RenkoBrick::STATUS::NOSTATUS && m_init_brick.volume == 0 ) {
			m_init_brick.symbol     = tick.getSymbol();
			m_init_brick.period     = m_period;
			m_init_brick.open_time  = tick_sending_time;
			m_init_brick.open_price = tick_bid;
			m_init_brick.low_price  = tick_bid;
			m_init_brick.volume     = 1;
			m_init_brick.point_size = tick_point_size;
		} else {
			m_init_brick.volume++;
		}

#ifdef CMAKE_SHOW_DEBUG_OUTPUT
		// console output
		std::stringstream ss_console;
#endif

		// make initial brick
		// add LONG brick
		if ( m_init_brick.volume > 1 && tick_bid > m_init_brick.open_price && Math::get_spread( tick_bid, m_init_brick.open_price, tick_point_size ) >= m_period ) {
			m_init_brick.diff        = Math::get_spread( tick_bid, m_init_brick.open_price, tick_point_size );
			m_init_brick.status      = RenkoBrick::STATUS::LONG;
			m_init_brick.close_price = tick_bid;
			m_init_brick.close_time  = tick_sending_time;

			if ( m_init_brick.diff > m_period ) {
				m_init_brick.diff        = m_period;
				m_init_brick.close_price = m_init_brick.open_price + ( m_period * tick_point_size );
			}

			m_init_brick.high_price  = m_init_brick.close_price;

			m_bricks.push_back( m_init_brick );

#ifdef CMAKE_SHOW_DEBUG_OUTPUT
			ss_console.clear();
			ss_console.str("");
			ss_console << m_init_brick.symbol << " 0+LONG  " << m_init_brick;
			console()->info("[RenkoChart] {}", ss_console.str() );
#endif

			// signal
			on_brick( m_init_brick );

			m_last_brick = m_init_brick;
			m_init_brick.clear();

			return true;
		}
		// add short brick
		else if ( m_init_brick.volume > 1 && tick_bid < m_init_brick.open_price && Math::get_spread( tick_bid, m_init_brick.open_price, tick_point_size ) >= m_period ) {
			m_init_brick.diff        = Math::get_spread( tick_bid, m_init_brick.open_price, tick_point_size );
			m_init_brick.status      = RenkoBrick::STATUS::SHORT;
			m_init_brick.close_price = tick_bid;
			m_init_brick.close_time  = tick_sending_time;

			if ( m_init_brick.diff > m_period ) {
				m_init_brick.diff        = m_period;
				m_init_brick.close_price = m_init_brick.open_price - ( m_period * tick_point_size );
			}

			m_init_brick.high_price  = m_init_brick.open_price;
			m_init_brick.low_price   = m_init_brick.close_price;
			
			m_bricks.push_back( m_init_brick );

#ifdef CMAKE_SHOW_DEBUG_OUTPUT
			ss_console.clear();
			ss_console.str("");
			ss_console << m_init_brick.symbol << " 0+SHORT " << m_init_brick;
			console()->info("[RenkoChart] {}", ss_console.str() );
#endif

			// signal
			on_brick( m_init_brick );
			
			m_last_brick = m_init_brick;
			m_init_brick.clear();

			return true;
		}

		return false;
	}

	/*!
	 * Return brick list
	 *
	 * @return std::vector<RenkoBrick>
	 */
	std::vector<RenkoBrick> RenkoChart::brick_list() {
		FIX::Locker lock( m_mutex );
		return m_bricks;
	}

	/*!
	 * Get renko brick at index
	 * The index is backwards:
	 * [4,3,2,1,0] 0 is the newest element
	 * 
	 * @param const int index 
	 * @return       [description]
	 * @throw IDEFIX::out_of_range
	 * @throw IDEFIX::element_not_found
	 */
	RenkoBrick RenkoChart::at(const int index) throw ( IDEFIX::out_of_range, IDEFIX::element_not_found ) {
		FIX::Locker lock( m_mutex );

		if ( index > m_bricks.size() ) {
			// out of range exception
			throw out_of_range(__FILE__, __LINE__);
		}

		int i = 0;
		
		for ( auto it = m_bricks.rbegin(); it != m_bricks.rend(); ++it ) {
			if ( i == index ) {
				return *it;
			}

			i++;
		}

		// not found
		throw element_not_found(__FILE__, __LINE__);
	}

	/*!
	 * Return brick count
	 * 
	 * @return int
	 */
	int RenkoChart::brick_count() {
		FIX::Locker lock( m_mutex );

		return m_bricks.size();
	}
};