/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef IDEFIX_FIX_ROUTER_H
#define IDEFIX_FIX_ROUTER_H
#include <fix8/f8config.h>
#include <fix8/f8includes.hpp>
#include "fix8/idefix8_types.hpp"
#include "fix8/idefix8_router.hpp"
#include "fix8/idefix8_classes.hpp"

class fix_client;

class fix_router : public FIX8::IDEFIX::idefix8_Router
{
  fix_client& _session;

public:
  fix_router(fix_client& session) : _session(session) {}
  virtual ~fix_router() {}

  // Override these methods to receive specific message callbacks.
  // bool operator() (const FIX8::IDEFIX::Heartbeat *msg) const;
  // bool operator() (const FIX8::IDEFIX::TestRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::ResendRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::Reject *msg) const;
  // bool operator() (const FIX8::IDEFIX::SequenceReset *msg) const;
  bool operator() (const FIX8::IDEFIX::Logout *msg) const;
  // bool operator() (const FIX8::IDEFIX::IOI *msg) const;
  // bool operator() (const FIX8::IDEFIX::Advertisement *msg) const;
  // bool operator() (const FIX8::IDEFIX::ExecutionReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::OrderCancelReject *msg) const;
  bool operator() (const FIX8::IDEFIX::Logon *msg) const;
  // bool operator() (const FIX8::IDEFIX::DerivativeSecurityList *msg) const;
  // bool operator() (const FIX8::IDEFIX::NewOrderMultileg *msg) const;
  // bool operator() (const FIX8::IDEFIX::MultilegOrderCancelReplace *msg) const;
  // bool operator() (const FIX8::IDEFIX::TradeCaptureReportRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::TradeCaptureReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::OrderMassStatusRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::QuoteRequestReject *msg) const;
  // bool operator() (const FIX8::IDEFIX::RFQRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::QuoteStatusReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::QuoteResponse *msg) const;
  // bool operator() (const FIX8::IDEFIX::Confirmation *msg) const;
  // bool operator() (const FIX8::IDEFIX::PositionMaintenanceRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::PositionMaintenanceReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::RequestForPositions *msg) const;
  // bool operator() (const FIX8::IDEFIX::RequestForPositionsAck *msg) const;
  // bool operator() (const FIX8::IDEFIX::PositionReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::TradeCaptureReportRequestAck *msg) const;
  // bool operator() (const FIX8::IDEFIX::TradeCaptureReportAck *msg) const;
  // bool operator() (const FIX8::IDEFIX::AllocationReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::AllocationReportAck *msg) const;
  // bool operator() (const FIX8::IDEFIX::ConfirmationAck *msg) const;
  // bool operator() (const FIX8::IDEFIX::SettlementInstructionRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::AssignmentReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::CollateralRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::CollateralAssignment *msg) const;
  // bool operator() (const FIX8::IDEFIX::CollateralResponse *msg) const;
  // bool operator() (const FIX8::IDEFIX::News *msg) const;
  // bool operator() (const FIX8::IDEFIX::CollateralReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::CollateralInquiry *msg) const;
  // bool operator() (const FIX8::IDEFIX::NetworkCounterpartySystemStatusRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::NetworkCounterpartySystemStatusResponse *msg) const;
  // bool operator() (const FIX8::IDEFIX::UserRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::UserResponse *msg) const;
  // bool operator() (const FIX8::IDEFIX::CollateralInquiryAck *msg) const;
  // bool operator() (const FIX8::IDEFIX::ConfirmationRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::Email *msg) const;
  // bool operator() (const FIX8::IDEFIX::NewOrderSingle *msg) const;
  // bool operator() (const FIX8::IDEFIX::NewOrderList *msg) const;
  // bool operator() (const FIX8::IDEFIX::OrderCancelRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::OrderCancelReplaceRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::OrderStatusRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::AllocationInstruction *msg) const;
  // bool operator() (const FIX8::IDEFIX::ListCancelRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::ListExecute *msg) const;
  // bool operator() (const FIX8::IDEFIX::ListStatusRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::ListStatus *msg) const;
  // bool operator() (const FIX8::IDEFIX::AllocationInstructionAck *msg) const;
  // bool operator() (const FIX8::IDEFIX::DontKnowTrade *msg) const;
  // bool operator() (const FIX8::IDEFIX::QuoteRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::Quote *msg) const;
  // bool operator() (const FIX8::IDEFIX::SettlementInstructions *msg) const;
  // bool operator() (const FIX8::IDEFIX::MarketDataRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::MarketDataSnapshotFullRefresh *msg) const;
  // bool operator() (const FIX8::IDEFIX::MarketDataIncrementalRefresh *msg) const;
  // bool operator() (const FIX8::IDEFIX::MarketDataRequestReject *msg) const;
  // bool operator() (const FIX8::IDEFIX::QuoteCancel *msg) const;
  // bool operator() (const FIX8::IDEFIX::QuoteStatusRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::MassQuoteAcknowledgement *msg) const;
  // bool operator() (const FIX8::IDEFIX::SecurityDefinitionRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::SecurityDefinition *msg) const;
  // bool operator() (const FIX8::IDEFIX::SecurityStatusRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::SecurityStatus *msg) const;
  // bool operator() (const FIX8::IDEFIX::TradingSessionStatusRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::TradingSessionStatus *msg) const;
  // bool operator() (const FIX8::IDEFIX::MassQuote *msg) const;
  // bool operator() (const FIX8::IDEFIX::BusinessMessageReject *msg) const;
  // bool operator() (const FIX8::IDEFIX::BidRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::BidResponse *msg) const;
  // bool operator() (const FIX8::IDEFIX::ListStrikePrice *msg) const;
  // bool operator() (const FIX8::IDEFIX::RegistrationInstructions *msg) const;
  // bool operator() (const FIX8::IDEFIX::RegistrationInstructionsResponse *msg) const;
  // bool operator() (const FIX8::IDEFIX::OrderMassCancelRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::OrderMassCancelReport *msg) const;
  // bool operator() (const FIX8::IDEFIX::NewOrderCross *msg) const;
  // bool operator() (const FIX8::IDEFIX::CrossOrderCancelReplaceRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::CrossOrderCancelRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::SecurityTypeRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::SecurityTypes *msg) const;
  // bool operator() (const FIX8::IDEFIX::SecurityListRequest *msg) const;
  // bool operator() (const FIX8::IDEFIX::SecurityList *msg) const;
  // bool operator() (const FIX8::IDEFIX::DerivativeSecurityListRequest *msg) const;
};


#endif //IDEFIX_FIX_ROUTER_H
