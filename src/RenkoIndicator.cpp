#include "RenkoIndicator.h"
#include "FIXManager.h"
#include <cmath>

namespace IDEFIX {
	RenkoIndicator::RenkoIndicator(const int brick_size, const PriceType price_type)
		: m_brick_size( brick_size ), m_price_type( price_type ) {
		resetCurrentBrick();
	}

	// Get name of this indicator
	std::string RenkoIndicator::getName() const {
		return "RenkoIndicator";
	}

	// Is called on every tick, to update the indicator
	void RenkoIndicator::onTick(FIXManager& manager, const MarketSnapshot& snapshot) {

		double price = 0;

		switch( m_price_type ) {
			default:
			case PriceType::BID_PRICE:
				price = snapshot.getBid();
				break;
			case PriceType::ASK_PRICE:
				price = snapshot.getAsk();
				break;
		}

		// first tick
		if ( m_current_brick.is_up == 0 && m_current_brick.tick_volume == 0 ) {
			m_current_brick.tick_volume += 1;
			m_current_brick.first_price  = price;
			m_current_brick.last_price   = price;
		} else {
			m_current_brick.tick_volume += 1;
			m_current_brick.last_price   = price;
		}

		auto pips_moved = std::abs( (m_current_brick.last_price - m_current_brick.first_price) * ( 1 / snapshot.getPointSize() ) );

		m_pips_moved += pips_moved;

		manager.console()->info( "pips_moved pointSize {:f} lp {:f} fp {:f} moved {:f} ", snapshot.getPointSize(), m_current_brick.last_price, m_current_brick.first_price, m_pips_moved );
		
		int total_move = static_cast<int>( m_pips_moved );
		if ( total_move % m_brick_size == 0 ) {
			manager.console()->info( "total_move {:d}", total_move );
			resetCurrentBrick();
			m_pips_moved = 0;
		}

		/*if ( pips_moved % m_brick_size == 0 ) {
			manager.console()->info( "RenkoIndicator is moved {:d) pips", pips_moved);

			// add current brick to list, and reset current brick
			RenkoBrick brick = m_current_brick;
			brick.close_time = snapshot.getSendingTime();
			m_bricks.push_back( brick );
			resetCurrentBrick();

			pips_moved = 0;
		}*/
	}

	/*!
	 * Reset current brick with default values
	 */
	void RenkoIndicator::resetCurrentBrick() {
		m_current_brick = RenkoBrick{0,0,0,0,""};
	}

	/*!
	 * Get latest renko brick from stack
	 * 
	 * @return RenkoBrick
	 */
	RenkoBrick RenkoIndicator::getLatest() {
		RenkoBrick brick{0,0,0,0,""};
		if ( m_bricks.empty() ) return brick;

		return m_bricks.back();
	}
};