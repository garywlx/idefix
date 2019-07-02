#pragma once

#include <map>
#include <optional>
#include <variant>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <utility>

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
 * Add or update element in map
 *
 * @param const typename V& map 
 * @param const typename V::key_type& key
 * @param const typename V::mapped_type& value
 * @return bool
 */
// template <typename V>
// bool SetInMap(const V& map, const typename V::key_type& key, const typename V::mapped_type& value) {
// 	auto it = map.find( key );
// 	if ( it == map.end() ) {
// 		auto insert = map.emplace( key, value );
// 		return insert->second;
// 	} else {
// 		it->second = value;
// 		return true;
// 	}
// 	return false;
// }

/**
 * Erase element from map
 * 
 * @param const V& map 
 * @param const typename V::key_type& key
 * @return bool
 */
template <typename V> 
bool EraseInMap(const V& map, const typename V::key_type& key) {
	auto it = map.find( key );
	if ( it == map.end() ) return false;
	return ( map.erase( key ) == 1 );
}

/**
 * Find in set reference
 * 
 * @param  const V& set
 * @param  const typename V::value_type& key
 * @return const typename V::value_type
 */
template <typename V> 
const typename V::value_type& FindInSet(const V& set, const typename V::value_type& key) {
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
template <typename V> 
const typename V::value_type& FindInSet(std::shared_ptr<V> set, const typename V::value_type& key ) {
	static const typename V::value_type kValue{};
	if ( ! set ) return kValue;
	auto it = set->find( key );
	if ( it == set->end() ) return kValue;
	return *it;
}

/**
 * Find in vector
 * 
 * @param const V& vector 
 * @param const typename V::value_type& key
 * @return const typename V::value_type&
 */
template <typename V> 
const typename V::value_type& FindInVector(const V& vector, const typename V::value_key& key) {
	static const typename V::value_key kValue{};
	auto it = std::find( vector.begin(), vector.end(), key );
	if ( it == vector.end() ) return kValue;
	return *it;
}

};