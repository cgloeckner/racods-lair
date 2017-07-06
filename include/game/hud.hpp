#pragma once
#include <core/common.hpp>
#include <core/event.hpp>
#include <rpg/event.hpp>
#include <rpg/entity.hpp>
#include <game/entity.hpp>
#include <game/resources.hpp>
#include <ui/floatingtext.hpp>

namespace game {

namespace hud_impl {

/// Context of the hud system
struct Context {
	sf::Font const * combat_font;
	unsigned int combat_size;
	sf::Sprite corner;
	mutable sf::Sprite border;
	
	core::LogContext& log;
	core::CameraSystem const & cameras;
	core::MovementManager const & movements;
	core::FocusManager const & focus;
	core::DungeonSystem& dungeons;
	rpg::StatsManager const & stats;
	rpg::PlayerManager const & players;
	HudManager& huds;
	Localization& locale;
	
	std::unordered_map<std::size_t, ui::FloatingTexts> floating_texts;
	
	Context(core::LogContext& log, core::CameraSystem const& camera_system,
		core::MovementManager const & movement_manager, core::FocusManager const & focus_manager, 
		core::DungeonSystem& dungeon_system, rpg::StatsManager const& stats_manager,
		rpg::PlayerManager const & player_manager, HudManager& hud_manager,
		Localization& locale);
};

// --------------------------------------------------------------------

void updateStatBars(HudData& hud, rpg::StatsData const & stats);
void updateExpBar(HudData& hud, rpg::PlayerData const & player);

void addCombatLabel(Context& context, core::ObjectID id, unsigned int value, sf::Color const & color);

// --------------------------------------------------------------------

void onTeleport(Context& context, core::TeleportEvent const& event);
void onFocus(Context& context, core::FocusEvent const& event);
void onStats(Context& context, rpg::StatsEvent const& event);
void onDeath(Context& context, rpg::DeathEvent const& event);
void onSpawn(Context& context, rpg::SpawnEvent const& event);
void onExp(Context& context, rpg::ExpEvent const& event);
void onFeedback(Context& context, rpg::FeedbackEvent const& event);
void onPowerup(Context& context, PowerupEvent const& event);
void onUpdate(Context const & context, HudData& hud, sf::Time const & elapsed);

// ---------------------------------------------------------------------------

void drawDecoration(Context const & context, sf::RenderTarget& target, sf::RenderStates states);

}  // ::hud_impl

// ---------------------------------------------------------------------------

class HudSystem
	// Event API
	: public utils::EventListener<core::TeleportEvent, core::FocusEvent, rpg::StatsEvent,
		  rpg::DeathEvent, rpg::SpawnEvent, rpg::ExpEvent, rpg::FeedbackEvent,
		  PowerupEvent>
	// Component API
	, public HudManager
	, public sf::Drawable {

  private:
	hud_impl::Context context;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
	HudSystem(core::LogContext& log, std::size_t max_objects, core::CameraSystem const& camera_system,
		core::MovementManager const & movement_manager, core::FocusManager const & focus_manager,
		core::DungeonSystem& dungeon_system, rpg::StatsManager const& stats_manage,
		rpg::PlayerManager const & player_manager, Localization& locale);
	
	void setup(sf::Font const & font, unsigned int char_size, sf::Texture const & corner_tex,
		sf::Texture const & border_tex);
	
	void handle(core::TeleportEvent const& event);
	void handle(core::FocusEvent const& event);
	void handle(rpg::StatsEvent const& event);
	void handle(rpg::DeathEvent const& event);
	void handle(rpg::SpawnEvent const& event);
	void handle(rpg::ExpEvent const& event);
	void handle(rpg::FeedbackEvent const& event);
	void handle(PowerupEvent const& event);
	
	void update(sf::Time const& elapsed);
};

}  // ::rage
