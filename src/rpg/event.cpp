#include <rpg/event.hpp>

namespace rpg {

ActionEvent::ActionEvent()
	: actor{0u}
	, idle{true}
	, action{default_value<PlayerAction>()}
	, perk{nullptr}
	, item{nullptr} {
}

ItemEvent::ItemEvent()
	: actor{0u}
	, item{nullptr}
	, type{ItemEvent::Use}
	, quantity{0u}
	, slot{EquipmentSlot::None} {}

PerkEvent::PerkEvent()
	: actor{0u}, perk{nullptr}, type{PerkEvent::Use}, level{0u} {}

QuickslotEvent::QuickslotEvent()
	: actor{0u}
	, type{QuickslotEvent::Assign}
	, item{nullptr}
	, perk{nullptr}
	, slot_id{0} {}

EffectEvent::EffectEvent()
	: actor{0u}, causer{0u}, effect{nullptr}, type{EffectEvent::Add} {}

ExpEvent::ExpEvent() : actor{0u}, exp{0u}, levelup{0u} {}

StatsEvent::StatsEvent() : actor{0u}, causer{0u}, delta{0} {}

BoniEvent::BoniEvent() : actor{0u}, type{BoniEvent::Add}, boni{nullptr} {}

DeathEvent::DeathEvent() : actor{0u}, causer{0u} {}

SpawnEvent::SpawnEvent()
	: actor{0u}
	, causer{0u}
	, respawn{false} {
}

CombatEvent::CombatEvent() : actor{0u}, target{0u}, meta_data{} {}

ProjectileEvent::ProjectileEvent()
	: type{ProjectileEvent::Create}, id{0u}, spawn{}, meta_data{} {}

InteractEvent::InteractEvent() : actor{0u}, target{0u} {}

TrainingEvent::TrainingEvent()
	: actor{0u}
	, type{TrainingEvent::Perk}
	, perk{nullptr}
	, attrib{default_value<Attribute>()} {}

FeedbackEvent::FeedbackEvent()
	: actor{0u}
	, type{default_value<FeedbackType>()} {}

}  // ::game

// ---------------------------------------------------------------------------
// Template instatiations

namespace utils {

template class SingleEventSender<rpg::ActionEvent>;
template class SingleEventSender<rpg::ItemEvent>;
template class SingleEventSender<rpg::PerkEvent>;
template class SingleEventSender<rpg::QuickslotEvent>;
template class SingleEventSender<rpg::EffectEvent>;
template class SingleEventSender<rpg::ExpEvent>;
template class SingleEventSender<rpg::StatsEvent>;
template class SingleEventSender<rpg::BoniEvent>;
template class SingleEventSender<rpg::DeathEvent>;
template class SingleEventSender<rpg::SpawnEvent>;
template class SingleEventSender<rpg::CombatEvent>;
template class SingleEventSender<rpg::ProjectileEvent>;
template class SingleEventSender<rpg::InteractEvent>;
template class SingleEventSender<rpg::FeedbackEvent>;

template class SingleEventListener<rpg::ActionEvent>;
template class SingleEventListener<rpg::ItemEvent>;
template class SingleEventListener<rpg::PerkEvent>;
template class SingleEventListener<rpg::QuickslotEvent>;
template class SingleEventListener<rpg::EffectEvent>;
template class SingleEventListener<rpg::ExpEvent>;
template class SingleEventListener<rpg::StatsEvent>;
template class SingleEventListener<rpg::BoniEvent>;
template class SingleEventListener<rpg::DeathEvent>;
template class SingleEventListener<rpg::SpawnEvent>;
template class SingleEventListener<rpg::CombatEvent>;
template class SingleEventListener<rpg::ProjectileEvent>;
template class SingleEventListener<rpg::InteractEvent>;
template class SingleEventListener<rpg::FeedbackEvent>;

}  // ::utils
