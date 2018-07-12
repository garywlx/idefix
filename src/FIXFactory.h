#ifndef IDEFIX_FIXFACTORY_H
#define IDEFIX_FIXFACTORY_H

#include <iostream>
#include "MarketOrder.h"
#include "FXCMFields.h"
#include <quickfix/Fields.h>
#include <quickfix/fix44/TradingSessionStatusRequest.h>
#include <quickfix/fix44/CollateralInquiry.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/NewOrderSingle.h>
#include <quickfix/fix44/NewOrderList.h>
#include <quickfix/fix44/RequestForPositions.h>
#include <quickfix/fix44/OrderStatusRequest.h>
#include <quickfix/fix44/OrderMassStatusRequest.h>
#include <vector>
#include <exception>

namespace IDEFIX {
class CustomEmptyException: public std::exception {
private:
	std::string m_msg;
public:
	CustomEmptyException(const std::string& msg): m_msg(msg) {}
	virtual const char* what() const throw() {
		return m_msg.c_str();
	}
};

class FIXFactory {
public:

	/*!
	 * Needed for NewOrderSingle
	 */
	enum SingleOrderType {
		MARKET_ORDER,
		STOPORDER,
		CLOSEORDER,
		MARKET_ORDER_SL, // OCO
		MARKET_ORDER_SL_TP // ELS
	};

	/*!
	 * Create TradingSessionStatusRequest
	 * 
	 * @param const std::string& requestID 
	 * @return FIX44::TradingSessionStatusRequest
	 */
	static FIX44::TradingSessionStatusRequest TradingSessionStatusRequest(const std::string& requestID) {
		FIX44::TradingSessionStatusRequest request;
		request.setField( FIX::TradSesReqID( requestID ) );
		request.setField( FIX::TradingSessionID( "FXCM" ) );
  		request.setField( FIX::SubscriptionRequestType( FIX::SubscriptionRequestType_SNAPSHOT ) );

  		return request;
	}

	/*!
	 * Request CollateralReport message. We will receive a CollateralReport for each
     * account under our login
     * 
	 * @param const std::string& requestID
	 * @return FIX44::CollateralInquiry
	 */
	static FIX44::CollateralInquiry CollateralInquiry(const std::string& requestID) {
		FIX44::CollateralInquiry request;
		request.setField( FIX::CollInquiryID( requestID ) );
		request.setField( FIX::TradingSessionID( "FXCM" ) );
		request.setField( FIX::SubscriptionRequestType( FIX::SubscriptionRequestType_SNAPSHOT ) );

		return request;
	}

	/*!
	 * Create message for Subscribe and Unsubscribe to/from MarketData
	 * 
	 * @param const std::string& symbol like EUR/USD
	 * @param const char requestType FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES
	 * @return FIX44::MarketDataRequest
	 */
	static FIX44::MarketDataRequest MarketDataRequest(const std::string& symbol, const char requestType){
		FIX44::MarketDataRequest request;
		request.setField( FIX::MDReqID( "Request_" + symbol ) );
		request.setField( FIX::SubscriptionRequestType( requestType ) );
		request.setField( FIX::MarketDepth( 0 ) );
		request.setField( FIX::NoRelatedSym( 1 ) );

		// Add the NoRelatedSym group to the request with Symbol
		// field set to EUR/USD
		FIX44::MarketDataRequest::NoRelatedSym symbols_group;
		symbols_group.setField( FIX::Symbol( symbol ) );
		request.addGroup( symbols_group );

		// Add the NoMDEntryTypes group to the request for each MDEntryType
		// that we are subscribing to. This includes Bid, Offer, High and Low
		FIX44::MarketDataRequest::NoMDEntryTypes entry_types;
		entry_types.setField( FIX::MDEntryType( FIX::MDEntryType_BID ) );
		request.addGroup( entry_types );
		entry_types.setField( FIX::MDEntryType( FIX::MDEntryType_OFFER ) );
		request.addGroup( entry_types );
		entry_types.setField( FIX::MDEntryType( FIX::MDEntryType_TRADING_SESSION_HIGH_PRICE ) );
		request.addGroup( entry_types );
		entry_types.setField( FIX::MDEntryType( FIX::MDEntryType_TRADING_SESSION_LOW_PRICE ) );
		request.addGroup( entry_types );

		return request;
	}

	/*!
	 * Create message for NewOrderSingle
	 * 
	 * @param const std::string& requestID
	 * @param const MarketOrder& marketOrder
	 * @param const SingleOrderType singleOrderType default is MARKET_ORDER
	 * @return FIX44::NewOrderSingle
	 * @throws CustomEmptyException
	 */
	static FIX44::NewOrderSingle NewOrderSingle(const std::string& requestID, const MarketOrder& marketOrder, const SingleOrderType singleOrderType = SingleOrderType::MARKET_ORDER){
		
		// checks.
		if( requestID.empty() ){
			throw new CustomEmptyException("requestID is empty!");
		}
		if( marketOrder.getAccountID().empty() ){
			throw new CustomEmptyException("marketOrder.AccountID is empty!");
		}

		FIX44::NewOrderSingle request;
		request.setField( FIX::ClOrdID( requestID ) );
		request.setField( FIX::Account( marketOrder.getAccountID() ) );
		request.setField( FIX::Symbol( marketOrder.getSymbol() ) );
		request.setField( FIX::TradingSessionID( "FXCM" ) );
		request.setField( FIX::TransactTime() );
		request.setField( FIX::OrderQty( marketOrder.getQty() ) );
		request.setField( FIX::Side( marketOrder.getSide() ) );
		request.setField( FIX::OrdType( FIX::OrdType_MARKET ) );

		// MARKET ORDER
		if( singleOrderType == SingleOrderType::MARKET_ORDER ) {
			request.setField( FIX::TimeInForce( FIX::TimeInForce_FILL_OR_KILL ) );
		} 
		// STOP ORDER
		else if ( singleOrderType == SingleOrderType::STOPORDER ) {
			request.setField( FIX::OrdType( FIX::OrdType_STOP ) );
			request.setField( FIX::Side( marketOrder.getOpposide() ) );
			request.setField( FIX::StopPx( marketOrder.getStopPrice() ) );
			request.setField( FIX::PositionEffect( FIX::PositionEffect_CLOSE ) );
		}
		// CLOSE ORDER
		else if( singleOrderType == SingleOrderType::CLOSEORDER ) {

			if( marketOrder.getPosID().empty() ){
				throw new CustomEmptyException("marketOrder.PosID is empty!");
			}

			request.setField( FXCM_FIX_FIELDS::FXCM_POS_ID, marketOrder.getPosID() );
			request.setField( FIX::Side( marketOrder.getOpposide() ) ) ;
		}

		return request;
	}

	/*!
	 * Create request for NewOrderList with 3 requestIDs for single market order
	 * used for order with stop loss
	 * 
	 * @param const std::string requestIDs[] 3 requestIDs for market order with stoploss, 4 requestIDs for market order with stoploss and takeprofit
	 * @param const MarketOrder marketOrder
	 * @return FIX44::NewOrderList
	 * @throws Exception
	 */
	static FIX44::NewOrderList NewOrderList(const std::vector<std::string> requestIDs, const MarketOrder& marketOrder) {

		FIX44::NewOrderList olist;
		
		// enough requestIDs?
		if( requestIDs.size() < 3 ){
			throw new CustomEmptyException("Not enough requestIDs!");
		}

		// has take profit?
		bool hasTP = ( requestIDs.size() == 4 && marketOrder.getTakePrice() != 0 );

		olist.setField( FIX::ListID( requestIDs[0] ) );
		olist.setField( FIX::TotNoOrders( ( hasTP ? 3 : 2 ) ) );
		olist.setField( FIX::FIELD::ContingencyType, "101" ); // ELS - Entry with Limit and Stop

		// Order.
		FIX44::NewOrderList::NoOrders order;
		order.setField( FIX::ClOrdID( requestIDs[1] )  );
		order.setField( FIX::ListSeqNo( 0 ) );
		order.setField( FIX::ClOrdLinkID( "1" ) ); // link to another clordid in list
		order.setField( FIX::Account( marketOrder.getAccountID() ) );
		order.setField( FIX::Symbol( marketOrder.getSymbol() ) );
		order.setField( FIX::Side( marketOrder.getSide() ) );
		order.setField( FIX::OrderQty( marketOrder.getQty() ) );
		order.setField( FIX::OrdType( FIX::OrdType_MARKET ) );
		olist.addGroup( order );

		// StopLoss.
		FIX44::NewOrderList::NoOrders stop;
		stop.setField( FIX::ClOrdID( requestIDs[2] ) );
		stop.setField( FIX::ListSeqNo( 1 ) );
		stop.setField( FIX::ClOrdLinkID( "2" ) );
		stop.setField( FIX::Account( marketOrder.getAccountID() ) );
		stop.setField( FIX::Side( marketOrder.getOpposide() ) );
		stop.setField( FIX::Symbol( marketOrder.getSymbol() ) );
		stop.setField( FIX::OrderQty( marketOrder.getQty() ) );
		stop.setField( FIX::OrdType( FIX::OrdType_STOP ) );
		stop.setField( FIX::StopPx( marketOrder.getStopPrice() ) );
		olist.addGroup( stop );

		// TakeProfit.
		if( hasTP ) {
			FIX44::NewOrderList::NoOrders limit;
			limit.setField( FIX::ClOrdID( requestIDs[3] ) );
			limit.setField( FIX::ListSeqNo( 2 ) );
			limit.setField( FIX::ClOrdLinkID( "2" ) );
			limit.setField( FIX::Account( marketOrder.getAccountID() ) );
			limit.setField( FIX::Side( marketOrder.getOpposide() ) );
			limit.setField( FIX::Symbol( marketOrder.getSymbol() ) );
			limit.setField( FIX::OrderQty( marketOrder.getQty() ) );
			limit.setField( FIX::OrdType( FIX::OrdType_LIMIT ) );
			limit.setField( FIX::Price( marketOrder.getTakePrice() ) );
			olist.addGroup( limit );
		}
		
		return olist;
	}

	static FIX44::RequestForPositions RequestForPositions(const std::string& requestID, const std::string& accountID, const FIX::PosReqType posReqType){
		// Set default fields
		FIX44::RequestForPositions request;
		request.setField( FIX::PosReqID("PosRequest_" + requestID ) );
		request.setField( FIX::PosReqType( posReqType ) );

		// AccountID for the request. This must be set for routing purposes. We must
		// also set the Parties AccountID field in the NoPartySubIDs group
		request.setField( FIX::Account( accountID ) );
		request.setField( FIX::SubscriptionRequestType( FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES ) );
		request.setField( FIX::AccountType( FIX::AccountType_ACCOUNT_IS_CARRIED_ON_NON_CUSTOMER_SIDE_OF_BOOKS_AND_IS_CROSS_MARGINED ) );
		request.setField( FIX::TransactTime() );
		request.setField( FIX::ClearingBusinessDate() );
		request.setField( FIX::TradingSessionID( "FXCM" ) );

		// Set NoPartyIDs group. These values are always as seen below
		request.setField( FIX::NoPartyIDs( 1 ) );
		FIX44::RequestForPositions::NoPartyIDs parties_group;
		parties_group.setField( FIX::PartyID( "FXCM ID" ) );
		parties_group.setField( FIX::PartyIDSource( 'D' ) );
		parties_group.setField( FIX::PartyRole( 3 ) );
		parties_group.setField( FIX::NoPartySubIDs( 1 ) );

		// Set NoPartySubIDs group
		FIX44::RequestForPositions::NoPartyIDs::NoPartySubIDs sub_parties;
		sub_parties.setField( FIX::PartySubIDType( FIX::PartySubIDType_SECURITIES_ACCOUNT_NUMBER ) );

		// Set Parties AccountID
		sub_parties.setField( FIX::PartySubID( accountID ) );

		// Add NoPartySubIds group
		parties_group.addGroup( sub_parties );

		// Add NoPartyIDs group
		request.addGroup( parties_group );

		return request;
	}

	static FIX44::OrderStatusRequest OrderStatusRequest(const std::string& orderID, const std::string& accountID, const std::string& symbol){

		if( orderID.empty() ){
			throw new CustomEmptyException("orderID is empty!");
		}

		FIX44::OrderStatusRequest request;
		request.setField( FIX::OrderID( orderID ) );
		request.setField( FIX::Account( accountID ) );
		request.setField( FIX::Symbol( symbol ) );

		return request;
	}

	/*!
	 * Create request for all positions at the system
	 * 
	 * @param const std::string& requestID
	 * @param const std::string& accountID
	 * @return FIX44::OrderMassStatusRequest
	 * @throws CustomEmptyException
	 */
	static FIX44::OrderMassStatusRequest OrderMassStatusRequest(const std::string& requestID, const std::string& accountID) {
		if ( requestID.empty() ) throw new CustomEmptyException("requestID is empty!");
		if ( accountID.empty() ) throw new CustomEmptyException("accountID is empty!");

		FIX44::OrderMassStatusRequest request;
		request.setField( FIX::MassStatusReqID( requestID ) );
		request.setField( FIX::MassStatusReqType( FIX::MassStatusReqType_STATUS_FOR_ALL_ORDERS ) ); 
		request.setField( FIX::Account( accountID ) );

		return request;
	}
};	
}

#endif