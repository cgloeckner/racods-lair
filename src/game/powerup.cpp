#include <game/powerup.hpp>

namespace game {

PowerupTrigger::PowerupTrigger(core::ObjectID gem, rpg::StatsManager const & stats_manager,
	rpg::PlayerManager const & player_manager, rpg::StatsSender& stats_sender,
	PowerupSender& powerup_sender, ReleaseListener& release_listener,
	PowerupType type)
	: core::BaseTrigger{}
	, gem{gem}
	, stats_manager{stats_manager}
	, player_manager{player_manager}
	, stats_sender{stats_sender}
	, powerup_sender{powerup_sender}
	, release_listener{release_listener}
	, expired{false} 
	, type{type}{
}

void PowerupTrigger::execute(core::ObjectID actor) {
	ASSERT(expired == false);
	
	if (!player_manager.has(actor)) {
		// ignore NPCs, bullets etc.
		return;
	}
	
	auto const & stats = stats_manager.query(actor);
	if (stats.stats[rpg::Stat::Life] == 0u) {
		// ignore dead players
		return;
	}
	
	utils::EnumMap<rpg::Stat, int> delta;
	bool full_life = (stats.stats[rpg::Stat::Life] == stats.properties[rpg::Property::MaxLife]);
	bool full_mana = (stats.stats[rpg::Stat::Mana] == stats.properties[rpg::Property::MaxMana]);
	switch (type) {
		case PowerupType::Life:
			if (full_life) {
				// ignore fully life-healed player
				return;
			}
			delta[rpg::Stat::Life] = stats.properties[rpg::Property::MaxLife] / 2;
			break;
		
		case PowerupType::Mana:
			if (full_mana) {
				// ignore fully mana-healed player
				return;
			}
			delta[rpg::Stat::Mana] = stats.properties[rpg::Property::MaxMana] / 2;
			break;
		
		case PowerupType::Rejuvenation:
			if (full_life && full_mana) {
				// ignore fully healed player
				return;
			}
			delta[rpg::Stat::Life] = stats.properties[rpg::Property::MaxLife] / 2;
			delta[rpg::Stat::Mana] = stats.properties[rpg::Property::MaxMana] / 2;
			break;
	}
	
	{
		PowerupEvent event;
		event.actor = actor;
		event.delta = delta;
		powerup_sender.send(event);
	}
	{
		rpg::StatsEvent event;
		event.actor = actor;
		event.delta = delta;
		stats_sender.send(event);
	}
	{
		ReleaseEvent event;
		event.actor = gem;
		release_listener.receive(event);
	}
	expired = true;
}

core::ObjectID PowerupTrigger::getId() const {
	return gem;
}

bool PowerupTrigger::isExpired() const {
	return expired;
}

} // ::game
