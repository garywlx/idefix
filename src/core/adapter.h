#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include "utility.h"

namespace idefix { 
class Adapter {
public:
	virtual ~Adapter();
	typedef std::unordered_map<std::string, std::string> StrMap;
	const std::string& getName() const { return m_name; }
	const StrMap& getConfig() const { return m_config; }
	std::string getConfig(const std::string& key) const {
		return FindInMap(m_config, key);
	}
	void setName(const std::string& name) { m_name = name; }
	void setConfig(const StrMap& config) { m_config = config; }
	std::string getVersion() const {
		return "1";
	}

	typedef Adapter* (*CFunc)();
	typedef std::function<Adapter*()> Func;

	Adapter* clone() {
		auto inst = m_create_func();
		inst->setName( getName() );
		inst->setConfig( getConfig() );
		return inst;
	}

	static Adapter* load(const std::string& sofile);
	virtual void start() noexcept = 0;
protected:
	std::string m_name;
	StrMap m_config;
	Func m_create_func;
};

class NetworkAdapter: public Adapter {
public:
	virtual void connect() noexcept {}
	virtual void disconnect() noexcept {}
};

template <typename T>
class AdapterManager {
public:
	typedef std::unordered_map<std::string, T*> AdapterMap;
	void add(T* adapter) { m_adapters[ adapter->getName() ] = adapter; }
	T* getAdapter(const std::string& name) { return FindInMap( m_adapters, name ); }
	const AdapterMap& getAdapters() { return m_adapters; }

private:
	AdapterMap m_adapters;
};
};