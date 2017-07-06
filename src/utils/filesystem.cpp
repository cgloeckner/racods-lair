#include <boost/filesystem.hpp>

#include <utils/filesystem.hpp>

namespace utils {

bool file_exists(std::string const & path) {
	return boost::filesystem::exists(path);
}

void create_dir(std::string const & path) {
	boost::filesystem::create_directory(path);
}

void rename_file(std::string const & o, std::string const & n) {
	boost::filesystem::rename(o, n);
}

void remove_file(std::string const & path) {
	boost::filesystem::remove(path);
}

std::time_t get_last_changed_date(std::string const & p) {
	return boost::filesystem::last_write_time(p);
}

void for_each_file(std::string const & path, BaseFileExtDirHandle handle) {
	boost::filesystem::path p{path};
	boost::filesystem::recursive_directory_iterator it{p};
	boost::filesystem::recursive_directory_iterator end;
	while (it != end) {
		auto const & tmp = it->path();
		auto base = tmp.parent_path().string();
		auto pos = base.find(path);
		base = base.substr(pos+path.size());
		auto fname = tmp.stem().string();
		auto ext = tmp.extension().string();
		auto dir = boost::filesystem::is_directory(*it);
		handle(base, fname, ext, dir);
		++it;
	}
}

void for_each_file(std::string const & path, std::string const & ext, BaseFileHandle handle) {
	for_each_file(path, [&](std::string const & base, std::string const & file, std::string const & _ext, bool is_dir) {
		if (!is_dir && _ext == ext) {
			handle(base, file);
		}
	});
}

} // ::utils
