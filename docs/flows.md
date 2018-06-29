# IDEFIX Flow

This describes how IDEFIX is working.

```mermaid
graph TB
IDEFIX-->StartSession
StartSession-->Login
StartSession-->onCreate
Login-->onLogon
Login-->|Yes|InputLoop
Login-->|No|EndSession

onLogon-->queryTradingStatus
queryTradingStatus-->onMessage

InputLoop((Input Loop))
InputLoop-->FIXManager["FIXManager.method"]
FIXManager-->onMessage

onMessage-->onMsgTSS[MSG TradingSessionStatus]
onMsgTSS-->queryAccounts
onMessage-->onMsgCR[MSG CollateralReport]
onMsgCR-->recordAccount
onMessage-->onMsgPR[MSG PositionReport]
onMsgPR-->updatePositions
onMessage-->onMsgMDSFR[MSG MarketDataSnapshot]
onMsgMDSFR-->updatePrices
onMessage-->onMsgER[MSG ExecutionReport]
onMsgER-->updatePositions

updatePrices-->updatePositions
updatePrices-->updateStrategies

InputLoop-->Logout
Logout-->onLogout
Logout-->EndSession
EndSession-->Exit
```

# FIX Message Flow in FIXManager

This section describes the message flow in a FIX application. Each query follows an answer, which is handled by an onMessage() function.

The document uses [mermaid](https://mermaidjs.github.io/flowchart.html) integration for flowchart image generation.

## queryPositionReport (PositionReport)

This method queries for open and closed positions.

- query: FIX44::RequestForPositions

If there are no positions:

- onMessage: FIX44::RequestForPositionAck

If there are positions:

- onMessage: FIX44::PositionReport

```mermaid
graph TB
A(FIXManager.queryPositionReport) -->C{has Entries?}
C -->|No| D[FIX44::RequestForPositionAck]
C -->|Yes| E[FIX44::PositionReport]
E -->F{Open Position?}
F -->|No| G[FIXManager.updateClosedPosition]
F -->|Yes| H[FIXManager.updateOpenPosition]
```

## queryAccounts (CollateralInquiry)

This method queries the available accounts and account settings.

- query: FIX44::CollateralInquiry
- onMessage: FIX44::CollateralInquiryAck
- onMessage: FIX44::CollateralReport

```mermaid
graph TB
A(FIXManager.queryAccounts)-->B{is Available?}
B-->|No| C[FIX44::CollateralInquiryAck]
B-->|Yes| D[FIX44::CollateralReport]
D-->E[FIXManager.updateAccount]
```

## subscribeMarketData (MarketDataRequest)

This method subscribes to price updates for a symbol.

- query: FIX44::MarketDataRequest
- onMessage: FIX44::MarketDataRequestReject
- onMessage: FIX44::MarketDataSnapshotFullRefresh

```mermaid
graph TB
A(FIXManager.subscribeMarketData)-->B{Is available?}
B-->|No|C[FIX44::MarketDataRequestReject]
B-->|Yes|D[FIX44::MarketDataSnapshotFullRefresh]
D-->E[FIXManager.updatePrices]
D-->F[FIXManager.updateStrategy]
```

## unsubscribeMarketData (MarketDataRequest)

This method unsubscribes a symbol from market data.

- query: FIX44::MarketDataRequest

- onMessage: FIX44::MarketDataRequestReject

```mermaid
graph TB
A(FIXManager.unsubscribeMarketData)-->B{Issue?}
B-->|Yes|C[FIX44::MarketDataRequestReject]
```

## queryClosePosition (NewOrderSingle)

This method queries the order session to close a position by FXCM_POS_ID.

- query: FIX44::NewOrderSingle
- onMessage: FIX44::ExecutionReport

```mermaid
graph TB
A(FIXManager.queryClosePosition)-->B[FIX44::ExecutionReport]
B-->C{FIX::ExecType}
C-->|ExecType_FILL|D[FIXManager.updateOpenPosition]
C-->|ExecType_REJECTED|E[FIXManager.errorPosition]
C-->|ExecType_CANCELLED|F[FIXManager.updateOpenPosition]
```

## marketOrder (NewOrderSingle)

This method queries the order session to open a new market order.

- query: FIX44::NewOrderSingle
- onMessage: FIX44::ExecutionReport

```mermaid
graph TB
A(FIXManager.marketOrder)-->B[FIX44::NewOrderSingle]
B-->C{FIX::ExecType}
C-->|ExecType_FILL|D[FIXManager.updateOpenPosition]
C-->|ExecType_REJECTED|E[FIXManager.errorPosition]
C-->|ExecType_CANCELLED|F[FIXManager.updateOpenPosition]
```

## stopOrder (NewOrderSingle)

This method queries the order session to close a market order by stop. Works only on non hedging accounts!!

- query: FIX44::NewOrderSingle
- onMessage: FIX44::ExecutionReport

```mermaid
graph TB
A(FIXManager.stopOrder)-->B[FIX44::NewOrderSingle]
B-->C{FIX::ExecType}
C-->|ExecType_FILL|D[FIXManager.updateOpenPosition]
C-->|ExecType_REJECTED|E[FIXManager.errorPosition]
C-->|ExecType_CANCELLED|F[FIXManager.updateOpenPosition]
```

## marketOrderWithStopLoss

This method opens a new market order with stoploss order linked.

- query: FIX44::NewOrderList
- onMessage: FIX44::ExecutionReport

```mermaid
graph TB
A(FIXManager.marketOrderWithStopLoss)-->B[FIX44::NewOrderSingle MarketOrder]
A-->C[FIX44::NewOrderSingle StopOrder]
B-->D
C-->D[FIX44::NewOrderList]
D-->E{FIX::ExecType}
E-->|ExecType_FILL|F[FIXManager.updateOpenPosition]
E-->|ExecType_REJECTED|G[FIXManager.errorPosition]
E-->|ExecType_CANCELLED|H[FIXManager.updateOpenPosition]

```

## marketOrderWithStopLossTakeProfit

This method opens a new market order with stoploss order and take profit order linked.

- query: FIX44::NewOrderList
- onMessage: FIX44::ExecutionReport

```mermaid
graph TB
A(FIXManager.marketOrderWithStopLoss)-->B[FIX44::NewOrderSingle MarketOrder]
A-->C[FIX44::NewOrderSingle StopOrder]
A-->C1[FIX44::NewOrderSingle TakeOrder]
B-->D
C1-->D
C-->D[FIX44::NewOrderList]
D-->E{FIX::ExecType}
E-->|ExecType_FILL|F[FIXManager.updateOpenPosition]
E-->|ExecType_REJECTED|G[FIXManager.errorPosition]
E-->|ExecType_CANCELLED|H[FIXManager.updateOpenPosition]
```

## queryTradingStatus (TradingSessionStatusRequest)

This method queries the trading session status. Additional information about the broker account can be retrieved here.

- query: FIX44::TradingSessionStatusRequest
- onMessage: FIX44::TradingSessionStatus

```mermaid
graph TB
A(FIXManager.queryTradingStatus)-->B[FIX44::TradingSessionStatusRequest]
B-->C[FIX44::TradingSessionStatus]
C-->D[FIXManager.queryAccounts]
C-->D1[FIXManager.updateSystemSettings]
```



# FIXManager

## marketSnapshot

This method returns the latest market snapshot.

## closeAllPositions

For each open position sends this method a FIX44::NewOrderSingle Message via queryClosePosition().

## nextRequestID

Returns the next request ID for ClOrdID.

## nextOrderID

Returns the next order ID for ClOrdID.

## recordAccount

Add accountID to the account list.

## getSessionSettingsPtr

Returns the session settings pointer.

## isMarketDataSession

Returns true if this is a market data session.

## isOrderSession

Returns true if this is an order data session.

## updatePrices

Updates bid, ask, spread values and adds latest market snapshot to list.

## getPosition

Returns the open or closed position from internal lists.

## startSession

Starts a new FIX Session.

## endSession

Terminates the current FIX Sessions. Deallocates memory.

## QuickFIX Methods

- onCreate
- onLogon
- onLogout
- toAdmin
- toApp
- fromAdmin
- fromApp