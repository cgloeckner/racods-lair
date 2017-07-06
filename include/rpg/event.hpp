#pragma once
#include <SFML/System.hpp>

#include <utils/spatial_scene.hpp>
#include <core/event.hpp>
#include <rpg/common.hpp>
#include <rpg/resources.hpp>
#include <rpg/gameplay.hpp>

namespace rpg {

struct ActionEvent {
	core::ObjectID actor;
	bool idle;
	PlayerAction action;
	PerkTemplate const * perk; // only used by bots
	ItemTemplate const * item; // only used by bots

	ActionEvent();
};

struct ItemEvent {
	core::ObjectID actor;
	ItemTemplate const* item;
	enum { Add, Remove, Use } type;
	std::size_t quantity;  // when added or removed
	EquipmentSlot slot;	// when un/equipped

	ItemEvent();
};

struct PerkEvent {
	core::ObjectID actor;
	PerkTemplate const* perk;
	enum { Set, Use } type;
	std::size_t level;  // when sets

	PerkEvent();
};

struct QuickslotEvent {
	core::ObjectID actor;
	enum { Assign, Release } type;
	ItemTemplate const* item;
	PerkTemplate const* perk;
	std::size_t slot_id;  // @ when assign

	QuickslotEvent();
};

struct EffectEvent {
	core::ObjectID actor, causer;
	EffectTemplate const* effect;
	enum { Add, Remove } type;

	EffectEvent();
};

struct ExpEvent {
	core::ObjectID actor;
	std::uint64_t exp;
	unsigned int levelup;

	ExpEvent();
};

struct StatsEvent {
	core::ObjectID actor, causer;
	utils::EnumMap<Stat, int> delta;

	StatsEvent();
};

struct BoniEvent {
	core::ObjectID actor;
	enum { Add, Remove } type;
	StatsBoni const* boni;

	BoniEvent();
};

struct DeathEvent {
	core::ObjectID actor, causer;

	DeathEvent();
};

struct SpawnEvent {
	core::ObjectID actor, causer;
	bool respawn;

	SpawnEvent();
};

struct CombatEvent {
	core::ObjectID actor, target;
	CombatMetaData meta_data;

	CombatEvent();
};

struct ProjectileEvent {
	enum { Create, Destroy } type;
	core::ObjectID id;  // owner / bullet
	SpawnMetaData spawn;
	CombatMetaData meta_data;

	ProjectileEvent();
};

struct InteractEvent {
	core::ObjectID actor, target;

	InteractEvent();
};

struct TrainingEvent {
	core::ObjectID actor;
	enum { Perk, Attrib } type;
	PerkTemplate const* perk;
	Attribute attrib;

	TrainingEvent();
};

struct FeedbackEvent {
	core::ObjectID actor;
	FeedbackType type;

	FeedbackEvent();
};

// ---------------------------------------------------------------------------
// event sender and listener

using ActionSender = utils::SingleEventSender<ActionEvent>;
using ItemSender = utils::SingleEventSender<ItemEvent>;
using PerkSender = utils::SingleEventSender<PerkEvent>;
using QuickslotSender = utils::SingleEventSender<QuickslotEvent>;
using EffectSender = utils::SingleEventSender<EffectEvent>;
using ExpSender = utils::SingleEventSender<ExpEvent>;
using StatsSender = utils::SingleEventSender<StatsEvent>;
using BoniSender = utils::SingleEventSender<BoniEvent>;
using DeathSender = utils::SingleEventSender<DeathEvent>;
using SpawnSender = utils::SingleEventSender<SpawnEvent>;
using CombatSender = utils::SingleEventSender<CombatEvent>;
using ProjectileSender = utils::SingleEventSender<ProjectileEvent>;
using InteractSender = utils::SingleEventSender<InteractEvent>;
using TrainingSender = utils::SingleEventSender<TrainingEvent>;
using FeedbackSender = utils::SingleEventSender<FeedbackEvent>;

using ActionListener = utils::SingleEventListener<ActionEvent>;
using ItemListener = utils::SingleEventListener<ItemEvent>;
using PerkListener = utils::SingleEventListener<PerkEvent>;
using QuickslotListener = utils::SingleEventListener<QuickslotEvent>;
using EffectListener = utils::SingleEventListener<EffectEvent>;
using ExpListener = utils::SingleEventListener<ExpEvent>;
using StatsListener = utils::SingleEventListener<StatsEvent>;
using BoniListener = utils::SingleEventListener<BoniEvent>;
using DeathListener = utils::SingleEventListener<DeathEvent>;
using SpawnListener = utils::SingleEventListener<SpawnEvent>;
using CombatListener = utils::SingleEventListener<CombatEvent>;
using ProjectileListener = utils::SingleEventListener<ProjectileEvent>;
using InteractListener = utils::SingleEventListener<InteractEvent>;
using TrainingListener = utils::SingleEventListener<TrainingEvent>;
using FeedbackListener = utils::SingleEventListener<FeedbackEvent>;

}  // ::game
