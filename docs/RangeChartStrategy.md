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
strategy("IDEFIX", overlay=true, pyramiding=1, initial_capital=10000, currency=currency.EUR)

renko_tickerid = renko(tickerid, "close", "Traditional", 0.0003)
rclose = security(renko_tickerid, period, close)
ropen = security(renko_tickerid, period, open)
sma_fast = sma( ( open + high + low + close ) / 4, 5)
sma_slow = sma( ( open + high + low + close ) / 4, 10)

last_ma = sma_fast

plot(last_ma, title="SignalMA", color=#0000ff, linewidth=3)
plot(sma_slow, title="Slow SMA 10", color=#ff0000, linewidth=3)

brick_1_long = ropen[1] < rclose[1]
brick_long = ropen < rclose
brick_above_ma = ropen > last_ma and rclose > last_ma
brick_below_ma = ropen < last_ma and rclose < last_ma

// order conditions
ts = timestamp(syminfo.timezone, 2018, 8, 19, 0, 0)
// entry signal long
// brick_1 == SHORT
// brick   == LONG
// OR
// brick_1 == LONG
// brick   == LONG
// 
// brick > last_ma
enterLong = not brick_1_long and brick_long
if ( not enterLong )
    enterLong = brick_1_long and brick_long
    
if ( enterLong )
    enterLong = brick_above_ma
// exit signal long
// brick_1 == LONG
// brick   == SHORT
// brick.close <= last_ma
exitLong = false
if ( strategy.position_size > 0 )
    exitLong = brick_1_long and not brick_long
    
if ( exitLong )
    exitLong = rclose < last_ma
// entry signal short
// brick_1 == LONG
// brick   == SHORT
// OR
// brick_1 == SHORT
// brick   == SHORT
// 
// brick < last_ma
enterShort = brick_1_long and not brick_long
if ( not enterShort )
    enterShort = not brick_1_long and not brick_long
if ( enterShort )
    enterShort = brick_below_ma
// exit singal short
// brick_1 == SHORT
// brick   == LONG
// brick > last_ma
exitShort = false
if ( strategy.position_size > 0 )
    exitShort = not brick_1_long and brick_long
    
if ( exitShort )
    exitShort = rclose > last_ma

// risk management
strategy.risk.max_drawdown(25, strategy.percent_of_equity)

// buy
strategy.entry(id="buy", qty=100000, long=true, when=time > ts and enterLong )
strategy.close(id="buy", when= exitLong )
// sell
strategy.entry(id="sell", qty=100000, long=false, when=time > ts and enterShort )
strategy.close(id="sell", when= exitShort )

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