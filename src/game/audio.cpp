#include <game/audio.hpp>

namespace game {

namespace audio_impl {

Context::Context(core::LogContext& log, core::SoundManager const & sounds,
	rpg::ItemManager const & items, rpg::PlayerManager const & players,
	core::SoundSender& sound_sender, core::MusicSender& music_sender)
	: log{log}
	, sounds{sounds}
	, items{items}
	, players{players}
	, sound_sender{sound_sender}
	, music_sender{music_sender}
	, feedback{}
	, music{} {
}

// --------------------------------------------------------------------

void onMusicStopped(Context& context) {
	if (context.music.empty()) {
		// no music assigned
		// note: don't print because this would occur over and over again
		return;
	}
	
	core::MusicEvent event;
	event.filename = utils::randomAt(context.music);
	context.music_sender.send(event);
}

void onAction(Context& context, core::ObjectID actor, core::SoundAction action) {
	auto const & sounds = context.sounds.query(actor).sfx[action];
	if (sounds.empty()) {
		// no sound provided
		return;
	}
	
	core::SoundEvent event;
	event.buffer = utils::randomAt(sounds);
	context.sound_sender.send(event);
}

void onItem(Context& context, rpg::ItemEvent const & event) {
	auto& item = *event.item;
	if (item.sound == nullptr) {
		// no sound assigned
		return;
	}
	if (event.type != rpg::ItemEvent::Use) {
		// no playback for adding or removing items
		return;
	}
	if (event.slot != rpg::EquipmentSlot::None) {
		// no playback for equipping or unequipping items
		return;
	}
	
	core::SoundEvent ev;
	ev.buffer = item.sound;
	context.sound_sender.send(ev);
}

void onPerk(Context& context, rpg::PerkEvent const & event) {
	auto& perk = *event.perk;
	if (perk.sound == nullptr) {
		context.log.debug << "[Core/Audio] Perk '" << perk.internal_name
			<< "' has no sound\n";
		return;
	}
	if (event.type != rpg::PerkEvent::Use) {
		// no playback for setting perk level
		return;
	}
	
	core::SoundEvent ev;
	ev.buffer = perk.sound;
	context.sound_sender.send(ev);
}

void onFeedback(Context& context, rpg::FeedbackEvent const & event) {
	if (!context.players.has(event.actor)) {
		// only playback feedback events for players
		return;
	}
	
	auto const & data = context.feedback[event.type];
	if (data.empty()) {
		// no sound bound
		context.log.debug << "[Core/Audio] No feedback sound bound for "
			<< to_string(event.type) << "\n";
		return;
	}
	
	core::SoundEvent ev;
	ev.buffer = utils::randomAt(data);
	context.sound_sender.send(ev);
}

void onExp(Context& context, rpg::ExpEvent const & event) {
	if (event.levelup == 0u) {
		// nothing to handle
		return;
	}
	
	if (context.levelup.empty()) {
		// no sound bound
		context.log.debug << "[Core/Audio] No levelup sound bound\n";
		return;
	}
	
	core::SoundEvent ev;
	ev.buffer = utils::randomAt(context.levelup);
	context.sound_sender.send(ev);
}

void onPowerup(Context& context) {
	if (context.powerup.empty()) {
		// no sound bound
		context.log.debug << "[Core/Audio] No powerup sound bound\n";
		return;
	}
	
	core::SoundEvent ev;
	ev.buffer = utils::randomAt(context.powerup);
	context.sound_sender.send(ev);
}

} // ::audio_impl

// --------------------------------------------------------------------

AudioSystem::AudioSystem(core::LogContext& log, std::size_t max_objects,
	rpg::ItemManager const & items, rpg::PlayerManager const & players)
	: utils::EventListener<core::MusicEvent, core::MoveEvent, rpg::ItemEvent,
		rpg::PerkEvent, rpg::StatsEvent, rpg::DeathEvent,
		rpg::SpawnEvent, rpg::FeedbackEvent, rpg::ExpEvent,
		rpg::ProjectileEvent, rpg::ActionEvent, game::PowerupEvent>{}
	, utils::EventSender<core::SoundEvent, core::MusicEvent>{}
	, core::SoundManager{max_objects}
	, context{log, *this, items, players, *this, *this} {
}

void AudioSystem::assign(rpg::FeedbackType type, sf::SoundBuffer const & buffer) {
	context.feedback[type].push_back(&buffer);
}

void AudioSystem::addMusic(std::string const & filename) {
	context.music.push_back(filename);
}

void AudioSystem::addLevelup(sf::SoundBuffer const & buffer) {
	context.levelup.push_back(&buffer);
}

void AudioSystem::addPowerup(sf::SoundBuffer const & buffer) {
	context.powerup.push_back(&buffer);
}

void AudioSystem::handle(core::MusicEvent const & event) {
	if (event.filename.empty()) {
		audio_impl::onMusicStopped(context);
	}
}

void AudioSystem::handle(core::MoveEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	
	// note: footstep sfx not implemented yet
}

void AudioSystem::handle(rpg::ItemEvent const & event) {
	audio_impl::onItem(context, event);
}

void AudioSystem::handle(rpg::PerkEvent const & event) {
	audio_impl::onPerk(context, event);
}

void AudioSystem::handle(rpg::StatsEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	if (event.delta[rpg::Stat::Life] < 0) {
		audio_impl::onAction(context, event.actor, core::SoundAction::Hit);
	}
}

void AudioSystem::handle(rpg::DeathEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	audio_impl::onAction(context, event.actor, core::SoundAction::Death);
}

void AudioSystem::handle(rpg::SpawnEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	audio_impl::onAction(context, event.actor, core::SoundAction::Spawn);
}

void AudioSystem::handle(rpg::FeedbackEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	audio_impl::onFeedback(context, event);
}

void AudioSystem::handle(rpg::ExpEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	audio_impl::onExp(context, event);
}

void AudioSystem::handle(rpg::ActionEvent const & event) {
	if (!has(event.actor)) {
		return;
	}
	
	switch (event.action) {
		case rpg::PlayerAction::Attack:
			if (context.items.has(event.actor)) {
				auto const & item = context.items.query(event.actor);
				auto primary = item.equipment[rpg::EquipmentSlot::Weapon];
				if (primary != nullptr) {
					rpg::ItemEvent tmp;
					tmp.type = rpg::ItemEvent::Use;
					tmp.item = primary;
					onItem(context, tmp);
				}
			}
			onAction(context, event.actor, core::SoundAction::Attack);
			break;
			
		default:
			break;
	}
	
	/*
	switch (event.meta_data.emitter) {
		case rpg::EmitterType::Weapon:
			context.log.debug << "Weapon use\n";
			{
				rpg::ItemEvent tmp;
				tmp.item = event.meta_data.primary;
				if (tmp.item != nullptr) {
					onItem(context, tmp);
				}
			}
			onAction(context, event.actor, core::SoundAction::Attack);
			break;
		
		case rpg::EmitterType::Perk:
			{
				rpg::PerkEvent tmp;
				tmp.perk = event.meta_data.perk;
				if (tmp.perk != nullptr) {
					onPerk(context, tmp);
				}
			}
			onAction(context, event.actor, core::SoundAction::Perk);
			break;
			
		default:
			break;
	}
	*/
}

void AudioSystem::handle(rpg::ProjectileEvent const & event) {
	if (event.type == rpg::ProjectileEvent::Destroy && has(event.id)) {
		audio_impl::onAction(context, event.id, core::SoundAction::Death);
	}
}

void AudioSystem::handle(game::PowerupEvent const & event) {
	audio_impl::onPowerup(context);
}

void AudioSystem::update(sf::Time const& elapsed) {
	dispatch<core::MusicEvent>(*this);
	dispatch<core::MoveEvent>(*this);
	dispatch<rpg::ItemEvent>(*this);
	dispatch<rpg::PerkEvent>(*this);
	dispatch<rpg::StatsEvent>(*this);
	dispatch<rpg::DeathEvent>(*this);
	dispatch<rpg::SpawnEvent>(*this);
	dispatch<rpg::FeedbackEvent>(*this);
	dispatch<rpg::ExpEvent>(*this);
	dispatch<rpg::ProjectileEvent>(*this);
	dispatch<rpg::ActionEvent>(*this);
	dispatch<game::PowerupEvent>(*this);

	propagate<core::SoundEvent>();
	propagate<core::MusicEvent>();
}

} // ::game
