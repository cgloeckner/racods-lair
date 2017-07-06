#include <vector>
#include <utils/assert.hpp>

#include <ui/systemgraph.hpp>

namespace ui {

SystemGraph::SystemGraph()
	: sf::Drawable{}
	, sf::Transformable{}
	, interval{sf::seconds(1.f)}
	, num_records{5}
	, max_value{10}
	, size{250u, 200u}
	, passed{sf::Time::Zero}
	, background{{250.f, 200.f}}
	, systems{} {
	background.setFillColor(sf::Color::Black);
}

void SystemGraph::setInterval(sf::Time const & interval) {
	this->interval = interval;
}

void SystemGraph::setNumRecords(std::size_t num_records) {
	this->num_records = num_records;
}

void SystemGraph::setMaxValue(std::size_t max_value) {
	this->max_value = max_value;
}

void SystemGraph::setSize(sf::Vector2u const & size) {
	this->size = size;
	background.setSize(sf::Vector2f{size});
}

void SystemGraph::setFillColor(sf::Color const & color) {
	background.setFillColor(color);
}

void SystemGraph::draw(
	sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	// draw background
	target.draw(background, states);
	// draw lines
	sf::VertexArray buffer{sf::LinesStrip};
	for (auto const& pair : systems) {
		sf::Vector2f pos;
		pos.x = (num_records - pair.second.values.size()) *
				(1.f * size.x / num_records);
		for (auto const& value : pair.second.values) {
			// calculate position
			auto scaled_value = 1.f * size.y * value / max_value;
			pos.y = size.y - scaled_value;
			// apply to vertex
			sf::Vertex vertex;
			vertex.position = pos;
			vertex.color = pair.second.color;
			buffer.append(vertex);
			// prepare next vertex
			pos.x += 1.f * size.x / num_records;
			pos.y = 0.f;
		}
		target.draw(buffer, states);
		buffer.clear();
	}
}

void SystemGraph::init(std::string const& name, sf::Color const& color) {
	systems[name].color = color;
}

std::size_t& SystemGraph::operator[](std::string const& name) {
	return systems[name].pending;
}

void SystemGraph::update(sf::Time const& elapsed) {
	passed += elapsed;
	if (passed >= interval) {
		passed -= interval;
		for (auto& pair : systems) {
			pair.second.values.push_back(pair.second.pending);
			pair.second.pending = 0u;
			if (pair.second.values.size() > num_records) {
				pair.second.values.pop_front();
			}
		}
	}
}

sf::Vector2u SystemGraph::getSize() const {
	return size;
}

LineList SystemGraph::getLines(std::string const & system) const {
	LineList lines;
	lines.reserve(num_records);
	
	// draw lines
	sf::VertexArray buffer{sf::LinesStrip};
	auto const & node = systems.find(system)->second;
	sf::Vector2f pos;
	pos.x = (num_records - node.values.size()) *
			(1.f * size.x / num_records);
	for (auto const& value: node.values) {
		// calculate position
		auto scaled_value = 1.f * size.y * value / max_value;
		pos.y = size.y - scaled_value;
		// apply to vertex
		sf::Vertex vertex;
		vertex.position = pos;
		vertex.color = node.color;
		buffer.append(vertex);
		// prepare next vertex
		pos.x += 1.f * size.x / num_records;
		pos.y = 0.f;
	}
	if (buffer.getVertexCount() >= 2u) {
		// pack to lines
		for (auto i = 0u; i < buffer.getVertexCount() - 1u; ++i) {
			lines.emplace_back(buffer[i].position, buffer[i+1].position);
		}
	}
	
	return lines;
}

SystemGraph::const_iterator SystemGraph::begin() const {
	return systems.begin();
}

SystemGraph::const_iterator SystemGraph::end() const {
	return systems.end();
}

}  // ::ui
