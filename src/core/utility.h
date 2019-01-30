#pragma once

#include <boost/algorithm/string.hpp>
#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace idefix {

typedef std::unordered_map<std::string, std::string> UnorderedStrMap;

/**
 * Find in map reference
 * 
 * @param const V&                    map
 * @param const typename V::key_type& key
 * @return const typename V::mapped_type&
 */
template <typename V> 
const typename V::mapped_type& FindInMap(const V& map, const typename V::key_type& key) {
	static const typename V::mapped_type kValue{};
	auto it = map.find( key );
	if ( it == map.end() ) return kValue;

	return it->second;
}

/**
 * Find in map pointer
 * 
 * @param std::shared_ptr<V>          map 
 * @param const typename V::key_type& key
 * @return const typename V::mapped_type&
 */
template <typename V> 
const typename V::mapped_type& FindInMap(std::shared_ptr<V> map, const typename V::key_type& key) {
	static const typename V::mapped_type kValue{};
	if ( !map ) return kValue;
	auto it = map->find( key );
	if ( it == map->end() ) return kValue;
	return it->second;
}

/**
 * Find in set reference
 * 
 * @param  const V& set
 * @param  const typename V::value_type& key
 * @return const typename V::value_type
 */
template <typename V> const typename V::value_type& FindInSet(const V& set, const typename V::value_type& key) {
	static const typename V::value_type kValue{};
	auto it = set.find( key );
	if ( it == set.end() ) return kValue;
	return *it;
}

/**
 * Find in set pointer
 * 
 * @param std::shared_ptr<V> set
 * @param const typename V::value_type& key
 * @return const typename V::value_type
 */
template <typename V> const typename V::value_type& FindInSet(std::shared_ptr<V> set, const typename V::value_type& key ) {
	static const typename V::value_type kValue{};
	if ( ! set ) return kValue;
	auto it = set->find( key );
	if ( it == set->end() ) return kValue;
	return *it;
}

};