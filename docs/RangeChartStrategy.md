# IDEFIX Range Chart Strategie

## Symbols & Trading Times

Die Symbole werden nur innerhalb der Start und End Zeit gehandelt. In dieser Zeit ist die Volatilität am höchsten. Die Tabelle stellt die Zeiten in GMT dar.

| Symbol  | Start | End   | Session     |
| ------- | ----- | ----- | ----------- |
| AUD/USD | 22:00 | 07:00 | Asian       |
| EUR/USD | 08:00 | 22:00 | European/US |
| GBP/USD | 08:00 | 22:00 | European/US |
| NZD/USD | 22:00 | 07:00 | Asian       |
| USD/CAD | 08:00 | 22:00 | European/US |
| USD/CHF | 08:00 | 22:00 | European/US |
| USD/JPY | 0:00  | 09:00 | Asian       |

Die Volatilität kann bei der **Zeitüberlagerung** am höher ausfallen.

| Symbol  | Start | End   | Session     |
| ------- | ----- | ----- | ----------- |
| AUD/USD | 0:00  | 07:00 | Asian       |
| EUR/USD | 13:00 | 17:00 | European/US |
| GBP/USD | 13:00 | 17:00 | European/US |
| NZD/USD | 0:00  | 7:00  | Asian       |
| USD/CAD | 13:00 | 17:00 | European/US |
| USD/CHF | 13:00 | 17:00 | European/US |
| USD/JPY | 13:00 | 17:00 | European/US |

## TradingView Pine Script

```pine

//@version=3
strategy("RenkoSMA", overlay=true, pyramiding=1, initial_capital=10000)

risk = input(title="risk", type=integer,defval=1,minval=1,maxval=100)

renko_tickerid = renko(tickerid, "close", "ATR", 3)
rclose = security(renko_tickerid, period, close)
signalMA = sma(close, 5)

// order conditions
ts = timestamp(syminfo.timezone, 2018, 6, 1, 0, 0)
enterLong = time > ts and rclose[1] < rclose and rclose > signalMA
exitLong = strategy.position_size > 0 and rclose[1] > rclose
enterShort = time > ts and rclose[1] > rclose and rclose < signalMA
exitShort = strategy.position_size > 0 and rclose[1] < rclose

// risk management
strategy.risk.max_drawdown(25, strategy.percent_of_equity)

// buy
strategy.entry(id="buy", qty=100000, long=true, when= enterLong )
strategy.close(id="buy", when= exitLong )
// sell
strategy.entry(id="sell", qty=100000, long=false, when= enterShort )
strategy.close(id="sell", when= exitShort )

plot(signalMA, title="SignalMA", color=#0000ff, linewidth=5)

// exit market
if ( hour == 22 )
    strategy.close_all()

```

## Renko Indicator
Range: ATR(3), close

## Signal SMA
Length: 3
Source: Renko Close Price

## Entry/Exit BUY
Entry: If spread < 1 and renkoclose(1) < renkoclose(0) and renkoclose(0) > signalMA
Exit: If positions > 0 and renkoclose(1) > renkoclose(0)

## Entry/Exit SELL
Entry: If spread < 1 and renkoclose(1) > renkoclose(0) and renkoclose(0) < signalMA
Exit: If positions > 0 and renkoclose(1) < renkoclose(0)

## Position Size
Maximal 1% der Equity im Gesamten. 

## Losses / Recovery Strategy
Wenn der Markt 1 RenkoRange gegen die Position läuft, wird eine Hedging Position mit der 1,2 fachen Positionsgröße der Negativposition eröffnet. So wie hier erklärt https://youtu.be/DJz4E7VyeSw "Recovery Zone". Alle Trades werden in Profit oder Break Even geschlossen.

Ein Stop Loss wird nicht verwendet!!!