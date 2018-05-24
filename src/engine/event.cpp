#include <utils/logger.hpp>
#include <engine/event.hpp>

namespace engine {

EventLogger::Node::Node()
	: stream{}
	, num_events{}
	, enabled{true} {
}

EventLogger::Node& EventLogger::at(std::type_index id) {
	auto i = nodes.find(id);
	if (i != nodes.end()) {
		return i->second;
	}
	
	return nodes[id];
}

void EventLogger::update() {
	dispatchAll(*this);
}

EventLogger::iterator EventLogger::begin() {
	return nodes.begin();
}

EventLogger::iterator EventLogger::end() {
	return nodes.end();
}

} // ::engine

// --------------------------------------------------------------------

std::string getName(std::string const & key, rpg::BaseResource const * ptr) {
	if (ptr == nullptr) {
		return "";
	}
	return key + "=" + ptr->internal_name;
}

std::ostream& operator<<(std::ostream& lhs, utils::IntervalState const & data) {
	return lhs << "current=" << data.current << ", min=" << data.min
		<< ", max=" << data.max << ", speed=" << data.speed << ", rise="
		<< data.rise << ", repeat=" << data.repeat;
}

std::ostream& operator<<(std::ostream& lhs, rpg::CombatMetaData const & data) {
	return lhs << "emitter=" << data.emitter
		<< getName(", primary", data.primary)
		<< getName(", secondary", data.secondary)
		<< getName(", perk", data.perk)
		<< getName(", effect", data.effect)
		<< getName(", trap", data.trap);
}

std::ostream& operator<<(std::ostream& lhs, rpg::SpawnMetaData const & data) {
	return lhs << "scene=" << data.scene << ", pos=" << data.pos
		<< ", direction=" << data.direction;
}

// --------------------------------------------------------------------

std::ostream& operator<<(std::ostream& lhs, core::InputEvent const & event) {
	return lhs << "actor=" << event.actor << ", move=" << event.move
		<< ", look=" << event.look;
}

std::ostream& operator<<(std::ostream& lhs, core::MoveEvent const & event) {
	return lhs << "actor=" << event.actor << ", source=" << event.source
		<< ", target=" << event.target << ", type="
		<< (event.type == core::MoveEvent::Left ? "Left" : "Reached");
}

std::ostream& operator<<(std::ostream& lhs, core::FocusEvent const & event) {
	return lhs << "observer=" << event.observer << ", observed="
		<< event.observed << ", type="
		<< (event.type == core::FocusEvent::Lost ? "Lost" : "Gained");
}

std::ostream& operator<<(std::ostream& lhs, core::CollisionEvent const & event) {
	return lhs << "actor=" << event.actor << ", collider="
		<< event.collider << ", pos=" << event.pos << ", reset_to="
		<< event.reset_to << ", interrupt=" << event.interrupt;
}

std::ostream& operator<<(std::ostream& lhs, core::AnimationEvent const & event) {
	lhs << "actor=" << event.actor << ", type=";
	switch (event.type) {
		case core::AnimationEvent::Action:
			lhs << "Action, action=" << event.action;
			break;
			
		case core::AnimationEvent::Brightness:
			lhs << "Brightness, " << event.interval;
			break;
			
		case core::AnimationEvent::Alpha:
			lhs << "Alpha, " << event.interval;
			break;
			
		case core::AnimationEvent::LightIntensity:
			lhs << "LightIntensity, " << event.interval;
			break;
			
		case core::AnimationEvent::LightRadius:
			lhs << "LightRadius, " << event.interval;
			break;
			
		case core::AnimationEvent::MinSaturation:
			lhs << "MinSaturation, " << event.interval;
			break;
			
		case core::AnimationEvent::MaxSaturation:
			lhs << "MaxSaturation, " << event.interval;
			break;
			
		case core::AnimationEvent::Legs:
			// todo: print better legani info :S
			lhs << "Legs, layer=" << event.leg_layer
				<< ", legs=" << event.legs;
			break;
			
		case core::AnimationEvent::Torso:
			// todo: print better torsoani info :S
			lhs << "Torso, layer=" << event.torso_layer
				<< ", torso=" << event.torso;
			break;
	}
	
	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, core::SpriteEvent const & event) {
	lhs << "actor=" << event.actor << ", type=";
	switch (event.type) {
		case core::SpriteEvent::Legs:
			lhs << "Legs, layer=" << event.leg_layer;
			break;
			
		case core::SpriteEvent::Torso:
			lhs << "Torso, layer=" << event.torso_layer;
			break;
	}
	// todo: print better texture info :S
	return lhs << ", texture=" << event.texture << "\n";
}

std::ostream& operator<<(std::ostream& lhs, core::SoundEvent const & event) {
	return lhs << "buffer=" << event.buffer << ", pitch=" << event.pitch
		<< ", relative_volume=" << event.relative_volume;
}

std::ostream& operator<<(std::ostream& lhs, core::MusicEvent const & event) {
	return lhs << "filename=" << event.filename;
}

std::ostream& operator<<(std::ostream& lhs, core::TeleportEvent const & event) {
	return lhs << "actor=" << event.actor << ", src_scene="
		<< event.src_scene << ", dst_scene=" << event.dst_scene
		<< ", src_pos=" << event.src_pos << ", dst_pos=" << event.dst_pos;
}

// --------------------------------------------------------------------

std::ostream& operator<<(std::ostream& lhs, rpg::ActionEvent const & event) {
	return lhs << "actor=" << event.actor << ", idle=" << event.idle
		<< ", action=" << event.action;
}

std::ostream& operator<<(std::ostream& lhs, rpg::ItemEvent const & event) {
	ASSERT(event.item != nullptr);
	lhs << "actor=" << event.actor << ", item="
		<< event.item->display_name << ", type=";
	switch (event.type) {
		case rpg::ItemEvent::Add:
			lhs << "Add, quantity=" << event.quantity;
			break;
			
		case rpg::ItemEvent::Remove:
			lhs << "Remove, quantity=" << event.quantity;
			break;
			
		case rpg::ItemEvent::Use:
			lhs << "Use, slot=" << event.slot;
			break;
	}
	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, rpg::PerkEvent const & event) {
	ASSERT(event.perk != nullptr);
	lhs << "actor=" << event.actor << ", perk=" << event.perk->internal_name
		<< ", type=";
	switch (event.type) {
		case rpg::PerkEvent::Set:
			lhs << "Set, level=" << event.level;
			break;
			
		case rpg::PerkEvent::Use:
			lhs << "Use";
			break;
	}
	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, rpg::QuickslotEvent const & event) {
	lhs << "actor=" << event.actor << ", type=";
	switch (event.type) {
		case rpg::QuickslotEvent::Assign:
			lhs << "Assign, slot_id=" << event.slot_id;
			break;
			
		case rpg::QuickslotEvent::Release:
			lhs << "Release";
			break;
	}
	return lhs << getName(", item", event.item)
		<< getName(", perk", event.perk);
}

std::ostream& operator<<(std::ostream& lhs, rpg::EffectEvent const & event) {
	ASSERT(event.effect != nullptr);
	return lhs << "actor=" << event.actor << ", causer=" << event.causer
		<< ", effect=" << event.effect->internal_name << ", type="
		<< (event.type == rpg::EffectEvent::Add ? "Add" : "Remove");
}

std::ostream& operator<<(std::ostream& lhs, rpg::ExpEvent const & event) {
	return lhs << "actor=" << event.actor << ", exp=" << event.exp
		<< ", levelup=" << event.levelup;
}

std::ostream& operator<<(std::ostream& lhs, rpg::StatsEvent const & event) {
	lhs << "actor=" << event.actor << ", causer=" << event.causer;
	for (auto const & pair: event.delta) {
		lhs << ", delta[" << pair.first << "]=" << pair.second;
	}
	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, rpg::BoniEvent const & event) {
	// todo: print better bonus info :S
	return lhs << "actor=" << event.actor << ", boni=" << event.boni
		<< ", type="
		<< (event.type == rpg::BoniEvent::Add ? "Add" : "Remove");
}

std::ostream& operator<<(std::ostream& lhs, rpg::DeathEvent const & event) {
	return lhs << "actor=" << event.actor << ", causer=" << event.causer;
}

std::ostream& operator<<(std::ostream& lhs, rpg::SpawnEvent const & event) {
	return lhs << "actor=" << event.actor << ", causer=" << event.causer;
}

std::ostream& operator<<(std::ostream& lhs, rpg::CombatEvent const & event) {
	return lhs << "actor=" << event.actor << ", target=" << event.target
		<< ", " << event.meta_data;
}

std::ostream& operator<<(std::ostream& lhs, rpg::ProjectileEvent const & event) {
	return lhs << "id=" << event.id << ", type="
		<< (event.type == rpg::ProjectileEvent::Create ? "Create" : "Destroy")
		<< event.spawn << event.meta_data;
}

std::ostream& operator<<(std::ostream& lhs, rpg::InteractEvent const & event) {
	return lhs << "actor=" << event.actor << ", target=" << event.target;
}

std::ostream& operator<<(std::ostream& lhs, rpg::TrainingEvent const & event) {
	lhs << "actor=" << event.actor << ", type=";
	switch (event.type) {
		case rpg::TrainingEvent::Perk:
			ASSERT(event.perk != nullptr);
			lhs << "Perk, perk=" << event.perk->internal_name;
			break;
			
		case rpg::TrainingEvent::Attrib:
			lhs << "Attrib, attrib=" << event.attrib;
			break;
	}
	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, rpg::FeedbackEvent const & event) {
	return lhs << "actor=" << event.actor << ", type=" << event.type;
}

// --------------------------------------------------------------------

std::ostream& operator<<(std::ostream& lhs, game::PathFailedEvent const & event) {
	return lhs << "actor=" << event.actor << ", pos=" << event.pos;
}

std::ostream& operator<<(std::ostream& lhs, game::PowerupEvent const & event) {
	lhs << "actor=" << event.actor;
	for (auto const & pair: event.delta) {
		lhs << ", delta[" << pair.first << "]=" << pair.second;
	}
	return lhs;
}

std::ostream& operator<<(std::ostream& lhs, game::ReleaseEvent const & event) {
	return lhs << "actor=" << event.actor;
}
