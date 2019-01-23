#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include "utility.h"

namespace idefix { 
class Adapter {
public:
	virtual ~Adapter() = default;
	typedef Adapter* (*CFunc)();
	typedef std::function<Adapter*()> Func;

	Adapter* clone() {
		auto inst = m_create_func();
		inst->setConfigFile( getConfigFile() );
		return inst;
	}

	static Adapter* load(const std::string& sofile);

	/**
	 * Set config file path
	 * 
	 * @param const std::string& filepath
	 */
	void setConfigFile(const std::string& filepath) {
		m_config_file = filepath;
	}

	/**
	 * Get config file path
	 * 
	 * @return std::string
	 */
	std::string getConfigFile() {
		return m_config_file;
	}

protected:
	Func m_create_func;
	std::string m_config_file;
};

/**
 * Base Class for NetworkAdapter
 */
class NetworkAdapter: public Adapter {
public:
	virtual void connect() noexcept {}
	virtual void disconnect() noexcept {}
};

};