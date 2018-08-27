#include "RenkoStrategy.h"
#include "../Chart.h"
#include "../indicator/Tick.h"
#include "../indicator/Bar.h"

#include <iostream>
using namespace std;

namespace IDEFIX {

RenkoStrategy::RenkoStrategy() {

}

// is called on initialization
void RenkoStrategy::on_init(Chart& chart) {
	cout << "RenkoStrategy->" << __FUNCTION__ << endl;
}

// is called on chart tick
void RenkoStrategy::on_tick(Chart& chart, const Tick& tick) {
	
}

// is called on chart bar
void RenkoStrategy::on_bar(Chart& chart, const Bar& bar) {
	cout << "RenkoStrategy->" << __FUNCTION__ << " new bar" << endl;
}

// is called on exit
void RenkoStrategy::on_exit(Chart& chart) {
	cout << "RenkoStrategy->" << __FUNCTION__ << endl;
}


};