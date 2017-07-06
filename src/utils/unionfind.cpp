#include <utils/unionfind.hpp>

namespace utils {

Unionfind::Set& Unionfind::make() {
	buffer.push_back(std::make_unique<Set>());
	auto& set = *buffer.back();
	set.rank = 0;
	set.parent = &set;
	return set;
}

Unionfind::Set& Unionfind::find(Unionfind::Set& child) {
	if (child.parent != &child) {
		child.parent = &find(*child.parent);
	}
	return *child.parent;
}

void Unionfind::join(Unionfind::Set& first, Unionfind::Set& second) {
	auto& p1 = find(first);
	auto& p2 = find(second);
	if (p1.rank > p2.rank) {
		p2.parent = &p1;
	} else {
		p1.parent = &p2;
	}
	if (p1.rank == p2.rank) {
		++p1.rank;
	}
}

}  // ::utils
