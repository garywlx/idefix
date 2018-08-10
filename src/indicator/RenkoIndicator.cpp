#include "RenkoIndicator.h"
#include "../FIXManager.h"
#include "../Math.h"

namespace IDEFIX {
	RenkoIndicator::RenkoIndicator(const double brick_size, const PriceType price_type)
		: m_brick_size( brick_size ), m_price_type( price_type ) {
		resetCurrentBrick();
	}

	// Get name of this indicator
	std::string RenkoIndicator::getName() const {
		return "RenkoIndicator";
	}

	// Is called on every tick, to update the indicator
	void RenkoIndicator::onTick(FIXManager& manager, const MarketSnapshot& snapshot) {

		// hold price, bid or ask
		double price = 0;
		// set price
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
		if ( m_current_brick.status == 0 && m_current_brick.tick_volume == 0 ) {
			m_current_brick.open_price  = price;
		}

		// increase tick volume
		m_current_brick.tick_volume += 1;

		// calculate pip movement since open price
		//auto pips_moved = std::abs( ( price - m_current_brick.open_price ) * ( 1 / snapshot.getPointSize() ) );
		auto pips_moved = Math::get_spread( price, m_current_brick.open_price, snapshot.getPointSize() );
		
		// if we moved equals or more then our brick size
		// close brick and open new one
		if ( pips_moved >= m_brick_size ) {
			// set close price
			m_current_brick.close_price = price;
			// define if this is up or down brick
			m_current_brick.status      = ( m_current_brick.open_price < m_current_brick.close_price ? 1 : -1 );
			// set close time
			m_current_brick.close_time  = snapshot.getSendingTime();

			// inform for debug
			manager.console()->warn( "[{} BRICK] {} open {:f} close {:f} ticks {:d} close_time {} pips {:f}", 
				snapshot.getSymbol(),
				m_current_brick.status == 1 ? "UP" : "DOWN",
				m_current_brick.open_price,
				m_current_brick.close_price,
				m_current_brick.tick_volume,
				m_current_brick.close_time,
				pips_moved
			);
			// debug eol

			// reset current brick
			resetCurrentBrick();
		}
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