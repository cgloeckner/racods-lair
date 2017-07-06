#include <SFML/System.hpp>

namespace sf {

std::string String::toAnsiString(const std::locale& locale) const
{
	// Prepare the output string
	std::string output;
	output.reserve(m_string.length() + 1);
	
	// Convert
	Utf32::toAnsi(m_string.begin(), m_string.end(), std::back_inserter(output), 0, locale);
	
	return output;
}

} // ::sf
