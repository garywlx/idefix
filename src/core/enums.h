#pragma once

namespace idefix {
	namespace enums {
		/**
		 * Type of order
		 */
		enum OrderType {
			LIMIT, // Limit Order
			MARKET, // Market Order
			STOP, // Stop Order
			TRAIL, // Trail Order,
			CLOSE, // FXCM Close order
			POSITION, // running position
			TRADE // closed position
		};

		/**
		 * Order Time In Force
		 */
		enum TIF {
			DAY, // DAY (expires at the end of the trading day)
			GTC, // Good Till Cancelled
			FOK // Fill or Kill
		};

		/**
		 * 
		 */
		enum OrderStatus {
			NEW, 
			FILLED,
			PARTIAL_FILLED,
			CANCELED,
			REJECTED, 
			STOPPED,
			UPDATE
		};

		/**
		 * Order Action (Buy or Sell)
		 */
		enum OrderAction {
			BUY,
			SELL
		};

		/**
		 * Instrument Product Type
		 */
		enum InstrumentType {
			UNKNOWN = 0,
			CURRENCY = 4, 
			INDEX = 7, 
			COMMODITY = 2
		};

		/**
		 * Execution Type
		 */
		enum ExecutionType {
			POSITIONS, // active
			TRADES, // done
			EXERCISES,
			ASSIGNMENTS,
			SETTLEMENT_ACTIVITY,
			BACKOUT_MESSAGE,
			DELTA_POSITIONS
		};
	};
};