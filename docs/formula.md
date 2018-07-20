# Forex Formulas

## Currency Pairs

| Major   | USD Major | EUR Major | Crossrates Major |
| ------- | --------- | --------- | ---------------- |
| EUR/USD | EUR/USD   | EUR/USD   | AUD/CAD          |
| USD/JPY | USD/JPY   | EUR/GBP   | AUD/CHF          |
| GBP/USD | GBP/USD   | EUR/JPY   | AUD/JPY          |
| USD/CHF | USD/CHF   | EUR/CHF   | AUD/NZD          |
| EUR/GBP | AUD/USD   | EUR/AUD   | CAD/CHF          |
| EUR/JPY | USD/CAD   | EUR/CAD   | CAD/JPY          |
| EUR/CHF | NZD/USD   | EUR/NZD   | CHF/JPY          |
| AUD/USD |           |           | GBP/AUD          |
| USD/CAD |           |           | GBP/CAD          |
| NZD/USD |           |           | GBP/CHF          |
|         |           |           | GBP/JPY          |
|         |           |           | GBP/NZD          |
|         |           |           | NZD/CAD          |
|         |           |           | NZD/CHF          |
|         |           |           | NZD/JPY          |

## Bid/Ask

|                   | Bid    | Ask    |                  |
| ----------------- | ------ | ------ | ---------------- |
|                   | 95.620 | 95.650 | Open BUY use ASK |
| Open SELL use BID | 96.400 | 96.430 |                  |
|                   | USD /  | JPY    |                  |



## Pip Value Calculation

$$
pipV = ( pointSize \div symbolPrice ) * lotSize
$$

- **pointSize** is how many decimal places has the price of a symbol. default is 0.0001 for major currency pairs.
- **symbolPrice** is the current symbol price. bid for selling, ask for buying.
- **lotSize** is the value for one standard lot. Default is 100.000 units.



## Profit Loss Calculation

$$
pnlV =( (abs(leftPx - rightPx) * 1 \div pointSize ) * ( qty * pointSize ) 
$$

- **abs** the absolute value of the substraction
- **leftPx** this is the left price, like entry price. check pipDiff.
- **rightPx** this is the right price, like current price. check pipDiff.
- **pointSize** is how many decimal places has the price of a symbol. default is 0.0001 for major currency pairs.



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

