#include <fstream>
#include <utils/binary_utils.hpp>

namespace utils {

sf::Packet loadBinaryFile(std::string const & filename) {
	std::ifstream file{filename, std::ifstream::binary};
	// workaround: load entire file to buffer
	std::vector<char> buffer;
	file.seekg(0, file.end);
	buffer.resize(file.tellg());
	file.seekg(0, file.beg);
	file.read(buffer.data(), buffer.size());
	// populate packet
	sf::Packet packet;
	packet.append(buffer.data(), buffer.size());
	return packet;
}

void saveBinaryFile(sf::Packet const & stream, std::string const & filename) {
	std::ofstream file{filename, std::ofstream::binary};
	file.write(reinterpret_cast<char const *>(stream.getData()), stream.getDataSize());
}

} // ::utils
