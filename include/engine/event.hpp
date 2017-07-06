#pragma once
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <sstream>

#include <core/event.hpp>
#include <rpg/event.hpp>
#include <game/event.hpp>

namespace engine {

using MultiEventListener = utils::EventListener<
	// core events
	core::InputEvent, core::MoveEvent, core::FocusEvent,
	core::CollisionEvent, core::AnimationEvent, core::SpriteEvent,
	core::SoundEvent, core::MusicEvent, core::TeleportEvent,
	// rpg events
	rpg::ActionEvent, rpg::ItemEvent, rpg::PerkEvent,
	rpg::QuickslotEvent, rpg::EffectEvent, rpg::ExpEvent,
	rpg::StatsEvent, rpg::BoniEvent, rpg::DeathEvent, rpg::SpawnEvent,
	rpg::CombatEvent, rpg::ProjectileEvent, rpg::InteractEvent,
	rpg::TrainingEvent, rpg::FeedbackEvent,
	// game events
	game::PathFailedEvent, game::PowerupEvent, game::ReleaseEvent>;

// --------------------------------------------------------------------

class EventLogger
	: public MultiEventListener {
  private:
	struct Node {
		std::stringstream stream;
		std::size_t num_events;
		bool enabled;
		
		Node();
	};
	
	using container = std::unordered_map<std::type_index, Node>;
	using iterator = container::iterator;
	
	container nodes;
	
	Node& at(std::type_index id);
	
  public:
	template <typename T>
	void handle(T const & event);
	
	template <typename T>
	void clear();
	
	template <typename T>
	void setEnabled(bool flag);
	
	void update();
	
	iterator begin();
	iterator end();
};

} // ::engine

// --------------------------------------------------------------------

std::ostream& operator<<(std::ostream& lhs, core::InputEvent const & event);
std::ostream& operator<<(std::ostream& lhs, core::MoveEvent const & event);
std::ostream& operator<<(std::ostream& lhs, core::FocusEvent const & event);
std::ostream& operator<<(std::ostream& lhs, core::CollisionEvent const & event);
std::ostream& operator<<(std::ostream& lhs, core::AnimationEvent const & event);
std::ostream& operator<<(std::ostream& lhs, core::SpriteEvent const & event);
std::ostream& operator<<(std::ostream& lhs, core::SoundEvent const & event);
std::ostream& operator<<(std::ostream& lhs, core::MusicEvent const & event);
std::ostream& operator<<(std::ostream& lhs, core::TeleportEvent const & event);

std::ostream& operator<<(std::ostream& lhs, rpg::ActionEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::ItemEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::PerkEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::QuickslotEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::EffectEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::ExpEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::StatsEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::BoniEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::DeathEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::SpawnEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::CombatEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::ProjectileEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::InteractEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::TrainingEvent const & event);
std::ostream& operator<<(std::ostream& lhs, rpg::FeedbackEvent const & event);

std::ostream& operator<<(std::ostream& lhs, game::PathFailedEvent const & event);
std::ostream& operator<<(std::ostream& lhs, game::PowerupEvent const & event);
std::ostream& operator<<(std::ostream& lhs, game::ReleaseEvent const & event);

// include implementation details
#include <engine/event.inl>
