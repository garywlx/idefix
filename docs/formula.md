# Forex Formulas

## Currency Pairs

| MAJORS              | MINOR                   | EXOTIC       |
| ------------------- | ----------------------- | ------------ |
| EUR/USD * (fiber)   | EUR/GBP (chunnel)       | USD/MXN (--) |
| USD/JPY * (gopher)  | EUR/JPY (yuppy)         | GBP/NOK (--) |
| GBP/USD * (cable)   | GBP/JPY (guppy)         | GBP/DKK (--) |
| USD/CHF * (swissie) | NZD/JPY (kiwi yen)      | CHF/NOK (--) |
| AUD/USD (aussie)    | CAD/CHF (loonie swissy) | EUR/TRY (--) |
| USD/CAD (loonie)    | AUD/JPY (--)            | USD/TRY (--) |
| NZD/USD (kiwi)      |                         |              |

\* Popular Currency Pair

() The names of the pair.



## Bid & Ask Price

|                               | Bid    | Ask    |                              |
| ----------------------------- | ------ | ------ | ---------------------------- |
|                               | 1.0916 | 1.0918 | Buy 1 EUR for 1.0918 USD ask |
| Sell 1 EUR for 1.0916 USD bid | 1.0916 | 1.0918 |                              |
|                               | EUR    | USD    |                              |

| Spread =          | AskPrice - | BidPrice |
| ----------------- | ---------- | -------- |
| 0.0002 ( 2 Pips ) | 1.0918 -   | 1.0916   |

AskPrice > BidPrice



## Pip Value Calculation

https://www.thebalance.com/calculating-pip-value-in-forex-pairs-1031022

Formular for USD account:
$$
pipV = symbolPointSize * positionQty
$$
Formular for EUR account:
$$
pipV = ( symbolPointSize * positionQty ) \div symbolPrice
$$


- **pointSize** is how many decimal places has the price of a symbol. default is 0.0001.
- **symbolPrice** is the current symbol ask price.
- **positionQty** is the value for the position qty.



## Profit Loss Calculation

Step 1: get pip difference of current bid price and position entry price.

Formular for BUY position: 
$$
pipDiff = snapshotBidPrice - positionEntryPrice
$$
Formular for SELL position:
$$
pipDiff = positionEntryPrice - snapshotBidPrice
$$
Step 2: 

Formular for USD account:
$$
profitLoss = pipDiff * positionQty
$$
Formular for EUR account:
$$
profitLoss = ( pipDiff * positionQty ) / snapshotBidPrice
$$


- **pipDiff** price difference in pip in decimal.
- **snapshotBidPrice** the bid price of the current symbol market snapshot.
- **positionEntryPrice** the entry price of the position.
- **positionQty** the quantity of the position.





## Convert Price to USD

$$
usdV = otherCurrencyValue \div usdPrice
$$

- **otherCurrencyValue** the value to convert
- **usdPrice** the current usd price.







## Andre Hedging Formel

$$
pnlVLong = (( currentAsk - entry ) * pipV * 100 ) * ( qty / 100000 )\\
pnlVShort = (( entry - currentBid ) * pipV * 100 ) * ( qty / 100000 )
$$

- **currentAsk** this is the ask value of the traded pair 
- **entry** this is the entry price of the position
- **pipV** this is the pip value for the traded pair
- **qty** this is the position size

