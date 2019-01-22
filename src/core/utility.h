#pragma once

#include <boost/algorithm/string.hpp>
#include <map>
#include <optional>
#include <variant>
#include <vector>

namespace idefix {

template <typename V>
const typename V::mapped_type& FindInMap(const V& map, const typename V::key_type& key) {
	static const typename V::mapped_type kValue{};
	auto it = map.find( key );
	if ( it == map.end() ) return kValue;

	return it->second;
}

template <typename V>
const typename V::mapped_type& FindInMap(std::shared_ptr<V> map, const typename V::key_type& key) {
	static const typename V::mapped_type kValue{};
	if ( !map ) return kValue;
	auto it = map->find( key );
	if ( it == map->end() ) return kValue;
	return it->second;
}

};