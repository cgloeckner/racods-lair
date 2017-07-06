namespace core {

template <typename Func>
void updateChunked(
	Func func, sf::Time const& elapsed, sf::Time const& steptime) {
	// update n times with maximum frametime
	auto num_steps = elapsed.asMilliseconds() / steptime.asMilliseconds();
	for (auto i = 0; i < num_steps; ++i) {
		func(steptime);
	}
	// update once with remaining frametime
	auto remain = elapsed % steptime;
	if (remain > sf::Time::Zero) {
		func(remain);
	}
}

}  // ::core
