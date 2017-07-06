#include <iostream>

namespace ui {

template <> constexpr char const * format_string<unsigned char>() { return "%hhu"; }
template <> constexpr char const * format_string<unsigned short>() { return "%hu"; }
template <> constexpr char const * format_string<unsigned int>() { return "%u"; }
template <> constexpr char const * format_string<unsigned long>() { return "%lu"; }
template <> constexpr char const * format_string<unsigned long long>() { return "%llu"; }

template <> constexpr char const * format_string<char>() { return "%hhd"; }
template <> constexpr char const * format_string<short>() { return "%hd"; }
template <> constexpr char const * format_string<int>() { return "%d"; }
template <> constexpr char const * format_string<long>() { return "%ld"; }
template <> constexpr char const * format_string<long long>() { return "%lld"; }

template <> constexpr char const * format_string<float>() { return "%f"; }

// --------------------------------------------------------------------

template <typename T>
bool InputNumber(std::string const & label, T& value, T min, T max,
	T step, ImGuiInputTextFlags extra_flags, char const * fmt) {
	char buf[64];
	std::sprintf(buf, fmt != nullptr ? fmt : format_string<T>(), value);
	
	ImGui::BeginGroup();
		ImGui::PushID(label.c_str());
		auto handled = ImGui::InputText(label.c_str(), buf, 64);
		if (handled) {
			std::sscanf(buf, fmt != nullptr ? fmt : format_string<T>(), &value);
		}
		
		if (step > 0) {
			ImGui::SameLine();
			if (ImGui::Button("-")) {
				value -= step;
				handled = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("+")) {
				value += step;
				handled = true;
			}
		}
		ImGui::PopID();
	ImGui::EndGroup();
	
	if (value < min) {
		value = min;
		handled = false;
	}
	if (value > max) {
		value = max;
		handled = false;
	}
	return handled;
}

template <typename T>
bool SliderNumber(std::string const & label, T& value, T min, T max, char const * fmt) {
	if (fmt == nullptr) {
		fmt = "%.0f";
	}
	float v = static_cast<float>(value);
	bool handled = ImGui::SliderFloat(label.c_str(), &v, static_cast<float>(min),
		static_cast<float>(max), fmt, 1.f);
	value = static_cast<T>(std::round(v));
	return handled;
}

// --------------------------------------------------------------------

template <typename T>
bool editInt(std::string const & key, T& value, std::string const & id) {
	ImGui::Text("%s", key.c_str()); ImGui::NextColumn();
	bool result = ui::InputNumber("##" + id, value); ImGui::NextColumn();
	ImGui::Separator();
	return result;
}

} // ::ui
