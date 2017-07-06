#include <utils/filesystem.hpp>
#include <utils/verifier.hpp>
#include <core/movement.hpp>
#include <core/focus.hpp>
#include <core/collision.hpp>

namespace game {

template <typename T>
void Mod::preload(bool force) {
	auto path = get_path<T>();
	auto ext = Mod::get_ext<T>();
	//log.debug << "  " << path << "\n";
	utils::for_each_file(path, ext, [&](std::string const & p, std::string const& key) {
		// preload resource
		auto full = mod_impl::concat(p, key);
		//log.debug << "      " << full << "\n";
		get<T>(full, force);
	});
}

template <typename T>
void Mod::preload_and_prepare(bool force) {
	auto path = get_path<T>();
	auto ext = Mod::get_ext<T>();
	//log.debug << "  " << path << "\n";
	utils::for_each_file(path, ext, [&](std::string const & p, std::string const& key) {
		// preload resource
		auto full = mod_impl::concat(p, key);
		//log.debug << "      " << full << "\n";
		auto& resource = query<T>(full, force);
		
		// prepare resource
		resource.internal_name = full;
		prepare(resource);
	});
}

template <typename T>
bool Mod::verify(utils::Logger& log) {
	auto path = get_path<T>();
	auto ext = Mod::get_ext<T>();
	bool result = true;
	log << "     " << path << "\n";
	utils::for_each_file(path, ext, [&](std::string const & p, std::string const& key) {
		auto full = mod_impl::concat(p, key);
		log << "        " << full << "\n";
		result &= mod_impl::verify(log, full, get<T>(full));
	});
	return result;
}

// --------------------------------------------------------------------

template <>
inline std::string Mod::get_path<sf::Texture>() const {
	return name + "/gfx";
}

template <>
inline std::string Mod::get_path<sf::SoundBuffer>() const {
	return name + "/sfx";
}

template <>
inline std::string Mod::get_path<sf::Music>() const {
	return name + "/music";
}

template <>
inline std::string Mod::get_path<sf::Font>() const {
	return name + "/font";
}

template <>
inline std::string Mod::get_path<game::AiScript>() const {
	return name + "/lua";
}

template <>
inline std::string Mod::get_path<rpg::TilesetTemplate>() const {
	return name + "/xml/tileset";
}

template <>
inline std::string Mod::get_path<rpg::SpriteTemplate>() const {
	return name + "/xml/sprite";
}

template <>
inline std::string Mod::get_path<rpg::EntityTemplate>() const {
	return name + "/xml/entity";
}

template <>
inline std::string Mod::get_path<rpg::EffectTemplate>() const {
	return name + "/xml/effect";
}

template <>
inline std::string Mod::get_path<rpg::BulletTemplate>() const {
	return name + "/xml/bullet";
}

template <>
inline std::string Mod::get_path<rpg::ItemTemplate>() const {
	return name + "/xml/item";
}

template <>
inline std::string Mod::get_path<rpg::PerkTemplate>() const {
	return name + "/xml/perk";
}

template <>
inline std::string Mod::get_path<rpg::TrapTemplate>() const {
	return name + "/xml/trap";
}

template <>
inline std::string Mod::get_path<game::BotTemplate>() const {
	return name + "/xml/bot";
}

template <>
inline std::string Mod::get_path<game::EncounterTemplate>() const {
	return name + "/xml/encounter";
}

template <>
inline std::string Mod::get_path<game::RoomTemplate>() const {
	return name + "/xml/room";
}

// --------------------------------------------------------------------

template <>
inline std::string Mod::get_ext<sf::Texture>() {
	return ".png";
}

template <>
inline std::string Mod::get_ext<sf::Font>() {
	return ".ttf";
}

template <>
inline std::string Mod::get_ext<sf::SoundBuffer>() {
	return ".ogg";
}

template <>
inline std::string Mod::get_ext<sf::Music>() {
	return ".ogg";
}

template <>
inline std::string Mod::get_ext<game::AiScript>() {
	return ".lua";
}

template <typename T>
inline std::string Mod::get_ext() {
	return ".xml";
}

// --------------------------------------------------------------------

template <typename T>
inline std::string Mod::get_filename(std::string const& fname) const {
	return get_path<T>() + "/" + fname + get_ext<T>();
}

template <typename T>
inline T const& Mod::get(std::string const& resource_key, bool reload) {
	auto fname = get_filename<T>(resource_key);
	return cache.get<T>(fname, reload);
}

template <typename T>
T& Mod::query(std::string const & resource_key, bool reload) {
	auto fname = get_filename<T>(resource_key);
	return cache.get<T>(fname, reload);
}

template <typename T>
std::vector<T const *> Mod::getAll() {
	std::vector<T const *> out;
	auto path = get_path<T>();
	auto ext = Mod::get_ext<T>();
	utils::for_each_file(path, ext, [&](std::string const & p, std::string const& key) {
		// preload, prepare and yield resource
		auto full = mod_impl::concat(p, key);
		auto& res = query<T>(full);
		if (res.internal_name != full) {
			res.internal_name = full;
			prepare(res);
		}
		out.push_back(&res);
	});
	return out;
}

template <typename T>
void Mod::getAllFiles(std::vector<std::string>& files) const {
	files.clear();
	auto path = get_path<T>();
	auto ext = Mod::get_ext<T>();
	utils::for_each_file(path, ext, [&](std::string const & p, std::string const& key) {
		files.push_back(key);
	});
}

}  // ::game
