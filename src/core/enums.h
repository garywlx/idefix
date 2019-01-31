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
			TRAIL // Trail Order
		};

		/**
		 * Order Time In Force
		 */
		enum TIF {
			DAY, // DAY (expires at the end of the trading day)
			GTC // Good Till Cancelled
		};

		/**
		 * 
		 */
		enum OrderStatus {
			NEW, 
			FILLED, 
			REMOVED, 
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

		// /**
		//  * Exchange Order Events
		//  */
		// enum ExchangeOrderEvent {
		//     MARKET_ORDER_SET_SL,
		//     MARKET_ORDER_SET_TP,
		//     MARKET_ORDER_CANCELED,
		//     MARKET_ORDER_REJECT,
		//     MARKET_ORDER_NEW,
		//     MARKET_ORDER_TP_HIT,
		//     MARKET_ORDER_SL_HIT
		// };

		enum ExecutionType {
			ENTRY,
			EXIT,
			SAR // Stop and Reverse
		};
	};
};