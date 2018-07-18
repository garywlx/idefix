# Forex Formulas

## Bid/Ask

|                   | Bid    | Ask    |                  |
| ----------------- | ------ | ------ | ---------------- |
|                   | 95.620 | 95.650 | Open BUY use ASK |
| Open SELL use BID | 96.400 | 96.430 |                  |
|                   | USD /  | JPY    |                  |



## Pip Value Calculation

Die Pip-Wert Berechnung im Forex-Handel ist dahingehend erst einmal sehr einfach, denn der Pip-Wert entspricht der vierten Nachkommastelle: also bei EUR-USD wäre der Pip-Wert 0,0001 US-Dollar. Wenn wir jetzt z.B. unser Risiko in einem Forex-Trade berechnen möchten, dann müssen die Positionsgröße mit dem Pip-Wert und der Anzahl der riskierten Pips multiplizieren.  

Ein Beispiel wäre:

Wir kaufen **EUR/USD** bei 1,2830 und unser Stop-Loss liegt bei 1,2790. Unser Risiko beträgt also 40 Pips. Wenn wir ein sogenanntes Standard-Lot handeln, also 100.000 USD kaufen, dann beträgt unser Risiko 100.000 * 0,0001 * 40 = 400 USD. 
$$
RiskUSD = 100000 * 0.0001 * 40 = 400
$$
Mit anderen Worten: Beim Standard-Lot beträgt der Pip-Wert 10 USD. Da wir unser Konto in der Regel in Euro führen, müssen wir den Pip-Wert dann noch in Euro umrechnen. Aktuell wäre das: 10 USD * 0,7795= 7,795 Euro, wobei 0,7795 der Umrechnungskurs von 1 USD zu einem Euro ist.
$$
eurousd =1\div1.2830 = 0.7759\\
euro = 0.7759 * 10 = 7.795
$$
Noch ein Beispiel für das Währungspaar **AUD/NZD**: Wir kaufen bei einem Kurs von 1,1085 mit einem Stop Loss bei 1,1050. Unser Risiko beträgt also 35 Pips. Das Währungspaar AUD-NZD wird in NZD gehandelt, wobei AUD die sogenannte Basiswährung und der NZD die sogenannte Kurswährung ist. Wir handeln wieder ein Standard-Lot in Höhe von 100.000 NZD und möchten wissen, wie hoch unser Risiko ist. Die Rechnung wäre: 100.000 * 0,0001 * 35 = 350 NZD.  
$$
RiskNZD = 100000 * 0.0001 * 35 = 350
$$
Beim Standardlot bringt also ein Pip eine Wertveränderung von 10 NZD mit sich. Jetzt brauchen wir noch den Umrechnungsfaktor von NZD in EUR, der aktuell 0,6291 beträgt, womit wir ein Risiko in Höhe von 350 NZD * 0,6291 = 220,18 EUR auf uns nehmen würden.
$$
eurnzd = 1 \div 1.5895 = 0.6291\\
eur = 0.6291 * 350 = 220.18
$$




Formula:
$$
pipValue = ( onePip / exchangeRate ) * lotSize
$$


**Example:**

- exchange rate: 1.62550 EUR/USD
- lot size: 100000
- one pip: 0.0001

$$
exchangeRate=1.62550\\
lotSize=100000\\
onePip=0.0001\\
pipValue = ( onePip / exchangeRate ) * lotSize\\
pipValue = ( 0.0001 / 1.62550 ) * 100000 = 6,1519532452
$$

