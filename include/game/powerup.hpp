#pragma once
#include <game/event.hpp>
#include <game/session.hpp>

namespace game {

enum class PowerupType {
	Life, Mana, Rejuvenation
};

class PowerupTrigger: public core::BaseTrigger {
  private:
	core::ObjectID const gem;
	rpg::StatsManager const & stats_manager;
	rpg::PlayerManager const & player_manager;
	rpg::StatsSender& stats_sender;
	PowerupSender& powerup_sender;
	ReleaseListener& release_listener;
	bool expired;
	
	PowerupType type;
	
  public:
	PowerupTrigger(core::ObjectID gem, rpg::StatsManager const & stats_manager,
		rpg::PlayerManager const & player_manager,
		rpg::StatsSender& stats_sender, PowerupSender& powerup_sender,
		ReleaseListener& release_listener, PowerupType type);
	
	void execute(core::ObjectID actor) override;
	
	core::ObjectID getId() const;
	bool isExpired() const override;
};

} // ::game
