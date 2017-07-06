#include <rpg/entity.hpp>

namespace rpg {

Perk::Perk() : perk{nullptr}, level{0u} {}

Perk::Perk(PerkTemplate const& perk, std::size_t level)
	: perk{&perk}, level{level} {}

Item::Item() : item{nullptr}, quantity{0u} {}

Item::Item(ItemTemplate const& item, std::size_t quantity)
	: item{&item}, quantity{quantity} {}

Shortcut::Shortcut() : perk{nullptr}, item{nullptr} {}

Shortcut::Shortcut(PerkTemplate const& perk) : perk{&perk}, item{nullptr} {}

Shortcut::Shortcut(ItemTemplate const& item) : perk{nullptr}, item{&item} {}

Effect::Effect() : effect{nullptr}, remain{sf::Time::Zero} {}

Effect::Effect(EffectTemplate const& effect)
	: effect{&effect}, remain{effect.duration} {}

CombatMetaData::CombatMetaData()
	: emitter{default_value<EmitterType>()}
	, primary{nullptr}
	, secondary{nullptr}
	, perk{nullptr}
	, effect{nullptr}
	, trap{nullptr} {}

}  // ::game
