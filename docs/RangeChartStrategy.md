# IDEFIX Range Chart Strategie

## Symbols & Trading Times

Die Symbole werden nur innerhalb der Start und End Zeit gehandelt. In dieser Zeit ist die Volatilität am höchsten.

| Symbol  | Start | End  |
| ------- | ----- | ---- |
| AUD/USD |       |      |
| EUR/USD |       |      |
| GBP/USD |       |      |
| NZD/USD |       |      |
| USD/CAD |       |      |
| USD/CHF |       |      |
| USD/JPY |       |      |



## Struktur Range Bar

Range: 3 Pips 

Variables: Open, Close, OpenTime, CloseTime, TickVolume, isUP



## Struktur EMA

Length: 5

Source: Close Price

Offset: -3



## Entry/Exit UP

Entry: Wenn Spread < 1 und vorhergehende Kerze UP und Preis unterhalb von EMA ist, dann kaufen. 

Exit: Wenn vorhergehende Kerze UP ist und aktuelle DOWN schließt oder Preis oberhalb von EMA ist.

 

## Entry/Exit DOWN

Entry: Wenn Spread < 1 und vorhergehende Kerze DOWN und Preis oberhalb von EMA ist, dann verkaufen.

Exit: Wenn vorhergehende Kerze DOWN ist und aktuelle UP schließt oder Preis unterhalb von EMA ist.



## Position Size

Maximal 1% der Equity im Gesamten. Die Position Size wird durch 5 geteilt, so dass in einer Bewegung 4 weitere Trades per Scale In aufgestockt werden können.



## Scale In

Es werden weitere Positionen in die selbe Richtung geöffnet, wenn neue Einstiegssignale generiert werden. Wird ein Gegensignal generiert, werden alle vorhergehenden offenen Positionen in diesem Asset geschlossen bevor in die Gegenrichtung gehandelt wird. 



## Losses / Recovery Strategy

Wenn der Markt 3 Pips gegen die Position läuft, wird eine Hedging Position eröffnet. So wie hier erklärt https://youtu.be/DJz4E7VyeSw "Recovery Zone". Alle Trades werden in Profit oder Break Even geschlossen.

Ein Stop Loss wird nicht verwendet!!!