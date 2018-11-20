# Lot calculation for stoploss in pips

## EURUSD

#### Based on symbol

tickvalue = 0,09

ticksize = 0,00001

pointsize = 0,00001

spread = 1,2

#### Manual changes

percent_to_risk = 5

free_margin = 49758,69

slpips = 10

#### calc

cbtickval = tickvalue * pointsize / ticksize = 0,09 * 0,00001 / 0,00001 = 0,09

riskmoney = percent_to_risk/100 * free_margin = 5/100 * 49758,69 = 2487,93

lot = riskmoney / ( slpips + spread ) * cbtickval = 2487,93 / ( 10 + 1,2 ) * 0,09 = 19,99

