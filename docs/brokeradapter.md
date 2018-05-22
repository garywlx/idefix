# Broker Adapter

The broker adapter is the abstract class to connect idefix to a price data stream. There are two adapter available. The FileAdapter and the FIXAdapter.

## FileAdapter

The FileAdapter reads historical price data from a file. The format should be .csv and the row format is:

```
DateTime,Bid,Ask
04/22/2018 21:02:52.801,1.22789,1.22782
04/22/2018 21:02:52.819,1.22789,1.22791
```

