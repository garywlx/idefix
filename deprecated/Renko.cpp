#include "Renko.h"
#include "../Math.h"
#include <stdexcept>
#include "../FIXManager.h"

namespace IDEFIX {
	Renko::Renko(const double period, const bool verbose_mode): m_period( period ), m_verbose_mode(verbose_mode) {
		m_init_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
		m_last_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
		m_current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
	}

	Renko::~Renko() {

	}

	/*!
	 * Add tick to list and calculate renko brick
	 * Returns true if successful
	 * 
	 * @param const Tick&  tick
	 * @return bool
	 */
	bool Renko::add_tick(const Tick& tick) {
		FIX::Locker lock( m_mutex );

		// add tick to list
		m_ticks.push_back( tick );

		// is this the first brick?
		if ( m_bricks.size() == 0 ) {
			if ( init_brick( tick ) ) {
				return true;
			}
		} 
		// get last brick
		else {
			m_last_brick = m_bricks.back();
		}

		// renko calculation
		if ( m_current_brick.status == RenkoBrick::STATUS::NOSTATUS && m_current_brick.volume == 0 ) {
			m_current_brick.open_time  = m_last_brick.close_time;
			m_current_brick.open_price = m_last_brick.close_price;
			m_current_brick.low_price  = m_last_brick.close_price;
			m_current_brick.volume     = 1;
			m_current_brick.point_size = tick.point_size;
		} 

		// increase volume
		m_current_brick.volume++;

		// make brick
		switch( m_last_brick.status ) {
			// last brick is LONG 
			case RenkoBrick::STATUS::LONG:
				// open new long brick
				if ( tick.bid > m_last_brick.close_price && Math::get_spread( tick.bid, m_last_brick.close_price, tick.point_size ) >= m_period ) {
					m_current_brick.open_time   = m_last_brick.close_time;
					m_current_brick.open_price  = m_last_brick.close_price;
					m_current_brick.diff        = Math::get_spread( tick.bid, m_last_brick.close_price, tick.point_size );
					m_current_brick.close_price = tick.bid;
					m_current_brick.close_time  = tick.datetime;
					m_current_brick.status      = RenkoBrick::STATUS::LONG;

					// correct price and diff if it is != m_period
					if ( m_current_brick.diff > m_period ) {
						m_current_brick.close_price = m_last_brick.close_price + ( m_period * tick.point_size );
						m_current_brick.diff        = m_period;
					}

					m_current_brick.low_price  = m_current_brick.open_price;
					m_current_brick.high_price = m_current_brick.close_price;
					
					// add brick to stack
					m_bricks.push_back( m_current_brick );

					if ( m_verbose_mode ) {
						cout << "L+LONG  " << m_current_brick << endl;
					}

					// reset current and last brick
					m_current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
					m_last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};

					return true;
				}
				// open new short brick
				else if ( tick.bid < m_last_brick.open_price && Math::get_spread( tick.bid, m_last_brick.open_price, tick.point_size ) >= m_period ) {
					m_current_brick.open_time   = m_last_brick.open_time;
					m_current_brick.open_price  = m_last_brick.open_price;
					m_current_brick.diff        = Math::get_spread( tick.bid, m_last_brick.open_price, tick.point_size );
					m_current_brick.close_price = tick.bid;
					m_current_brick.close_time  = tick.datetime;
					m_current_brick.status      = RenkoBrick::STATUS::SHORT;

					if ( m_current_brick.diff > m_period ) {
						m_current_brick.close_price = m_last_brick.open_price - ( m_period * tick.point_size );
						m_current_brick.diff        = m_period;
					}

					m_current_brick.low_price  = m_current_brick.close_price;
					m_current_brick.high_price = m_current_brick.open_price;

					// add brick to stack
					m_bricks.push_back( m_current_brick );

					if ( m_verbose_mode ) {
						cout << "L+SHORT " << m_current_brick << endl;
					}
					
					// reset current and last brick
					m_current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
					m_last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};

					return true;
				}
				break;
			// last brick is SHORT
			case RenkoBrick::STATUS::SHORT:
				// open new short brick
				if ( tick.bid < m_last_brick.close_price && Math::get_spread( tick.bid, m_last_brick.close_price, tick.point_size ) >= m_period ) {
					m_current_brick.open_time   = m_last_brick.close_time;
					m_current_brick.open_price  = m_last_brick.close_price;
					m_current_brick.diff        = Math::get_spread( tick.bid, m_last_brick.close_price, tick.point_size );
					m_current_brick.close_price = tick.bid;
					m_current_brick.close_time  = tick.datetime;
					m_current_brick.status      = RenkoBrick::STATUS::SHORT;

					if ( m_current_brick.diff > m_period ) {
						m_current_brick.close_price = m_last_brick.close_price - ( m_period * tick.point_size );
						m_current_brick.diff        = m_period;
					}

					m_current_brick.low_price  = m_current_brick.close_price;
					m_current_brick.high_price = m_current_brick.open_price;

					// add brick to stack
					m_bricks.push_back( m_current_brick );
					
					if ( m_verbose_mode ) {
						cout << "S+SHORT " << m_current_brick << endl;
					}

					// reset current and last brick
					m_current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
					m_last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};

					return true;
				}
				// open new long brick
				else if ( tick.bid > m_last_brick.open_price && Math::get_spread( tick.bid, m_last_brick.open_price, tick.point_size ) >= m_period ) {
					m_current_brick.open_time   = m_last_brick.open_time;
					m_current_brick.open_price  = m_last_brick.open_price;
					m_current_brick.diff        = Math::get_spread( tick.bid, m_last_brick.open_price, tick.point_size );
					m_current_brick.close_price = tick.bid;
					m_current_brick.close_time  = tick.datetime;
					m_current_brick.status      = RenkoBrick::STATUS::LONG;

					if ( m_current_brick.diff > m_period ) {
						m_current_brick.close_price = m_last_brick.open_price + ( m_period * tick.point_size );
						m_current_brick.diff        = m_period;
					}

					m_current_brick.low_price = m_current_brick.open_price;
					m_current_brick.high_price = m_current_brick.close_price;

					// add brick to stack
					m_bricks.push_back( m_current_brick );

					if ( m_verbose_mode ) {
						cout << "S+LONG  " << m_current_brick << endl;
					}

					// reset current and last brick
					m_current_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};
					m_last_brick    = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};

					return true;
				}
				break;
			case RenkoBrick::STATUS::NOSTATUS:
				// do nothing
				break;
		}

		return false;
	}

	/*!
	 * Add initial brick, the very first
	 * 
	 * @param const Tick& tick
	 * @return bool True if a brick was added
	 */
	bool Renko::init_brick(const Tick& tick) {
		FIX::Locker lock( m_mutex );
	
		// init first brick
		if ( m_init_brick.status == RenkoBrick::STATUS::NOSTATUS && m_init_brick.volume == 0 ) {
			m_init_brick.open_time  = tick.datetime;
			m_init_brick.open_price = tick.bid;
			m_init_brick.low_price  = tick.bid;
			m_init_brick.volume     = 1;
			m_init_brick.point_size = tick.point_size;
		} else {
			m_init_brick.volume++;
		}

		// make initial brick
		// add LONG brick
		if ( m_init_brick.volume > 1 && tick.bid > m_init_brick.open_price && Math::get_spread( tick.bid, m_init_brick.open_price, tick.point_size ) >= m_period ) {
			m_init_brick.diff        = Math::get_spread( tick.bid, m_init_brick.open_price, tick.point_size );
			m_init_brick.status      = RenkoBrick::STATUS::LONG;
			m_init_brick.close_price = tick.bid;
			m_init_brick.close_time  = tick.datetime;

			if ( m_init_brick.diff > m_period ) {
				m_init_brick.diff        = m_period;
				m_init_brick.close_price = m_init_brick.open_price + ( m_period * tick.point_size );
			}

			m_init_brick.high_price  = m_init_brick.close_price;

			m_bricks.push_back( m_init_brick );

			if ( m_verbose_mode ) {
				cout << "0+LONG  " << m_init_brick << endl;
			}

			m_last_brick = m_init_brick;
			m_init_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};

			return true;
		}
		// add short brick
		else if ( m_init_brick.volume > 1 && tick.bid < m_init_brick.open_price && Math::get_spread( tick.bid, m_init_brick.open_price, tick.point_size ) >= m_period ) {
			m_init_brick.diff        = Math::get_spread( tick.bid, m_init_brick.open_price, tick.point_size );
			m_init_brick.status      = RenkoBrick::STATUS::SHORT;
			m_init_brick.close_price = tick.bid;
			m_init_brick.close_time  = tick.datetime;

			if ( m_init_brick.diff > m_period ) {
				m_init_brick.diff        = m_period;
				m_init_brick.close_price = m_init_brick.open_price - ( m_period * tick.point_size );
			}

			m_init_brick.high_price  = m_init_brick.open_price;
			m_init_brick.low_price   = m_init_brick.close_price;
			
			m_bricks.push_back( m_init_brick );

			if ( m_verbose_mode ) {
				cout << "0+SHORT " << m_init_brick << endl;
			}

			m_last_brick = m_init_brick;
			m_init_brick = {"","","",0,0,0,0,RenkoBrick::STATUS::NOSTATUS,0,0};

			return true;
		}

		return false;
	}

	/*!
	 * Return brick list
	 *
	 * @return std::vector<RenkoBrick>
	 */
	std::vector<RenkoBrick> Renko::brick_list() {
		FIX::Locker lock( m_mutex );
		return m_bricks;
	}

	/*!
	 * Return tick list
	 * 
	 * @return std::vector<Tick>
	 */
	std::vector<Tick> Renko::tick_list() {
		FIX::Locker lock( m_mutex );
		return m_ticks;
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
	RenkoBrick Renko::at(const int index) throw ( IDEFIX::out_of_range, IDEFIX::element_not_found ) {
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

	// Get name of this indicator, must be unique
	std::string Renko::getName() const {
		return "renko_brick_indicator";
	}
	// Is called on every tick
	void Renko::onTick(FIXManager& manager, const MarketSnapshot& snapshot) {
		try {
			Tick tick = {snapshot.getSendingTime(), snapshot.getBid(), snapshot.getAsk(), snapshot.getPointSize()};
			if ( add_tick( tick ) ) {
				auto brick = at( 0 );

				cout << brick << endl;
			}	
		} catch( IDEFIX::out_of_range& e ) {
			manager.console()->error( "[{}:out_of_range] {}", __FUNCTION__, e.what() );
			throw;
		} catch( IDEFIX::element_not_found& e ) {
			manager.console()->error( "[{}:element_not_found] {}", __FUNCTION__, e.what() );
			throw;
		} catch( ... ) {
			manager.console()->error( "[{}:exception] other.", __FUNCTION__ );
		}
	}

};