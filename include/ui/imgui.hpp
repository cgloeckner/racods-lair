#pragma once
#include <vector>
#include <string>
#include <limits>
#include <sstream>

#include <imgui.h>
#include <imgui-SFML.h>

#define IM_COLOR(sfColor) IM_COL32(sfColor.r, sfColor.g, sfColor.b, sfColor.a)

namespace ui {

bool vector_getter(void* vec, int index, char const ** out);

bool Combo(std::string const & label, int& index, std::vector<std::string>& values);

bool ListBox(std::string const & label, int& index, std::vector<std::string>& values);

bool InputText(std::string const & label, std::string& str, std::size_t max_size=255,
	ImGuiInputTextFlags flags=0, ImGuiTextEditCallback callback=nullptr,
	void* user_data=nullptr);

// --------------------------------------------------------------------

bool forwardStream(std::stringstream& stream, ImGuiTextBuffer& buffer);

// --------------------------------------------------------------------

template <typename T>
constexpr char const * format_string();

template <typename T>
bool InputNumber(std::string const & label, T& value,
	T min=std::numeric_limits<T>::min(), T max=std::numeric_limits<T>::max(),
	T step=T{1}, ImGuiInputTextFlags extra_flags=0, char const * fmt=nullptr);

template <typename T>
bool SliderNumber(std::string const & label, T& value, T min, T max,
	char const * fmt=nullptr);

// --------------------------------------------------------------------

void showPair(std::string const & key, std::string const & value);

void showSprite(std::string const & key, sf::Sprite const & sprite);

bool editBool(std::string const & key, bool& ptr);

template <typename T>
bool editInt(std::string const & key, T& value, std::string const & id="");

bool editFloat(std::string const & key, float& value, float min, float max);

bool editSelect(std::string const & key, int& index, std::vector<std::string>& data);

bool editString(std::string const & key, std::string& data);

} // ::ui

// include implementation details
#include <ui/imgui.inl>
