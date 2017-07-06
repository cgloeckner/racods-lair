#pragma once
#include <core/event.hpp>
#include <rpg/entity.hpp>
#include <rpg/event.hpp>
#include <rpg/stats.hpp>
#include <rpg/effect.hpp>
#include <rpg/item.hpp>
#include <rpg/perk.hpp>
#include <rpg/quickslot.hpp>
#include <rpg/player.hpp>
#include <game/event.hpp>
#include <engine/event.hpp>

namespace engine {

struct AvatarSystem
	: utils::EventListener<rpg::ItemEvent, rpg::PerkEvent, rpg::TrainingEvent,
		  rpg::StatsEvent, rpg::QuickslotEvent, rpg::ActionEvent, rpg::ExpEvent,
		  rpg::EffectEvent, rpg::SpawnEvent> {

	rpg::StatsSystem stats;
	rpg::EffectSystem effect;
	rpg::ItemSystem item;
	rpg::PerkSystem perk;
	rpg::QuickslotSystem quickslot;
	rpg::PlayerSystem player;

	AvatarSystem(core::LogContext& log, std::size_t max_objects);
	
	void connect(MultiEventListener& listener);
	void disconnect(MultiEventListener& listener);

	template <typename T>
	void bind(utils::SingleEventListener<T>& listener);
	
	template <typename T>
	void unbind(utils::SingleEventListener<T> const & listener);

	void handle(rpg::ItemEvent const& event);
	void handle(rpg::PerkEvent const& event);
	void handle(rpg::TrainingEvent const& event);
	void handle(rpg::StatsEvent const& event);
	void handle(rpg::QuickslotEvent const& event);
	void handle(rpg::ActionEvent const& event);
	void handle(rpg::ExpEvent const& event);
	void handle(rpg::EffectEvent const& event);
	void handle(rpg::SpawnEvent const& event);
	void handle(game::PowerupEvent const& event);

	sf::Time update(sf::Time const& elapsed);
	
	void clear();
};

/*

AvatarSystem : rage/avatar
	IN:
		Item -- Add/Remove/Use Anfrage
		Perk -- Set/Use Anfrage
		Training -- Anfrage
		Stats -- Änderung durch Kampf
		Quickslot -- Assign/Release
		Action -- NextSlot/PrevSlot/UseSlot
		Exp -- Anfrage durch Kampf
	OUT:
		Animation -- FrameAnimation anspielen
		Sprite -- SpriteTexture ändern
		Boni -- notify
		Stats -- notify
		Exp -- notify
		Feedback -- notify
		Training -- notify
		Death -- notify
		Effect -- notify
		Combat -- anstehender Kampf (durch Effect)
		Perk -- use


*/

}  // ::state
