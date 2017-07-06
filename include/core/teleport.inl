namespace core {

template <typename Pred>
bool getFreePosition(Pred pred, sf::Vector2u& pos, std::size_t max_drift) {
	// seek suitable position near the initial position
	sf::Vector2u tmp;
	for (int drift = 0; drift <= max_drift; ++drift) {
		sf::Vector2i dir;
		if (drift > 0) {
			for (dir.y = -drift; dir.y <= drift; ++dir.y) {
				for (dir.x = -drift; dir.x <= drift; ++dir.x) {
					tmp = sf::Vector2u{sf::Vector2i{pos} + dir};
					if (pred(tmp)) {
						pos = tmp;
						return true;
					}
				}
			}
		} else {
			tmp = sf::Vector2u{sf::Vector2i{pos} + dir};
			if (pred(tmp)) {
				pos = tmp;
				return true;
			}
		}
	}

	// no position found
	return false;
}

}  // ::core
