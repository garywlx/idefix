#include "SimpleMovingAverage.h"

#include <iostream>
using namespace std;

namespace IDEFIX {
	SimpleMovingAverage::SimpleMovingAverage() {
		// set default to 5
		m_period = 5;
		clear();
	}

	SimpleMovingAverage::SimpleMovingAverage(const int period) {
		m_period = period;
		clear();
	}

	SimpleMovingAverage::~SimpleMovingAverage() {

	}

	double SimpleMovingAverage::value() {
		return m_total / m_period;
	}

	void SimpleMovingAverage::add(const double value) {
		// substract the oldest number from total
		m_total -= m_values.at( 0 );
		// add the new number to total
		m_total += value;
		// remove oldest number
		m_values.erase( m_values.begin() );	
		// add new number to the end
		m_values.push_back( value );
	}

	void SimpleMovingAverage::clear() {
		m_values.clear();
		
		// init array
		for( int i = 0; i < m_period; i++ ) {
			m_values.push_back( 0 );
		}

		m_total = 0;
	}

};