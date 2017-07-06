#pragma once
#include <string>
#include <Thor/Resources.hpp>

namespace utils {

template <typename T>
class SingleResourceCache {
  private:
	thor::ResourceHolder<T, std::string> holder;

  public:
	bool has(std::string const & filename) const;
	T& get(std::string const& filename, bool reload = false);
};

// ---------------------------------------------------------------------------

template <typename... Tail>
class MultiResourceCacheImpl;

// inherit from multiple resource caches
template <typename Head, typename... Tail>
class MultiResourceCacheImpl<Head, Tail...>
	: public SingleResourceCache<Head>,
	  public MultiResourceCacheImpl<Tail...> {};

template <>
class MultiResourceCacheImpl<> {};

// ---------------------------------------------------------------------------

template <typename... Resources>
class MultiResourceCache : protected MultiResourceCacheImpl<Resources...> {
  public:
	template <typename T>
	bool has(std::string const & filename) const;
	
	template <typename T>
	T& get(std::string const& filename, bool reload = false);
};

}  // ::utils

// include implementation details
#include <utils/resource_cache.inl>
