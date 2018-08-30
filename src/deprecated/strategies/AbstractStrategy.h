#ifndef IDEFIX_ABSTRACTSTRATEGY_H
#define IDEFIX_ABSTRACTSTRATEGY_H

namespace IDEFIX {
	class Chart;
	class MarketSnapshot;
	class Bar;

	struct AbstractStrategy {
		// is called on initialization
		virtual void on_init(Chart* chart)=0;
		// is called on chart tick
		virtual void on_tick(Chart* chart, const MarketSnapshot& tick)=0;
		// is called on chart bar
		virtual void on_bar(Chart* chart, const Bar& bar)=0;
		// is called on exit
		virtual void on_exit(Chart* chart)=0;
	};
};

#endif