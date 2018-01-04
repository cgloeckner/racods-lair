#include <ui/imgui.hpp>

namespace ui {

bool vector_getter(void* vec, int index, char const ** out) {
	auto& vector = *reinterpret_cast<std::vector<std::string>*>(vec);
	if (index < 0 || index >= static_cast<int>(vector.size())) {
		return false;
	}
	*out = vector.at(index).c_str();
	return true;
}

bool Combo(std::string const & label, int& index, std::vector<std::string>& values) {
	return ImGui::Combo(label.c_str(), &index, &vector_getter,
		reinterpret_cast<void*>(&values), values.size());
}

bool ListBox(std::string const & label, int& index, std::vector<std::string>& values) {
	return ImGui::ListBox(label.c_str(), &index, &vector_getter,
		reinterpret_cast<void*>(&values), values.size());
}

bool InputText(std::string const & label, std::string& str, std::size_t max_size,
	ImGuiInputTextFlags flags, ImGuiTextEditCallback callback, void* user_data) {
	if (str.size() > max_size) {
		// too large for editing
		ImGui::Text("%s", str.c_str());
		return false;
	}
	
	std::string buffer{str};
	buffer.resize(max_size);
	bool changed = ImGui::InputText(label.c_str(), &buffer[0], max_size,
		flags, callback, user_data);
	if (changed) {
		auto i = buffer.find_first_of('\0');
		str = buffer.substr(0u, i);
	}
	return changed;
}

// --------------------------------------------------------------------

bool forwardStream(std::stringstream& stream, ImGuiTextBuffer& buffer) {
	auto data = stream.str();
	stream.str("");
	buffer.appendf("%s", data.c_str());
	return !data.empty();
}

// --------------------------------------------------------------------

template <>
bool SliderNumber(std::string const & label, float& value, float min, float max, char const * fmt) {
	return ImGui::SliderFloat(label.c_str(), &value, min, max, fmt, 1.f);
}

// I don't use them atm but its better to be "notified" ^^
template <>
bool SliderNumber(std::string const & label, unsigned long long& value, unsigned long long min, unsigned long long max, char const * display_format) {
	throw "SliderNumber<unsigned long long> not implemented yet\n";
}

template <>
bool SliderNumber(std::string const & label, long long& value, long long min, long long max, char const * display_format) {
	throw "SliderNumber<long long> not implemented yet\n";
}

template <>
bool SliderNumber(std::string const & label, double& value, double min, double max, char const * display_format) {
	throw "SliderNumber<double> not implemented yet\n";
}

template <>
bool SliderNumber(std::string const & label, long double& value, long double min, long double max, char const * display_format) {
	throw "SliderNumber<long double> not implemented yet\n";
}

// --------------------------------------------------------------------

void showPair(std::string const & key, std::string const & value) {
	ImGui::Text("%s", key.c_str()); ImGui::NextColumn();
	ImGui::TextWrapped("%s", value.c_str()); ImGui::NextColumn();
	ImGui::Separator();
}

bool editBool(std::string const & key, bool* ptr) {
	ImGui::Text("%s", key.c_str()); ImGui::NextColumn();
	bool result = ImGui::Checkbox(*ptr ? "true" : "false", ptr); ImGui::NextColumn();
	ImGui::Separator();
	return result;
}

bool editFloat(std::string const & key, float& value, float min, float max) {
	ImGui::Text("%s", key.c_str()); ImGui::NextColumn();
	bool result = ui::InputNumber("", value, min, max, 0.05f); ImGui::NextColumn();
	ImGui::Separator();
	return result;
}

bool editSelect(std::string const & key, int& index, std::vector<std::string>& data) {
	ImGui::Text("%s", key.c_str()); ImGui::NextColumn();
	bool result = ui::Combo("", index, data); ImGui::NextColumn();
	ImGui::Separator();
	return result;
}

bool editString(std::string const & key, std::string& data) {
	ImGui::Text("%s", key.c_str()); ImGui::NextColumn();
	bool result = ui::InputText("", data); ImGui::NextColumn();
	ImGui::Separator();
	if (data.empty()) {
		result = false;
	}
	return result;
}


} // ::ui
