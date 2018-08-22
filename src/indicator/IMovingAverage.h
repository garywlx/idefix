#ifndef IDEFIX_IMOVING_AVERAGE_H
#define IDEFIX_IMOVING_AVERAGE_H

#include <vector>
#include <sstream>

namespace IDEFIX {
class IMovingAverage {
public:
	std::vector<double> m_values;
	int m_period;
	double m_total;

	virtual double value()=0;
	virtual void add(const double value)=0;
	virtual void clear()=0;
	virtual void set_period(const int period) {
		if ( m_period != period ) {
			m_period = period;
		}
	}
};
};

inline std::ostream& operator<<(std::ostream& out, IDEFIX::IMovingAverage& ma) {
	out << ma.value();
	return out;
}

#endif