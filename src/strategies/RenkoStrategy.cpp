#include "RenkoStrategy.h"
#include "../Chart.h"
#include "../indicator/Tick.h"
#include "../indicator/Bar.h"
#include "../Console.h"

#include <iostream>
#include <sstream>
using namespace std;

namespace IDEFIX {

RenkoStrategy::RenkoStrategy() {}

// is called on initialization
void RenkoStrategy::on_init(Chart& chart) {
	console()->info("[RenkoStrategy:{}] {}", chart.symbol(), __FUNCTION__ );
}

// is called on chart tick
void RenkoStrategy::on_tick(Chart& chart, const Tick& tick) {
	std::stringstream ss;
	ss << tick;
	//console()->info("[RenkoStrategy:{}] {} {}", chart.symbol(), __FUNCTION__, ss.str() );
}

// is called on chart bar
void RenkoStrategy::on_bar(Chart& chart, const Bar& bar) {
	std::stringstream ss;
	ss << bar;
	console()->info("[RenkoStrategy:{}] {} {}", chart.symbol(), __FUNCTION__, ss.str() );
}

// is called on exit
void RenkoStrategy::on_exit(Chart& chart) {
	console()->info("[RenkoStrategy:{}] {}", chart.symbol(), __FUNCTION__ );
}


};