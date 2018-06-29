#ifndef IDEFIX_MARKET_H
#define IDEFIX_MARKET_H

#include <string>
#include <vector>
#include "types/MarketSnapshot.h"
#include <quickfix/Mutex.h>

using namespace std;

namespace IDEFIX {
class Market {
private:
	string m_symbol;
	unsigned int m_symbol_precision;
	double m_symbol_pointsize;
	vector<MarketSnapshot> m_snapshots;
	mutable FIX::Mutex m_mutex;

public:
	// Constructs a new Market without identifier
	explicit Market(){}
	// Constructs a new Market with empty snapshot list
	explicit Market(const string symbol): m_symbol(symbol){}
	// Constructs a new Market with snapshot symbol and first entry
	explicit Market(const MarketSnapshot& snapshot): m_symbol(snapshot.getSymbol()) {
		add(snapshot);
	}
	inline ~Market(){}

	// Returns the symbol of this market
	inline string getSymbol() const {
		FIX::Locker lock(m_mutex);
		return m_symbol;
	}

	// Returns all snapshots as vector
	inline vector<MarketSnapshot> getSnapshots() {
		FIX::Locker lock(m_mutex);
		return m_snapshots;
	}

	// add new snapshot to the end of the list
	inline void add(const MarketSnapshot snapshot){
		FIX::Locker lock(m_mutex);
		m_snapshots.push_back(snapshot);
	}

	// Returns latest market snapshot
	inline MarketSnapshot getLatestSnapshot() const {
		FIX::Locker lock(m_mutex);
		auto it = m_snapshots.end();
		return (*it);
	}

	// Returns the size of the snapshot list
	inline int getSize() const {
		FIX::Locker lock(m_mutex);
		return (int)m_snapshots.size();
	}

	// Returns true if this is a valid market object
	inline bool isValid() const {
		FIX::Locker lock(m_mutex);
		return ! m_symbol.empty();
	}
};
};

#endif