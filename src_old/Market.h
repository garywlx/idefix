#ifndef IDEFIX_MARKET_H
#define IDEFIX_MARKET_H

#include <string>
#include <vector>
#include "MarketSnapshot.h"

using namespace std;

namespace IDEFIX {
class Market {
private:
	std::string m_symbol;
	std::vector<MarketSnapshot> m_snapshots;

public:
	// Constructs a new Market without identifier
	explicit Market(){}
	// Constructs a new Market with empty snapshot list
	explicit Market(const string& symbol): m_symbol(symbol){}
	// Constructs a new Market with snapshot symbol and first entry
	explicit Market(const MarketSnapshot& snapshot): m_symbol(snapshot.getSymbol()) {
		add(snapshot);
	}
	inline ~Market(){}

	// Returns the symbol of this market
	inline string getSymbol() const {
		return m_symbol;
	}

	// Returns all snapshots as vector
	inline vector<MarketSnapshot> getSnapshots() {
		return m_snapshots;
	}

	// add new snapshot to the end of the list
	inline void add(const MarketSnapshot snapshot){
		m_snapshots.push_back(snapshot);
	}

	// Returns latest market snapshot
	inline MarketSnapshot getLatestSnapshot() const {
		auto snapshot = m_snapshots.back();
		return snapshot;
	}

	// Returns the size of the snapshot list
	inline int getSize() const {
		return (int)m_snapshots.size();
	}

	// Returns true if this is a valid market object
	inline bool isValid() const {
		return ! m_symbol.empty();
	}

	// Returns true if the snapshot list is empty
	inline bool isEmpty() const {
		return m_snapshots.empty();
	}

	/*!
	 * Returns a range of snapshots beginning from index from_start
	 *
	 * @param const int from_start  The from_start index
	 *
	 * @return std::vector<MarketSnapshot>
	 */
	inline std::vector<MarketSnapshot> getRange(const int from_start) {
		if ( isEmpty() ) {
			return m_snapshots;
		}

		auto list_size      = getSize();
		int list_from_start = from_start;
		std::vector<MarketSnapshot> result;

		// if from_start is negative, we use last element index - from_start
		// like size()-3 means [0,1,2,3,>>4<<,5,6] = 4
		if ( from_start < 0 ) {
			list_from_start = list_size - std::abs( from_start );
			// if from_start value lower then 0, return result because we are out of bounds
			if ( list_from_start < 0 ) {
				return result;
			}
		}

		// loop through array begining at from_start index
		for ( int i = list_from_start; i < list_size; i++ ) {
			result.push_back( m_snapshots.at( i ) );
		}

		return result;
	}

};
};

#endif