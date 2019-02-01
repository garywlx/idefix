#pragma once

#include "core/algo.h"

/**
 * Tick70 Algorithm
 */

namespace idefix {
class Tick70: public virtual Algo {
public: 
	Tick70();

	std::string name() const noexcept;
	void initialize(DataContext& ctx);

	// is called when a new tick is available
	void onTick(DataContext& ctx, std::shared_ptr<Instrument> instrument);
};
};