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



### Pip Value Calculation When Trading a USD Account

When the USD is listed second in a pair the pip value is fixed and doesn't change, assuming you have a USD dollar account.

The fixed pip amount is:

- $10 for a standard lot which is 100,000 worth of currency.
- $1 for a mini lot which is 10,000 worth of currency.
- $0.10 for a micro lot is 1,000 worth of currency.

These pip values apply to any pair where the USD is listed second, such as the EUR/USD, [GBP/USD](https://www.thebalance.com/best-time-to-day-trade-the-gbp-usd-forex-pair-1031020), AUD/USD, NZD/USD

If the USD isn't listed second:

- Divide the pip values above by the USD/XXX rate. 

For example, to get the pip value of a standard lot for the USD/CAD, when trading a USD account, divide \$10 by the USD/CAD rate. If the USD/CAD rate is 1.2500 the standard lot pip value in USD is \$8, or \$10 divided by 1.25.



### Pip Value Calculation for a Non-USD Account

Whatever currency the account is, when that currency is listed second in a pair the pip values are fixed. 

For example, if you have a Canadian dollar (CAD) account, any pair that is XXX/CAD, such as the USD/CAD will have a fixed pip value. A standard lot is CAD\$10, a mini lot is CAD\$1, and a micro lot is CAD\$0.10.

To find the value of a pip when the CAD is listed first, divide the fixed pip rate by the exchange rate. For example, to find the value of a mini lot, if the CAD/CHF exchange rate is 0.7820, a pip is worth CAD$1.27.
If the pair includes the [JPY](https://www.thebalance.com/best-time-to-day-trade-the-usd-jpy-forex-pair-1031021), for example the JPY/CAD, then multiply the result by 10. For example, if the CAD/JPY is priced at 89.09, to find out the standard pip value divide CAD\$10 by 89.09, then multiply the result by 10, for a pip value of CAD\$11.23.

Go through this process with any account currency to find pip values for pairs that include that currency.



### Pip Value For Other Currency Pairs

Not all currency pairs include your account currency. You may have a USD account, but want to trade the EUR/GBP. How's how to figure out the pip value for pairs that don't include your account currency. 

The second currency is always fixed if a person had an account in that currency. For example, we know that if a person held a GBP account then the EUR/GBP pip value is GBP10 for a standard lot, as discussed above. The next step is converting GBP10 to our own currency. If our account is USD, divide GBP10 by the USD/GBP rate. If the rate is 0.7600, then the pip value is USD$13.16.

If you can only find a "backward" quote, such as the GBP/USD rate being 1.3152, then divide one by the rate to get 0.7600. That is the USD/GBP rate. You can then do the calculation above. 

If your account currency is euros and you want to know the pip value of the AUD/CAD, remember that for a person with a CAD account a standard lot would be CAD$10 for this pair. Convert that CAD$10 to euros by dividing it by the EUR/CAD rate. If the rate is 1.4813, the standard lot pip value is EUR6.75.

Always consider which currency is providing the pip value: the second currency (YYY). Once you know that, convert the fixed pip value in that currency to your own by dividing it by XXX/YYY, where XXX is your own account currency. 



## Pip Value Calculation USD/JPY in EUR account

$$
pipV = (symbolPointSize * positionQty ) \div snapshotEurJpyAsk
$$

The same as in PipValue Calculation but with EUR/JPY as pair to calculate.



## (A) Profit Loss Calculation

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



## Profit Loss USD/JPY Calculation

Step 1 as descibed in A.

Step 2: 

Formular for USD account:
$$
profitLoss = ( ( pipDiff / snapshotBidPrice ) * positionQty )
$$
Formular for EUR account:
$$
profitLoss = ( (pipDiff / eurjpyBidPrice ) * positionQty )
$$

* **eurjpyBidPrice** the snapshot for EUR/JPY to convert to EUR account.



## Profit Loss Calculation for NON EUR Pairs in EUR Account

Pairs like GBP/USD needs to be converted.

Step 1 as described in A.

Step 2:
$$
profitLoss = (( pipDiff / conversionPrice ) * positionQty ) \div snapshotBidPrice
$$

* **conversionPrice** The price to convert to. e.g. for GBP/USD use EUR/GBP Bid as conversion price.

