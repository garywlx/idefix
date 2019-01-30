#pragma once

#include <string>
#include <functional>

#include "datacontext.h"
#include "ordercontext.h"

namespace idefix {
class Algo {
public:
	virtual ~Algo() {};

	typedef Algo* (*CFunc)();
	typedef std::function<Algo*()> Func;

	Algo* clone() {
		auto inst = m_create_func();
		inst->setConfigFile( getConfigFile() );
		return inst;
	}

	static Algo* load(const std::string& sofile);

	void setConfigFile(const std::string& filepath) {
		m_config_file = filepath;
	}

	std::string getConfigFile() {
		return m_config_file;
	}

	// is called to identify this algorithm
	virtual std::string name() const noexcept { return ""; };
	// is called on initializing the algo
	virtual void initialize(DataContext& ctx) {};
	// is called when a new tick is available
	virtual void onTick(DataContext& ctx, std::shared_ptr<Instrument> instrument) {};

private:
	Func m_create_func;
	std::string m_config_file;
};
};

