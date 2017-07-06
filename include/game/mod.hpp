#pragma once
#include <string>
#include <list>
#include <SFML/Graphics.hpp>

#include <game/resources.hpp>

namespace game {

class Mod;

namespace mod_impl {

std::string concat(std::string const & lhs, std::string const & rhs);

template <typename T>
bool verify(utils::Logger& log, std::string const& key, T const& resource);

} // ::mod_impl

// --------------------------------------------------------------------

/// Mod API
/**
 *	A mod is determined by its base directory, which is used for
 *	construction. After that, resources can be preloaded and the entire
 *	mod can be checked for verification. This is useful to detect
 *	missing or broken files.
 *	Each resource that is queried, is identified by its type and the
 *	filename (without path prefix and extension suffix). All resources
 *	are supposed to be located at the hierarchy within the given mod
 *	directory.
 */
class Mod {
  private:
	core::LogContext& log;
	ResourceCache& cache;
	std::vector<std::string> processed_tilesets;
	
	// all instances of lua scripts
	std::list<AiScript> scripts;
	
	template <typename T>
	void preload(bool force);

	template <typename T>
	void preload_and_prepare(bool force);
	
	template <typename T>
	bool verify(utils::Logger& log);
	
  public:
	/// Path to the mod's parent directory
	std::string const name;

	/// Create a new mod
	/**
	 *	The given resource cache is assigned to the mod. So
	 *	multiple can share one resource cache. The cache's lifetime
	 *	is finally up to the application.
	 *
	 *	@param cache Reference to ResourceCache which will be used
	 *	@param name Path to the mod's parent directory
	 */
	Mod(core::LogContext& log, ResourceCache& cache, std::string const & name);

	/// Will preload all resources of the mod
	/**
	 *	This will preload all files of each supported resource
	 *	type.
	 *	@param force Whether already loaded resourced are force reloaded
	 *	
	 *	@throw [tba: see get()]
	 */
	void preload(bool force=false);
	
	template <typename T>
	void prepare(T& resource);
	
	/// Verify the mod
	/**
	 *	This verifies the current mod. If the mod is broken, the
	 *	provided log is used to print out errors inside the debug
	 *	channel. The final state (whether valid or broken) is
	 *	finally returned.
	/// @param log Logger instance for verification output
	 *	@return true if mod is valid, false if not
	 */
	bool verify(utils::Logger& log);

	/// Query given resource type's search path
	/**
	 *	@return search path of the given resource's type
	 */
	template <typename T>
	std::string get_path() const;

	/// Query given resource type's file extension
	/**
	 *	@return file extension of the given resource's type
	 */
	template <typename T>
	static std::string get_ext();

	/// Query given resource type's concrete filename
	/**
	 *	This will create the actual filename for the provided
	 *	resource. This will include the resource's name, as well as
	 *	the search path and file extension.
	 *
	 *	@param fname Name of the resource
	 *	@return full filename of the resource
	 */
	template <typename T>
	std::string get_filename(std::string const& fname) const;

	/// Query a const resource
	/**
	 *	If the desired resource was already loaded to the cache,
	 *	it will be returned after a cache lookup. Otherwise it will
	 *	be loaded from disk using the resource's corresponding
	 *	filename. If `reload` is enabled, the cache will be ignored
	 *	and the resource is always loaded. In this case, the
	 *	existing resource is reloaded but not reconstructed. So all
	 *	previous references to this resource will stay valid and
	 *	refer to the updated resource.
	 *
	 *	@param resource_key resource name without path and extension
	 *	@param reload|false (optional) determine force reload resource
	 */
	template <typename T>
	T const& get(std::string const& resource_key, bool reload = false);
	
	/// Query a non-const resource
	/// This returns a non-const reference to an existing resource
	/// If the desired resource was already loaded to the cache,
	/// it will be returned after a cache lookup. Otherwise it will
	/// be loaded from disk using the resource's corresponding
	/// filename. If `reload` is enabled, the cache will be ignored
	/// and the resource is always loaded. In this case, the
	/// existing resource is reloaded but not reconstructed. So all
	/// previous references to this resource will stay valid and
	/// refer to the updated resource.
	/// @param resource_key resource name without path and extension
	/// @param force|false (optional) determine force reload resource
	template <typename T>
	T& query(std::string const & resource_key, bool force=false);
	
	AiScript& createScript(std::string const & fname);
	std::list<AiScript>& getAllScripts();
	
	template <typename T>
	std::vector<T const *> getAll();
	
	template <typename T>
	void getAllFiles(std::vector<std::string>& files) const;
	
	std::vector<sf::Texture const *> getAllAmbiences();
};

} // ::game

// include implementation details
#include <game/mod.inl>
