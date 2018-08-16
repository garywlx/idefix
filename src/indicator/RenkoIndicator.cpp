#include "RenkoIndicator.h"
#include "../FIXManager.h"
#include "../Math.h"

namespace IDEFIX {
	RenkoIndicator::RenkoIndicator(const double brick_size, const PriceType price_type, RENKO::ONCLOSE_CB_T callback)
		: m_brick_size( brick_size ), m_price_type( price_type ), m_on_close_cb( callback ) {
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
		if ( m_current_brick.status == RenkoBrick::STATUS::NOSTATUS && m_current_brick.tick_volume == 0 ) {
			m_current_brick.open_price  = price;
			m_current_brick.symbol      = snapshot.getSymbol();
		}

		// increase tick volume
		m_current_brick.tick_volume += 1;

		// calculate pip movement since open price
		auto pips_moved = Math::get_spread( price, m_current_brick.open_price, snapshot.getPointSize() );
		
		// if we moved equals or more then our brick size
		// close brick and open new one
		if ( pips_moved >= m_brick_size ) {
			// set close price
			m_current_brick.close_price = price;
			// define if this is up or down brick
			m_current_brick.status      = ( m_current_brick.open_price < m_current_brick.close_price ? RenkoBrick::STATUS::LONG : RenkoBrick::STATUS::SHORT );
			// set close time
			m_current_brick.close_time  = snapshot.getSendingTime();
			// call callback
			onClose( m_current_brick );

			// reset current brick
			resetCurrentBrick();
		}
	}

	/*!
	 * Reset current brick with default values
	 */
	void RenkoIndicator::resetCurrentBrick() {
		m_current_brick = RenkoBrick{0,0,0,RenkoBrick::STATUS::NOSTATUS,""};
	}

	/*!
	 * Get latest renko brick from stack
	 * 
	 * @return RenkoBrick
	 */
	RenkoBrick RenkoIndicator::getLatest() {
		RenkoBrick brick{0,0,0,RenkoBrick::STATUS::NOSTATUS,"",""};
		if ( m_bricks.empty() ) return brick;

		return m_bricks.back();
	}

	/*!
	 * Call the onClose callback if it is registered
	 * 
	 * @param const RenkoBrick& brick current brick
	 */
	void RenkoIndicator::onClose(const RenkoBrick& brick) {
		if ( m_on_close_cb != nullptr ) {
			m_on_close_cb( brick );
		}
	}
};