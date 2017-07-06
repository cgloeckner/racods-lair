#pragma once
#include <functional>
#include <string>
#include <ctime>

namespace utils {

using BaseFileExtDirHandle = std::function<void(std::string const & base, std::string const & file, std::string const & ext, bool is_dir)>;
using BaseFileHandle = std::function<void(std::string const & base, std::string const & file)>;

bool file_exists(std::string const & path);

void create_dir(std::string const & path);
void rename_file(std::string const & o, std::string const & n);
void remove_file(std::string const & path);

std::time_t get_last_changed_date(std::string const & p);

void for_each_file(std::string const & path, BaseFileExtDirHandle handle);
void for_each_file(std::string const & path, std::string const & ext, BaseFileHandle handle);

} // ::utils
