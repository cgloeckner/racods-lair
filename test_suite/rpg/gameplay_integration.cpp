#include <vector>
#include <boost/test/unit_test.hpp>
#include <testsuite/sfml_system.hpp>
#include <testsuite/singleton.hpp>

#include <utils/assert.hpp>
#include <core/algorithm.hpp>
#include <core/animation.hpp>
#include <core/collision.hpp>
#include <core/dungeon.hpp>
#include <core/focus.hpp>
#include <core/movement.hpp>
#include <core/teleport.hpp>
#include <rpg/action.hpp>
#include <rpg/combat.hpp>
#include <rpg/delay.hpp>
#include <rpg/effect.hpp>
#include <rpg/input.hpp>
#include <rpg/interact.hpp>
#include <rpg/item.hpp>
#include <rpg/perk.hpp>
#include <rpg/player.hpp>
#include <rpg/projectile.hpp>
#include <rpg/quickslot.hpp>
#include <rpg/stats.hpp>
#include <rpg/resources.hpp>

struct GameplayFixture
	: utils::EventListener<core::SpriteEvent, rpg::ProjectileEvent,
		  rpg::ExpEvent, rpg::StatsEvent, rpg::DeathEvent, rpg::FeedbackEvent> {
	sf::Texture dummy_tileset;
	sf::SoundBuffer dummy_sound;
	core::IdManager id_manager;
	core::LogContext log;
	std::vector<core::ObjectID> objects;

	std::vector<core::SpriteEvent> sprites;
	std::vector<rpg::ProjectileEvent> projectiles;
	std::vector<rpg::ExpEvent> exps;
	std::vector<rpg::StatsEvent> _stats;
	std::vector<rpg::DeathEvent> deaths;
	std::vector<rpg::FeedbackEvent> feedbacks;

	std::vector<core::InputEvent> moves;  // scheduled

	core::AnimationSystem animation;
	core::RenderManager render;
	core::DungeonSystem dungeon;
	core::MovementSystem movement;
	core::CollisionSystem collision;
	core::FocusSystem focus;

	rpg::StatsSystem stats;
	rpg::EffectSystem effect;
	rpg::ItemSystem item;
	rpg::PerkSystem perk;
	rpg::PlayerSystem player;
	rpg::CombatSystem combat;
	rpg::ProjectileSystem projectile;

	rpg::ActionSystem action;
	rpg::DelaySystem delay;
	rpg::InputSystem input;
	rpg::InteractSystem interact;
	rpg::QuickslotSystem quickslot;

	utils::SceneID scene, scene2;

	rpg::SpriteTemplate weapon_sprite, body_sprite, armor_sprite;
	rpg::EntityTemplate arrow_entity, flame_entity;
	rpg::ItemTemplate flamesword, icebow, armor, manapotion;
	rpg::PerkTemplate fireball, healing, healing_other;
	rpg::BulletTemplate arrow, flame;
	rpg::EffectTemplate burn, frozen;
	rpg::TrapTemplate trap;

	utils::Keybinding<rpg::PlayerAction> keys;

	GameplayFixture()
		: utils::EventListener<core::SpriteEvent, rpg::ProjectileEvent,
			  rpg::ExpEvent, rpg::StatsEvent, rpg::DeathEvent,
			  rpg::FeedbackEvent>{}
		, dummy_tileset{}
		, dummy_sound{}
		, id_manager{}
		, log{}
		, objects{}
		, sprites{}
		, projectiles{}
		, exps{}
		, _stats{}
		, deaths{}
		, feedbacks{}
		, moves{}  // graphics system
		, animation{log, 1000u}
		, render{}  // physics system
		, dungeon{}
		, movement{log, 1000u, dungeon}
		, collision{log, 1000u, dungeon, movement}
		, focus{log, 1000u, dungeon, movement}  // roleplaying system
		, stats{log, 1000u}
		, effect{log, 1000u}
		, item{log, 1000u, stats}
		, perk{log, 1000u, stats}
		, player{log, 1000u, stats}
		, combat{log, movement, projectile, perk, stats, interact, 0.f}
		, projectile{log, 1000u, movement, collision, dungeon}  // behavior system
		, action{log, 1000u}
		, delay{log, dungeon, movement, focus, animation, item, stats, interact,
			  player}
		, input{log, 1000u, dungeon, movement, focus}
		, interact{log, 1000u, movement, focus, player}
		, quickslot{log, 1000u}  // others
		, scene{0u}
		, scene2{0u}
		, weapon_sprite{}
		, body_sprite{}
		, armor_sprite{}
		, arrow_entity{}
		, flame_entity{}
		, flamesword{}
		, icebow{}
		, armor{}
		, manapotion{}
		, fireball{}
		, healing{}
		, healing_other{}
		, arrow{}
		, flame{}
		, burn{}
		, frozen{}
		, trap{}
		, keys{} {
		// log.debug.add(std::cout);

		// connect animation events
		animation.bind<core::AnimationEvent>(
			action);  // react on finished action
		action.bind<core::AnimationEvent>(animation);  // animate move/stop
		delay.bind<core::AnimationEvent>(animation);   // animate action
		item.bind<core::AnimationEvent>(animation);	// update layers
		perk.bind<core::AnimationEvent>(animation);	// animate casting

		// connect sprite events
		item.bind<core::SpriteEvent>(*this);  // update layers

		// connect collision events
		collision.bind<core::CollisionEvent>(movement);  // interrupt movement
		collision.bind<core::CollisionEvent>(action);	// interrupt movement
		collision.bind<core::CollisionEvent>(
			projectile);  // trigger bullet collision

		// connect move events
		movement.bind<core::MoveEvent>(collision);  // try movement
		collision.bind<core::MoveEvent>(focus);		// update focus on move
		collision.bind<core::MoveEvent>(action);	// propagate movement
		collision.bind<core::MoveEvent>(interact);  // propagate movement

		// connect input events
		input.bind<core::InputEvent>(action);		// try movement/looking
		action.bind<core::InputEvent>(movement);	// try movement
		action.bind<core::InputEvent>(focus);		// try looking
		interact.bind<core::InputEvent>(movement);  // move barrier

		// connect action events
		input.bind<rpg::ActionEvent>(action);	  // try action
		action.bind<rpg::ActionEvent>(delay);	  // trigger action
		action.bind<rpg::ActionEvent>(quickslot);  // trigger action

		// connect item event
		interact.bind<rpg::ItemEvent>(item);   // propagate loot
		quickslot.bind<rpg::ItemEvent>(item);  // try item use

		// connect perk event
		perk.bind<rpg::PerkEvent>(
			delay);  // delay perk usage after mana consume
		quickslot.bind<rpg::PerkEvent>(perk);  // try perk use

		// connect boni event
		effect.bind<rpg::BoniEvent>(stats);  // on effect inflict/vanish
		item.bind<rpg::BoniEvent>(stats);	// on equip change

		// connect interact event
		delay.bind<rpg::InteractEvent>(interact);  // trigger interaction

		// connect combat events
		effect.bind<rpg::CombatEvent>(
			combat);  // trigger effect's damage/recovery
		delay.bind<rpg::CombatEvent>(
			combat);  // trigger delayed combat (e.g. melee)
		projectile.bind<rpg::CombatEvent>(combat);  // trigger bullet's damage

		// connect stats events
		combat.bind<rpg::StatsEvent>(stats);  // propagate damage
		item.bind<rpg::StatsEvent>(stats);	// heal via potion
		perk.bind<rpg::StatsEvent>(stats);	// consume mana
		stats.bind<rpg::StatsEvent>(*this);   // display at hud

		// connect exp events
		combat.bind<rpg::ExpEvent>(player);  // propagate exp gain
		player.bind<rpg::ExpEvent>(stats);   // to trigger levelup
		player.bind<rpg::ExpEvent>(*this);   // display at hud

		// connect effect events
		combat.bind<rpg::EffectEvent>(effect);  // inflict effect

		// connect death events
		stats.bind<rpg::DeathEvent>(action);  // propagate death
		stats.bind<rpg::DeathEvent>(*this);   // display at hud

		// connect projectile events
		combat.bind<rpg::ProjectileEvent>(*this);	  // destroy projectile
		delay.bind<rpg::ProjectileEvent>(*this);	   // create projectile
		projectile.bind<rpg::ProjectileEvent>(*this);  // destroy projectile

		// connect quickslot events
		item.bind<rpg::QuickslotEvent>(quickslot);  // release item
		perk.bind<rpg::QuickslotEvent>(
			quickslot);  // release perk (after level set to 0)

		// connect training events
		player.bind<rpg::TrainingEvent>(perk);   // to train a perk
		player.bind<rpg::TrainingEvent>(stats);  // to train an attribute

		// connect feedback events
		quickslot.bind<rpg::FeedbackEvent>(*this);
		item.bind<rpg::FeedbackEvent>(*this);
		perk.bind<rpg::FeedbackEvent>(*this);
		player.bind<rpg::FeedbackEvent>(*this);

		// add scenes
		sf::Vector2u grid_size{10u, 10u};
		scene =
			dungeon.create(dummy_tileset, grid_size, sf::Vector2f{1.f, 1.f});
		scene2 =
			dungeon.create(dummy_tileset, grid_size, sf::Vector2f{1.f, 1.f});
		for (auto i : {scene, scene2}) {
			auto& d = dungeon[i];
			for (auto y = 0u; y < grid_size.y; ++y) {
				for (auto x = 0u; x < grid_size.x; ++x) {
					auto& cell = d.getCell({x, y});
					if (x == 0u || x == grid_size.x - 1u || y == 0u ||
						y == grid_size.y - 1u) {
						cell.terrain = core::Terrain::Wall;
					} else {
						cell.terrain = core::Terrain::Floor;
					}
				}
			}
		}
		// prepare sprites
		weapon_sprite.legs.duration = sf::seconds(1.f);
		weapon_sprite.legs.frames.resize(1u);
		weapon_sprite.legs.frames[0].duration = sf::seconds(1.f);
		weapon_sprite.frameset = &dummy_tileset;
		armor_sprite.legs.duration = sf::seconds(1.f);
		armor_sprite.legs.frames.resize(1u);
		armor_sprite.legs.frames[0].duration = sf::seconds(1.f);
		armor_sprite.frameset = &dummy_tileset;
		body_sprite.legs.duration = sf::seconds(1.f);
		body_sprite.legs.frames.resize(1u);
		body_sprite.frameset = &dummy_tileset;
		body_sprite.legs.frames[0].duration = sf::seconds(1.f);
		for (auto const value : utils::EnumRange<core::AnimationAction>{}) {
			weapon_sprite.torso[value].duration = sf::seconds(1.f);
			weapon_sprite.torso[value].frames.resize(1u);
			weapon_sprite.torso[value].frames[0].duration = sf::seconds(1.f);
			armor_sprite.torso[value].duration = sf::seconds(1.f);
			armor_sprite.torso[value].frames.resize(1u);
			armor_sprite.torso[value].frames[0].duration = sf::seconds(1.f);
			body_sprite.torso[value].duration = sf::seconds(1.f);
			body_sprite.torso[value].frames.resize(1u);
			body_sprite.torso[value].frames[0].duration = sf::seconds(1.f);
		}
		arrow_entity.sprite = &weapon_sprite;
		flame_entity.sprite = &weapon_sprite;
		// prepare items
		flamesword.type = rpg::ItemType::Weapon;
		flamesword.two_handed = false;
		flamesword.melee = true;
		flamesword.slot = rpg::EquipmentSlot::Weapon;
		flamesword.damage[rpg::DamageType::Fire] = 10;
		flamesword.effect.effect = &burn;
		flamesword.effect.ratio = 1.f;
		flamesword.sound = &dummy_sound;
		flamesword.sprite = &weapon_sprite;
		icebow.type = rpg::ItemType::Weapon;
		icebow.two_handed = true;
		icebow.melee = false;
		icebow.slot = rpg::EquipmentSlot::Weapon;
		icebow.damage[rpg::DamageType::Ice] = 10;
		icebow.effect.effect = &frozen;
		icebow.effect.ratio = 1.f;
		icebow.bullet.bullet = &arrow;
		icebow.sound = &dummy_sound;
		icebow.sprite = &weapon_sprite;
		armor.type = rpg::ItemType::Armor;
		armor.slot = rpg::EquipmentSlot::Body;
		armor.sound = &dummy_sound;
		armor.sprite = &armor_sprite;
		armor.boni.defense[rpg::DamageType::Blade] = 10;
		manapotion.type = rpg::ItemType::Potion;
		manapotion.recover[rpg::Stat::Mana] = 10;
		manapotion.effect.effect = &frozen;
		manapotion.effect.ratio = 1.f;
		manapotion.sound = &dummy_sound;
		// prepare perks
		fireball.type = rpg::PerkType::Enemy;
		fireball.damage[rpg::DamageType::Fire] = 10;
		fireball.effect.effect = &burn;
		fireball.effect.ratio = 1.f;
		fireball.bullet.bullet = &flame;
		fireball.sound = &dummy_sound;
		healing.type = rpg::PerkType::Self;
		healing.recover[rpg::Stat::Life] = 10;
		healing_other.type = rpg::PerkType::Allied;
		healing_other.recover[rpg::Stat::Life] = 10;
		// prepare effects
		burn.damage[rpg::DamageType::Fire] = 10;
		burn.duration = sf::milliseconds(2500);
		frozen.damage[rpg::DamageType::Ice] = 10;
		frozen.duration = sf::milliseconds(2500);
		// prepare trap
		trap.damage[rpg::DamageType::Bullet] = 10;
		trap.effect.effect = &frozen;
		trap.effect.ratio = 1.f;
		// prepare bullets
		arrow.radius = 0.1f;
		arrow.entity = &arrow_entity;
		flame.radius = 0.5f;
		flame.entity = &flame_entity;
		// prepare keys
		keys.set(rpg::PlayerAction::Attack, {sf::Keyboard::F1});
		keys.set(rpg::PlayerAction::Interact, {sf::Keyboard::F2});
		keys.set(rpg::PlayerAction::UseSlot, {sf::Keyboard::F3});
		keys.set(rpg::PlayerAction::PrevSlot, {sf::Keyboard::F4});
		keys.set(rpg::PlayerAction::NextSlot, {sf::Keyboard::F5});
		keys.set(rpg::PlayerAction::Pause, {sf::Keyboard::F6});
		keys.set(rpg::PlayerAction::ToggleAutoLook, {sf::Keyboard::F7});
		keys.set(rpg::PlayerAction::MoveN, {sf::Keyboard::Up});
		keys.set(rpg::PlayerAction::MoveS, {sf::Keyboard::Down});
		keys.set(rpg::PlayerAction::MoveW, {sf::Keyboard::Left});
		keys.set(rpg::PlayerAction::MoveE, {sf::Keyboard::Right});
		keys.set(rpg::PlayerAction::LookN, {sf::Keyboard::W});
		keys.set(rpg::PlayerAction::LookS, {sf::Keyboard::S});
		keys.set(rpg::PlayerAction::LookW, {sf::Keyboard::A});
		keys.set(rpg::PlayerAction::LookE, {sf::Keyboard::D});
	}

	void setInput(rpg::PlayerAction action, bool pressed) {
		sf::Event event;
		if (pressed) {
			event.type = sf::Event::KeyPressed;
		} else {
			event.type = sf::Event::KeyReleased;
		}
		event.key.code = keys.get(action).key.key;
		input.handle(event);
	}

	void reset() {
		for (auto i : {scene, scene2}) {
			auto& d = dungeon[i];
			// clear dungeons
			for (auto y = 0u; y < 10u; ++y) {
				for (auto x = 0u; x < 10u; ++x) {
					auto& cell = d.getCell({x, y});
					cell.entities.clear();
					cell.trigger = nullptr;
					if (x == 0u || x == 9u || y == 0u || y == 9u) {
						cell.terrain = core::Terrain::Wall;
					} else {
						cell.terrain = core::Terrain::Floor;
					}
				}
			}
		}
		// remove components
		for (auto id : objects) {
			destroyObject(id);
		}
		objects.clear();

		// cleanup systems
		id_manager.reset();

		animation.cleanup();
		render.cleanup();
		movement.cleanup();
		collision.cleanup();
		focus.cleanup();

		stats.cleanup();
		effect.cleanup();
		item.cleanup();
		perk.cleanup();
		player.cleanup();
		projectile.cleanup();

		action.cleanup();
		input.cleanup();
		interact.cleanup();
		quickslot.cleanup();

		delay.reset();
		input.reset();

		// reset animation events
		dynamic_cast<core::AnimationSender&>(animation).clear();
		dynamic_cast<core::AnimationSender&>(action).clear();
		dynamic_cast<core::AnimationSender&>(delay).clear();
		dynamic_cast<core::AnimationSender&>(item).clear();
		dynamic_cast<core::AnimationSender&>(perk).clear();
		dynamic_cast<core::AnimationListener&>(action).clear();
		dynamic_cast<core::AnimationListener&>(animation).clear();

		// reset sprite events
		dynamic_cast<core::SpriteSender&>(item).clear();
		dynamic_cast<core::SpriteListener&>(*this).clear();

		// reset collision events
		dynamic_cast<core::CollisionSender&>(collision).clear();
		dynamic_cast<core::CollisionListener&>(movement).clear();
		dynamic_cast<core::CollisionListener&>(action).clear();
		dynamic_cast<core::CollisionListener&>(projectile).clear();

		// reset move events
		dynamic_cast<core::MoveSender&>(movement).clear();
		dynamic_cast<core::MoveSender&>(collision).clear();
		dynamic_cast<core::MoveListener&>(collision).clear();
		dynamic_cast<core::MoveListener&>(focus).clear();
		dynamic_cast<core::MoveListener&>(action).clear();

		// reset input events
		dynamic_cast<core::InputSender&>(input).clear();
		dynamic_cast<core::InputSender&>(action).clear();
		dynamic_cast<core::InputSender&>(interact).clear();
		dynamic_cast<core::InputListener&>(action).clear();
		dynamic_cast<core::InputListener&>(movement).clear();
		dynamic_cast<core::InputListener&>(focus).clear();

		// reset action events
		dynamic_cast<rpg::ActionSender&>(input).clear();
		dynamic_cast<rpg::ActionSender&>(action).clear();
		dynamic_cast<rpg::ActionListener&>(action).clear();
		dynamic_cast<rpg::ActionListener&>(delay).clear();
		dynamic_cast<rpg::ActionListener&>(quickslot).clear();

		// reset item event
		dynamic_cast<rpg::ItemSender&>(interact).clear();
		dynamic_cast<rpg::ItemSender&>(quickslot).clear();
		dynamic_cast<rpg::ItemListener&>(item).clear();

		// reset perk event
		dynamic_cast<rpg::PerkSender&>(perk).clear();
		dynamic_cast<rpg::PerkSender&>(quickslot).clear();
		dynamic_cast<rpg::PerkListener&>(delay).clear();
		dynamic_cast<rpg::PerkListener&>(perk).clear();

		// reset boni event
		dynamic_cast<rpg::BoniSender&>(effect).clear();
		dynamic_cast<rpg::BoniSender&>(item).clear();
		dynamic_cast<rpg::BoniListener&>(stats).clear();

		// reset interact event
		dynamic_cast<rpg::InteractSender&>(delay).clear();
		dynamic_cast<rpg::InteractListener&>(interact).clear();

		// reset combat events
		dynamic_cast<rpg::CombatSender&>(effect).clear();
		dynamic_cast<rpg::CombatSender&>(delay).clear();
		dynamic_cast<rpg::CombatSender&>(projectile).clear();
		dynamic_cast<rpg::CombatListener&>(combat).clear();

		// reset stats events
		dynamic_cast<rpg::StatsSender&>(combat).clear();
		dynamic_cast<rpg::StatsSender&>(item).clear();
		dynamic_cast<rpg::StatsSender&>(stats).clear();
		dynamic_cast<rpg::StatsListener&>(stats).clear();
		dynamic_cast<rpg::StatsListener&>(*this).clear();

		// reset exp events
		dynamic_cast<rpg::ExpSender&>(combat).clear();
		dynamic_cast<rpg::ExpSender&>(player).clear();
		dynamic_cast<rpg::ExpListener&>(player).clear();
		dynamic_cast<rpg::ExpListener&>(stats).clear();
		dynamic_cast<rpg::ExpListener&>(*this).clear();

		// reset effect events
		dynamic_cast<rpg::EffectSender&>(combat).clear();
		dynamic_cast<rpg::EffectListener&>(effect).clear();
		dynamic_cast<rpg::EffectListener&>(effect).clear();

		// reset death events
		dynamic_cast<rpg::DeathSender&>(stats).clear();
		dynamic_cast<rpg::DeathListener&>(action).clear();
		dynamic_cast<rpg::DeathListener&>(*this).clear();

		// reset projectile events
		dynamic_cast<rpg::ProjectileSender&>(combat).clear();
		dynamic_cast<rpg::ProjectileSender&>(delay).clear();
		dynamic_cast<rpg::ProjectileSender&>(projectile).clear();
		dynamic_cast<rpg::ProjectileListener&>(*this).clear();

		// reset quickslot events
		dynamic_cast<rpg::QuickslotSender&>(item).clear();
		dynamic_cast<rpg::QuickslotSender&>(perk).clear();
		dynamic_cast<rpg::QuickslotListener&>(quickslot).clear();

		// reset training events
		dynamic_cast<rpg::TrainingSender&>(player).clear();
		dynamic_cast<rpg::TrainingListener&>(perk).clear();
		dynamic_cast<rpg::TrainingListener&>(stats).clear();

		// reset feedback events
		dynamic_cast<rpg::FeedbackSender&>(quickslot).clear();
		dynamic_cast<rpg::FeedbackSender&>(item).clear();
		dynamic_cast<rpg::FeedbackSender&>(perk).clear();
		dynamic_cast<rpg::FeedbackSender&>(player).clear();
		dynamic_cast<rpg::FeedbackListener&>(*this).clear();

		sprites.clear();
		projectiles.clear();
		exps.clear();
		_stats.clear();
		deaths.clear();
		feedbacks.clear();

		moves.clear();
	}

	void update(sf::Time const& elapsed) {
		core::updateChunked([&](sf::Time const& t) {
			for (auto const& event : moves) {
				movement.receive(event);
				focus.receive(event);
			}
			moves.clear();

			movement.update(t);
			collision.update(t);
			focus.update(t);

			input.update(t);
			action.update(t);
			animation.update(t);

			quickslot.update(t);
			effect.update(t);
			stats.update(t);
			item.update(t);
			perk.update(t);
			interact.update(t);

			delay.update(t);
			projectile.update(t);
			combat.update(t);
			player.update(t);

			dispatch<core::SpriteEvent>(*this);
			dispatch<rpg::ProjectileEvent>(*this);
			dispatch<rpg::ExpEvent>(*this);
			dispatch<rpg::StatsEvent>(*this);
			dispatch<rpg::DeathEvent>(*this);
			dispatch<rpg::FeedbackEvent>(*this);

		}, elapsed, sf::milliseconds(core::MAX_FRAMETIME_MS));

		animation.cleanup();
		render.cleanup();
		movement.cleanup();
		collision.cleanup();
		focus.cleanup();

		stats.cleanup();
		effect.cleanup();
		item.cleanup();
		perk.cleanup();
		player.cleanup();
		projectile.cleanup();

		action.cleanup();
		input.cleanup();
		interact.cleanup();
		quickslot.cleanup();
	}

	void handle(core::SpriteEvent const& event) { sprites.push_back(event); }

	void handle(rpg::ProjectileEvent const& event) {
		projectiles.push_back(event);

		switch (event.type) {
			case rpg::ProjectileEvent::Create:
				createProjectile(event);
				break;

			case rpg::ProjectileEvent::Destroy:
				core::vanish(dungeon[1u], movement.query(event.id));
				destroyObject(event.id);
				utils::pop(objects, event.id);
				break;
		}
	}

	void handle(rpg::ExpEvent const& event) { exps.push_back(event); }
	void handle(rpg::StatsEvent const& event) { _stats.push_back(event); }
	void handle(rpg::DeathEvent const& event) { deaths.push_back(event); }
	void handle(rpg::FeedbackEvent const& event) { feedbacks.push_back(event); }

	void moveObject(
		core::ObjectID id, sf::Vector2i const& move, sf::Vector2i const& look) {
		core::InputEvent event;
		event.actor = id;
		event.move = move;
		event.look = look;
		movement.receive(event);
		focus.receive(event);
	}

	void rotateObject(core::ObjectID id, sf::Vector2i const& look) {
		core::InputEvent event;
		event.actor = id;
		event.look = look;
		focus.receive(event);
	}

	core::ObjectID createObject(
		sf::Vector2u const& pos, sf::Vector2i const& look) {
		auto id = id_manager.acquire();
		objects.push_back(id);
		auto& move_data = movement.acquire(id);
		/*
		move_data.pos = sf::Vector2f{pos};
		move_data.target = pos;
		move_data.scene = 1u;
		*/
		move_data.look = look;
		move_data.max_speed = 50.f;
		core::spawn(dungeon[1u], move_data, pos);
		/*
		auto& d = dungeon[1u];
		d.getCell(pos).entities.push_back(id);
		*/
		auto& focus_data = focus.acquire(id);
		focus_data.look = look;
		focus_data.sight = 10.f;
		focus_data.display_name = "not empty";
		collision.acquire(id);
		auto& ani = animation.acquire(id);
		ani.tpl.torso[core::SpriteTorsoLayer::Base] = &body_sprite.torso;
		render.acquire(id);
		// publish object
		core::MoveEvent event;
		event.actor = id;
		event.target = pos;
		event.type = core::MoveEvent::Left;
		focus.receive(event);

		return id;
	}

	core::ObjectID createBarrier(sf::Vector2u const& pos) {
		auto id = createObject(pos, {0, 1});
		auto& i = interact.acquire(id);
		i.type = rpg::InteractType::Barrier;
		return id;
	}

	core::ObjectID createCorpse(sf::Vector2u const& pos) {
		auto id = createObject(pos, {0, 1});
		auto& i = interact.acquire(id);
		i.type = rpg::InteractType::Corpse;
		return id;
	}

	core::ObjectID createCharacter(
		sf::Vector2u const& pos, sf::Vector2i const& look) {
		auto id = createObject(pos, look);
		action.acquire(id);
		item.acquire(id);
		perk.acquire(id);
		effect.acquire(id);
		auto& s = stats.acquire(id);
		s.level = 10u;
		for (auto& pair : s.attributes) {
			pair.second = 25u;
		}
		for (auto& pair : s.base_props) {
			pair.second = 10u;
		}
		rpg::stats_impl::refresh(s);
		s.stats[rpg::Stat::Life] = s.properties[rpg::Property::MaxLife];
		s.stats[rpg::Stat::Mana] = s.properties[rpg::Property::MaxMana];
		s.stats[rpg::Stat::Stamina] = s.properties[rpg::Property::MaxStamina];
		auto& ani = animation.query(id);
		ani.tpl.legs[core::SpriteLegLayer::Base] = &body_sprite.legs;
		return id;
	}

	core::ObjectID createPlayer(sf::Vector2u const& pos,
		sf::Vector2i const& look, rpg::PlayerID player_id) {
		auto id = createCharacter(pos, look);
		quickslot.acquire(id);
		auto& p = player.acquire(id);
		p.player_id = player_id;
		auto& in = input.acquire(id);
		in.keys = keys;
		return id;
	}

	core::ObjectID createProjectile(rpg::ProjectileEvent const& event) {
		auto spawn = event.spawn;
		if (event.id > 0u) {
			auto& m = movement.query(event.id);
			auto& f = focus.query(event.id);
			auto p = m.pos;
			p.x = std::round(p.x);
			p.y = std::round(p.y);
			// spawn.pos = sf::Vector2u{sf::Vector2i{p} + f.look};
			spawn.pos = sf::Vector2u{p};
			spawn.direction = f.look;
		}

		auto id = createObject(spawn.pos, spawn.direction);
		auto& f = focus.query(id);
		f.sight = 0.f;  // bullet cannot be focused
		auto& c = collision.query(id);
		c.is_projectile = true;
		c.radius = arrow.radius;
		auto& p = projectile.acquire(id);
		p.owner = event.id;
		p.bullet = &arrow;
		p.ignore.push_back(event.id);
		p.meta_data = event.meta_data;
		// schedule movement
		moves.emplace_back();
		auto& ev = moves.back();
		ev.actor = id;
		ev.move = spawn.direction;
		ev.look = spawn.direction;
		return id;
	}

	void destroyObject(core::ObjectID id) {
		if (animation.has(id)) {
			animation.release(id);
		}
		if (render.has(id)) {
			render.release(id);
		}
		if (movement.has(id)) {
			movement.release(id);
		}
		if (collision.has(id)) {
			collision.release(id);
		}
		if (focus.has(id)) {
			focus.release(id);
		}

		if (stats.has(id)) {
			stats.release(id);
		}
		if (effect.has(id)) {
			effect.release(id);
		}
		if (item.has(id)) {
			item.release(id);
		}
		if (perk.has(id)) {
			perk.release(id);
		}
		if (player.has(id)) {
			player.release(id);
		}
		if (projectile.has(id)) {
			projectile.release(id);
		}

		if (action.has(id)) {
			action.release(id);
		}
		if (input.has(id)) {
			input.release(id);
		}
		if (interact.has(id)) {
			interact.release(id);
		}
		if (quickslot.has(id)) {
			quickslot.release(id);
		}
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(gameplay_integration)

BOOST_AUTO_TEST_CASE(player_will_moves_if_arrowkey_is_pressed) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	// trigger input
	fix.setInput(rpg::PlayerAction::MoveS, true);
	fix.update(sf::milliseconds(200));
	// test body
	auto& move = fix.movement.query(id);
	BOOST_CHECK_CLOSE(move.pos.x, 1.f, 0.0001f);
	BOOST_CHECK_GT(move.pos.y, 2.f);
	BOOST_CHECK_VECTOR_EQUAL(move.target, sf::Vector2u(1u, 3u));
	// test ani
	auto& ani = fix.animation.query(id);
	BOOST_CHECK(ani.is_moving);
}

BOOST_AUTO_TEST_CASE(player_will_stop_if_arrowkey_is_released) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	// trigger input
	fix.setInput(rpg::PlayerAction::MoveS, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::MoveS, false);
	fix.update(sf::milliseconds(2000));
	// test body
	auto& move = fix.movement.query(id);
	BOOST_CHECK_VECTOR_CLOSE(move.pos, sf::Vector2f(1.f, 3.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(move.target, sf::Vector2u(1u, 3u));
	// test ani
	auto& ani = fix.animation.query(id);
	BOOST_CHECK(!ani.is_moving);
}

BOOST_AUTO_TEST_CASE(player_will_move_one_tile_if_arrowkeys_are_tapped) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	// trigger input
	fix.setInput(rpg::PlayerAction::MoveS, true);
	fix.setInput(rpg::PlayerAction::MoveE, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::MoveE, false);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::MoveS, false);
	fix.update(sf::milliseconds(1000));
	// test body
	auto& move = fix.movement.query(id);
	BOOST_CHECK_VECTOR_CLOSE(move.pos, sf::Vector2f(2.f, 3.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(move.target, sf::Vector2u(2u, 3u));
	auto& focus = fix.focus.query(id);
	BOOST_CHECK_VECTOR_EQUAL(focus.look, sf::Vector2i(0, 1));
	// test action
	auto& action = fix.action.query(id);
	BOOST_CHECK(!action.moving);
}

BOOST_AUTO_TEST_CASE(player_will_strife_if_move_and_look_are_triggered) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {-1, 0}, 1u);
	// trigger input
	fix.setInput(rpg::PlayerAction::MoveE, true);
	fix.setInput(rpg::PlayerAction::LookN, true);
	fix.update(sf::milliseconds(100));
	// test body
	auto& move = fix.movement.query(id);
	BOOST_CHECK_GT(move.pos.x, 1.f);
	BOOST_CHECK_CLOSE(move.pos.y, 2.f, 0.0001f);
	auto& focus = fix.focus.query(id);
	BOOST_CHECK_VECTOR_EQUAL(focus.look, sf::Vector2i(0, -1));
}

BOOST_AUTO_TEST_CASE(player_will_at_least_look_if_movement_is_impossible) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	// trigger input
	fix.setInput(rpg::PlayerAction::MoveW, true);
	fix.update(sf::milliseconds(100));
	// test body
	auto& move = fix.movement.query(id);
	BOOST_CHECK_VECTOR_CLOSE(move.pos, sf::Vector2f(1.f, 2.f), 0.0001f);
	auto& focus = fix.focus.query(id);
	BOOST_CHECK_VECTOR_EQUAL(focus.look, sf::Vector2i(-1, 0));
}

BOOST_AUTO_TEST_CASE(player_will_not_move_or_look_if_dead) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& action = fix.action.query(id);
	action.dead = true;
	// trigger input
	fix.setInput(rpg::PlayerAction::MoveS, true);
	fix.update(sf::milliseconds(100));
	// test body
	auto& move = fix.movement.query(id);
	BOOST_CHECK_VECTOR_CLOSE(move.pos, sf::Vector2f(1.f, 2.f), 0.0001f);
	auto& focus = fix.focus.query(id);
	BOOST_CHECK_VECTOR_EQUAL(focus.look, sf::Vector2i(1, 0));
}

BOOST_AUTO_TEST_CASE(player_is_stopped_after_collision) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {-1, 0}, 1u);
	// trigger input
	fix.setInput(rpg::PlayerAction::MoveW, true);
	fix.update(sf::milliseconds(100));
	// test body
	auto& move = fix.movement.query(id);
	BOOST_CHECK_VECTOR_CLOSE(move.pos, sf::Vector2f(1.f, 2.f), 0.0001f);
	auto& focus = fix.focus.query(id);
	BOOST_CHECK_VECTOR_EQUAL(focus.look, sf::Vector2i(-1, 0));
	// test ani
	auto& ani = fix.animation.query(id);
	BOOST_CHECK(!ani.is_moving);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(player_can_attack_void) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(100));
	// test ani
	auto& ani = fix.animation.query(id);
	BOOST_CHECK(ani.current != core::AnimationAction::Idle);
	// continue
	fix.setInput(rpg::PlayerAction::Attack, false);
	fix.update(sf::milliseconds(2000));
	// test ani
	BOOST_CHECK(ani.current == core::AnimationAction::Idle);
}

BOOST_AUTO_TEST_CASE(player_cannot_attack_if_dead) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& action = fix.action.query(id);
	action.dead = true;
	auto& ani = fix.animation.query(id);
	ani.current = core::AnimationAction::Die;
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(100));
	// test ani
	BOOST_CHECK(ani.current == core::AnimationAction::Die);
}

BOOST_AUTO_TEST_CASE(player_can_attack_enemy) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto other = fix.createCharacter({2u, 2u}, {0, 1});
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::Attack, false);
	fix.update(sf::milliseconds(3000));
	// expect stats event
	BOOST_REQUIRE_EQUAL(fix._stats.size(), 1u);
	BOOST_CHECK_EQUAL(fix._stats[0].actor, other);
	BOOST_CHECK_EQUAL(fix._stats[0].causer, id);
	BOOST_CHECK_LT(fix._stats[0].delta[rpg::Stat::Life], 0);
}

BOOST_AUTO_TEST_CASE(
	player_does_not_damage_enemy_by_attack_if_dungeon_is_changed) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto other = fix.createCharacter({2u, 2u}, {0, 1});
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(100));
	// change dungeon
	auto& body = fix.movement.query(other);
	body.scene = 2u;
	// wait
	fix.setInput(rpg::PlayerAction::Attack, false);
	fix.update(sf::milliseconds(2000));
	// test target's life
	auto& target = fix.stats.query(other);
	BOOST_CHECK_EQUAL(target.stats[rpg::Stat::Life],
		target.properties[rpg::Property::MaxLife]);
}

BOOST_AUTO_TEST_CASE(player_creates_bullet_when_shooting_by_bow) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& item = fix.item.query(id);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.icebow;
	// trigger input
	BOOST_REQUIRE_EQUAL(fix.projectiles.size(), 0u);
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(10));
	fix.setInput(rpg::PlayerAction::Attack, false);
	fix.update(sf::milliseconds(1000));
	// expect bullet creation
	auto const& events = fix.projectiles;
	BOOST_REQUIRE_EQUAL(events.size(), 1u);
	BOOST_CHECK(events[0].type == rpg::ProjectileEvent::Create);
	BOOST_CHECK_EQUAL(events[0].id, id);
	// expect bullet movement
	BOOST_REQUIRE_EQUAL(fix.objects.size(), 2u);
	auto& bullet_move = fix.movement.query(fix.objects[1u]);
	BOOST_CHECK_CLOSE(bullet_move.pos.y, 2.f, 0.0001f);
	BOOST_CHECK_GT(bullet_move.pos.x, 1.f);
}

BOOST_AUTO_TEST_CASE(player_can_damage_far_enemy_by_shooting_by_bow) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& item = fix.item.query(id);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.icebow;
	auto other = fix.createCharacter({3u, 2u}, {0, 1});
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(10));
	fix.setInput(rpg::PlayerAction::Attack, false);
	fix.update(sf::milliseconds(2000));

	// test target's life
	auto& target = fix.stats.query(other);
	BOOST_CHECK_LT(target.stats[rpg::Stat::Life],
		target.properties[rpg::Property::MaxLife]);
}

BOOST_AUTO_TEST_CASE(player_can_damage_near_enemy_by_shooting_by_bow) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& item = fix.item.query(id);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.icebow;
	auto other = fix.createCharacter({2u, 2u}, {0, 1});
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(10));
	fix.setInput(rpg::PlayerAction::Attack, false);
	fix.update(sf::milliseconds(2000));

	// test target's life
	auto& target = fix.stats.query(other);
	BOOST_CHECK_LT(target.stats[rpg::Stat::Life],
		target.properties[rpg::Property::MaxLife]);
}

BOOST_AUTO_TEST_CASE(player_can_kill_enemy_by_shooting_multiple_times) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& item = fix.item.query(id);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.icebow;
	auto other = fix.createCharacter({3u, 2u}, {0, 1});
	auto& target = fix.stats.query(other);
	target.stats[rpg::Stat::Life] = 100;
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(10000));
	// test target's life
	BOOST_CHECK_EQUAL(target.stats[rpg::Stat::Life], 0);
}

BOOST_AUTO_TEST_CASE(player_can_kill_enemy_by_attacking_multiple_times) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto other = fix.createCharacter({2u, 2u}, {0, 1});
	auto& target = fix.stats.query(other);
	target.stats[rpg::Stat::Life] = 20;
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(20000));
	// test target's life
	BOOST_CHECK_EQUAL(target.stats[rpg::Stat::Life], 0);
}

BOOST_AUTO_TEST_CASE(player_can_attack_enemy_by_bow_while_moving_back) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({5u, 2u}, {1, 0}, 1u);
	auto& item = fix.item.query(id);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.icebow;
	auto other = fix.createCharacter({7u, 2u}, {0, 1});
	auto& target = fix.stats.query(other);
	target.stats[rpg::Stat::Life] = 20;
	// trigger input
	fix.setInput(rpg::PlayerAction::MoveW, true);
	fix.setInput(rpg::PlayerAction::LookE, true);
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(20000));
	// test target's life
	BOOST_CHECK_EQUAL(target.stats[rpg::Stat::Life], 0);
	// and player's position
	auto& body = fix.movement.query(id);
	BOOST_CHECK_VECTOR_CLOSE(body.pos, sf::Vector2f(1.f, 2.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(body.target, sf::Vector2u(1u, 2u));
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(player_increases_defense_by_using_armor) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({5u, 2u}, {1, 0}, 1u);
	auto const & item = fix.item.query(id);
	
	rpg::ItemEvent event;
	event.actor = id;
	event.item = &fix.armor;
	event.type = rpg::ItemEvent::Add;
	event.quantity = 1u;
	fix.item.handle(event);
	event.type = rpg::ItemEvent::Use;
	event.slot = fix.armor.slot;
	fix.item.handle(event);
	
	BOOST_REQUIRE(rpg::hasItem(item, fix.armor, 1u));
	BOOST_REQUIRE_EQUAL(item.equipment[event.slot], &fix.armor);
	
	fix.update(sf::milliseconds(50u));
	
	auto const & stat = fix.stats.query(id);
	BOOST_CHECK_EQUAL(stat.base_def[rpg::DamageType::Blade], 10u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(player_can_select_previous_quickslot) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	// trigger input
	fix.setInput(rpg::PlayerAction::PrevSlot, true);
	fix.update(sf::milliseconds(240));
	fix.setInput(rpg::PlayerAction::PrevSlot, false);
	fix.update(sf::milliseconds(100));
	// test slot_id
	BOOST_CHECK_EQUAL(qslot.slot_id, 1u);
}

BOOST_AUTO_TEST_CASE(player_can_select_next_quickslot) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	// trigger input
	fix.setInput(rpg::PlayerAction::NextSlot, true);
	fix.update(sf::milliseconds(240));
	fix.setInput(rpg::PlayerAction::NextSlot, false);
	fix.update(sf::milliseconds(100));
	// test slot_id
	BOOST_CHECK_EQUAL(qslot.slot_id, 3u);
}

BOOST_AUTO_TEST_CASE(player_can_skip_quickslot_by_holding_key) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	// trigger input
	fix.setInput(rpg::PlayerAction::NextSlot, true);
	fix.update(sf::milliseconds(251));
	fix.setInput(rpg::PlayerAction::NextSlot, false);
	fix.update(sf::milliseconds(100));
	// test slot_id
	BOOST_CHECK_EQUAL(qslot.slot_id, 4u);
}

BOOST_AUTO_TEST_CASE(using_empty_quickslot_will_not_crash) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::UseSlot, false);
	fix.update(sf::milliseconds(100));
}

BOOST_AUTO_TEST_CASE(player_can_use_item_via_quickslot) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.manapotion};
	auto& items = fix.item.query(id);
	rpg::item_impl::addItem(items, fix.manapotion, 1u);
	auto& stats = fix.stats.query(id);
	stats.stats[rpg::Stat::Mana] = 50;
	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::UseSlot, false);
	fix.update(sf::milliseconds(1000));
	// expect increased mana
	BOOST_CHECK_EQUAL(stats.stats[rpg::Stat::Mana], 60);
}

BOOST_AUTO_TEST_CASE(player_can_use_perk) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& perk = fix.perk.query(id);
	perk.perks.emplace_back(fix.healing, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.healing};
	auto& stats = fix.stats.query(id);
	stats.stats[rpg::Stat::Life] = 50;
	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(100));
	// test ani
	auto& ani = fix.animation.query(id);
	BOOST_CHECK(ani.current != core::AnimationAction::Idle);
	// continue
	fix.setInput(rpg::PlayerAction::UseSlot, false);
	fix.update(sf::milliseconds(1000));
	// test ani
	BOOST_CHECK(ani.current == core::AnimationAction::Idle);
}

BOOST_AUTO_TEST_CASE(player_can_use_defensive_perk_via_quickslot) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& perk = fix.perk.query(id);
	perk.perks.emplace_back(fix.healing, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.healing};
	auto& stats = fix.stats.query(id);
	stats.stats[rpg::Stat::Life] = 20;
	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::UseSlot, false);
	fix.update(sf::milliseconds(100000));
	// expect increased life
	BOOST_CHECK_GT(stats.stats[rpg::Stat::Life], 20);
}

BOOST_AUTO_TEST_CASE(player_can_damage_enemy_by_offensive_perk_via_quickslot) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& perk = fix.perk.query(id);
	perk.perks.emplace_back(fix.fireball, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.fireball};
	auto other = fix.createCharacter({3u, 2u}, {0, 1});
	auto& target = fix.stats.query(other);
	auto prev = target.stats[rpg::Stat::Life];
	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::UseSlot, false);
	fix.update(sf::milliseconds(10000));
	// expect target's life decreased
	BOOST_CHECK_LT(target.stats[rpg::Stat::Life], prev);
}

BOOST_AUTO_TEST_CASE(projectile_vanishs_after_object_collision) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({3u, 2u}, {1, 0}, 1u);
	auto& item = fix.item.query(id);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.icebow;
	fix.createCharacter({5u, 2u}, {0, 1});
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.projectiles.clear();
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::Attack, false);
	fix.update(sf::milliseconds(700));
	BOOST_REQUIRE_EQUAL(fix.objects.size(), 3u);
	fix.update(sf::milliseconds(3000));

	BOOST_REQUIRE_EQUAL(fix.objects.size(), 2u);
}

BOOST_AUTO_TEST_CASE(projectile_vanishs_after_tile_collision) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({3u, 2u}, {-1, 0}, 1u);
	auto& item = fix.item.query(id);
	item.equipment[rpg::EquipmentSlot::Weapon] = &fix.icebow;
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.projectiles.clear();
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::Attack, false);
	fix.update(sf::milliseconds(700));
	BOOST_REQUIRE_EQUAL(fix.objects.size(), 2u);
	fix.update(sf::milliseconds(3000));

	BOOST_REQUIRE_EQUAL(fix.objects.size(), 1u);
}

BOOST_AUTO_TEST_CASE(player_cannot_select_prev_slot_if_dead) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;

	auto& action = fix.action.query(id);
	action.dead = true;

	// trigger input
	fix.setInput(rpg::PlayerAction::PrevSlot, true);
	fix.update(sf::milliseconds(100));
	// check slot
	BOOST_CHECK_EQUAL(qslot.slot_id, 2u);
}

BOOST_AUTO_TEST_CASE(player_cannot_select_next_slot_if_dead) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;

	auto& action = fix.action.query(id);
	action.dead = true;

	// trigger input
	fix.setInput(rpg::PlayerAction::NextSlot, true);
	fix.update(sf::milliseconds(100));
	// check slot
	BOOST_CHECK_EQUAL(qslot.slot_id, 2u);
}

BOOST_AUTO_TEST_CASE(player_cannot_use_quickslot_if_dead) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& perk = fix.perk.query(id);
	perk.perks.emplace_back(fix.fireball, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.fireball};

	auto& action = fix.action.query(id);
	action.dead = true;
	auto& ani = fix.animation.query(id);
	ani.current = core::AnimationAction::Die;

	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(100));
	// check animation
	BOOST_CHECK(ani.current == core::AnimationAction::Die);
}

BOOST_AUTO_TEST_CASE(player_cannot_cast_if_not_enough_mana) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& stats = fix.stats.query(id);
	stats.stats[rpg::Stat::Mana] = 0;
	auto& perk = fix.perk.query(id);
	perk.perks.emplace_back(fix.fireball, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.fireball};

	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(50));
	// check animation
	auto& ani = fix.animation.query(id);
	BOOST_CHECK(ani.current == core::AnimationAction::Idle);
	// expect feedback event
	BOOST_CHECK_EQUAL(fix.feedbacks.size(), 1u);
}

BOOST_AUTO_TEST_CASE(player_cannot_quickuse_missing_item) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.manapotion};

	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(250));
	// expect no stats animation
	BOOST_CHECK(fix._stats.empty());
	// expect feedback event
	BOOST_CHECK_EQUAL(fix.feedbacks.size(), 1u);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(player_stops_movement_if_killed) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& stats = fix.stats.query(id);
	stats.stats[rpg::Stat::Life] = 1;

	rpg::ProjectileEvent event;
	event.spawn.scene = 1u;
	event.spawn.pos = {3u, 2u};
	event.spawn.direction = {-1, 0};
	event.meta_data.emitter = rpg::EmitterType::Trap;
	event.meta_data.trap = &fix.trap;
	fix.createProjectile(event);

	// move towards bullet
	fix.setInput(rpg::PlayerAction::MoveE, true);
	fix.update(sf::milliseconds(3000));

	// expect player holds his position
	auto& body = fix.movement.query(id);
	auto pos = body.pos;
	fix.update(sf::milliseconds(1000));
	BOOST_CHECK_VECTOR_CLOSE(body.pos, pos, 0.0001f);
}

BOOST_AUTO_TEST_CASE(player_stops_actions_if_killed) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& stats = fix.stats.query(id);
	stats.stats[rpg::Stat::Life] = 1;

	rpg::ProjectileEvent event;
	event.spawn.scene = 1u;
	event.spawn.pos = {3u, 2u};
	event.spawn.direction = {-1, 0};
	event.meta_data.emitter = rpg::EmitterType::Trap;
	event.meta_data.trap = &fix.trap;
	fix.createProjectile(event);

	// start action
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(3000));
	BOOST_REQUIRE_EQUAL(fix.objects.size(), 1u);
	// expect death
	BOOST_CHECK_EQUAL(fix._stats.size(), 1u);
	BOOST_CHECK_EQUAL(fix.deaths.size(), 1u);
	auto& action = fix.action.query(id);
	BOOST_CHECK(action.dead);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(bullet_is_blocked_by_barrier) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	rpg::ProjectileEvent event;
	event.type = rpg::ProjectileEvent::Create;
	event.spawn.pos = {1u, 2u};
	event.spawn.direction = {1, 0};
	event.meta_data.emitter = rpg::EmitterType::Trap;
	event.meta_data.trap = &fix.trap;
	auto id = fix.createProjectile(event);
	fix.createBarrier({3u, 2u});
	// move projectile
	fix.update(sf::milliseconds(1000));
	// expect projectile destruction
	BOOST_REQUIRE_EQUAL(fix.projectiles.size(), 1u);
	BOOST_CHECK_EQUAL(fix.projectiles[0].id, id);
	BOOST_CHECK(fix.projectiles[0].type == rpg::ProjectileEvent::Destroy);
}

BOOST_AUTO_TEST_CASE(player_can_interact) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	fix.createBarrier({2u, 2u});
	// push barrier
	fix.setInput(rpg::PlayerAction::Interact, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::Interact, false);
	// test ani
	auto const& ani = fix.animation.query(id);
	BOOST_CHECK(ani.current == core::AnimationAction::Use);
	// continue
	fix.update(sf::milliseconds(1000));
	// test ani
	BOOST_CHECK(ani.current == core::AnimationAction::Idle);
}

BOOST_AUTO_TEST_CASE(player_is_not_blocked_after_interact) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	fix.createBarrier({2u, 2u});
	// push barrier
	fix.setInput(rpg::PlayerAction::Interact, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::Interact, false);
	// test action
	auto& action = fix.action.query(id);
	BOOST_REQUIRE(!action.idle);
	// wait until interaction is processed
	fix.update(sf::milliseconds(1000));
	// test action
	fix.update(sf::milliseconds(1500));
	BOOST_REQUIRE(action.idle);
}

BOOST_AUTO_TEST_CASE(player_is_not_blocked_after_failed_interact) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	// push barrier
	fix.setInput(rpg::PlayerAction::Interact, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::Interact, false);
	fix.update(sf::milliseconds(1500));
	// test action
	auto& action = fix.action.query(id);
	BOOST_REQUIRE(action.idle);
}

BOOST_AUTO_TEST_CASE(player_cannot_interact_if_dead) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	fix.createBarrier({2u, 2u});
	auto& action = fix.action.query(id);
	action.dead = true;
	auto& ani = fix.animation.query(id);
	ani.current = core::AnimationAction::Die;
	// trigger input
	fix.setInput(rpg::PlayerAction::Interact, true);
	fix.update(sf::milliseconds(100));
	// test ani
	BOOST_CHECK(ani.current == core::AnimationAction::Die);
}

BOOST_AUTO_TEST_CASE(player_can_push_barrier_but_it_stops_automatically) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto barrier = fix.createBarrier({2u, 2u});
	// push barrier
	fix.setInput(rpg::PlayerAction::Interact, true);
	fix.update(sf::milliseconds(3000));
	// expect barrier's new position
	auto& body = fix.movement.query(barrier);
	BOOST_CHECK_VECTOR_CLOSE(body.pos, sf::Vector2f(3.f, 2.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(body.target, sf::Vector2u(3u, 2u));
}

BOOST_AUTO_TEST_CASE(player_cannot_push_barrier_towards_wall) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	fix.createPlayer({2u, 2u}, {-1, 0}, 1u);
	auto barrier = fix.createBarrier({1u, 2u});
	// try to push barrier
	fix.setInput(rpg::PlayerAction::Interact, true);
	fix.update(sf::milliseconds(1000));
	// expect barrier's new position
	auto& body = fix.movement.query(barrier);
	BOOST_CHECK_VECTOR_CLOSE(body.pos, sf::Vector2f(1.f, 2.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(body.target, sf::Vector2u(1u, 2u));
	auto& i = fix.interact.query(barrier);
	BOOST_CHECK(!i.moving);
}

BOOST_AUTO_TEST_CASE(player_cannot_push_barrier_towards_object) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	fix.createPlayer({2u, 2u}, {1, 0}, 1u);
	auto barrier = fix.createBarrier({3u, 2u});
	fix.createBarrier({4u, 2u});
	// try to push barrier
	fix.setInput(rpg::PlayerAction::Interact, true);
	fix.update(sf::milliseconds(2000));
	// expect barrier's new position
	auto& body = fix.movement.query(barrier);
	BOOST_CHECK_VECTOR_CLOSE(body.pos, sf::Vector2f(3.f, 2.f), 0.0001f);
	BOOST_CHECK_VECTOR_EQUAL(body.target, sf::Vector2u(3u, 2u));
	auto& i = fix.interact.query(barrier);
	BOOST_CHECK(!i.moving);
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(player_gains_exp_for_attack) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	fix.createCharacter({2u, 2u}, {0, 1});
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(1000));
	// expect exp event
	BOOST_REQUIRE_EQUAL(fix.exps.size(), 1u);
	BOOST_CHECK_EQUAL(fix.exps[0].actor, id);
}

BOOST_AUTO_TEST_CASE(player_can_levelup) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	fix.createCharacter({2u, 2u}, {0, 1});
	auto& player = fix.player.query(id);
	player.exp = 999999ul;
	// trigger input
	fix.setInput(rpg::PlayerAction::Attack, true);
	fix.update(sf::milliseconds(1000));
	// expect exp event
	BOOST_REQUIRE_EQUAL(fix.exps.size(), 1u);
	BOOST_CHECK_EQUAL(fix.exps[0].actor, id);
	BOOST_CHECK_GE(fix.exps[0].levelup, 1u);
}

BOOST_AUTO_TEST_CASE(player_can_train_attribute) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& player = fix.player.query(id);
	player.attrib_points = 2u;
	auto& stats = fix.stats.query(id);
	auto strength = stats.attributes[rpg::Attribute::Strength];
	// trigger training
	rpg::TrainingEvent event;
	event.actor = id;
	event.type = rpg::TrainingEvent::Attrib;
	event.attrib = rpg::Attribute::Strength;
	fix.player.receive(event);
	fix.update(sf::milliseconds(100));
	// expect dexreased attrib points
	BOOST_CHECK_EQUAL(player.attrib_points, 1u);
	// expect increased strength
	BOOST_CHECK_EQUAL(
		stats.attributes[rpg::Attribute::Strength], strength + 1u);
}

BOOST_AUTO_TEST_CASE(player_can_train_perk) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& player = fix.player.query(id);
	player.perk_points = 2u;
	auto& perk = fix.perk.query(id);
	perk.perks.emplace_back(fix.fireball, 1u);
	// trigger training
	rpg::TrainingEvent event;
	event.actor = id;
	event.type = rpg::TrainingEvent::Perk;
	event.perk = &fix.fireball;
	fix.player.receive(event);
	fix.update(sf::milliseconds(200));
	// expect dexreased perk points
	BOOST_CHECK_EQUAL(player.perk_points, 1u);
	// expect increased fireball level
	BOOST_CHECK_EQUAL(rpg::getPerkLevel(perk, fix.fireball), 2u);
}

BOOST_AUTO_TEST_CASE(player_cannot_train_attribute_without_attrib_points) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& stats = fix.stats.query(id);
	auto strength = stats.attributes[rpg::Attribute::Strength];
	// trigger training
	rpg::TrainingEvent event;
	event.actor = id;
	event.type = rpg::TrainingEvent::Attrib;
	event.attrib = rpg::Attribute::Strength;
	fix.player.receive(event);
	fix.update(sf::milliseconds(100));
	// expect same strength
	BOOST_CHECK_EQUAL(stats.attributes[rpg::Attribute::Strength], strength);
	// expect feedback event
	BOOST_CHECK_EQUAL(fix.feedbacks.size(), 1u);
}

BOOST_AUTO_TEST_CASE(player_cannot_train_perk_withou_perk_points) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& perk = fix.perk.query(id);
	perk.perks.emplace_back(fix.fireball, 1u);
	// trigger training
	rpg::TrainingEvent event;
	event.actor = id;
	event.type = rpg::TrainingEvent::Perk;
	event.perk = &fix.fireball;
	fix.player.receive(event);
	fix.update(sf::milliseconds(200));
	// expect same fireball level
	BOOST_CHECK_EQUAL(rpg::getPerkLevel(perk, fix.fireball), 1u);
	// expect feedback event
	BOOST_CHECK_EQUAL(fix.feedbacks.size(), 1u);
}

BOOST_AUTO_TEST_CASE(player_can_equip_via_shortcut) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.icebow};
	auto& item = fix.item.query(id);
	rpg::item_impl::addItem(item, fix.icebow, 1u);
	// trigger quickuse
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(249));
	// expect weapon equipped
	BOOST_CHECK_EQUAL(item.equipment[rpg::EquipmentSlot::Weapon], &fix.icebow);
	// expect sprite event
	BOOST_REQUIRE_EQUAL(fix.sprites.size(), 1u);
}

BOOST_AUTO_TEST_CASE(player_uses_mana_to_cast) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& perk = fix.perk.query(id);
	perk.perks.emplace_back(fix.fireball, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.fireball};
	auto& stats = fix.stats.query(id);
	stats.stats[rpg::Stat::Mana] = 50;
	// trigger input
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(100));
	fix.setInput(rpg::PlayerAction::UseSlot, false);
	fix.update(sf::milliseconds(250));
	// expect mana consume
	BOOST_REQUIRE_EQUAL(fix._stats.size(), 1u);
	BOOST_CHECK_EQUAL(fix._stats[0].actor, id);
	auto mana = fix._stats[0].delta[rpg::Stat::Mana];
	BOOST_CHECK_LT(mana, 0);
	// expect decreased mana
	BOOST_CHECK_EQUAL(stats.stats[rpg::Stat::Mana], 50 + mana);
}

BOOST_AUTO_TEST_CASE(shortcut_is_cleared_if_last_item_is_quickused) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({1u, 2u}, {1, 0}, 1u);
	auto& qslot = fix.quickslot.query(id);
	qslot.slot_id = 2u;
	qslot.slots[2u] = {fix.manapotion};
	auto& item = fix.item.query(id);
	rpg::item_impl::addItem(item, fix.manapotion, 1u);
	// trigger quickuse
	fix.setInput(rpg::PlayerAction::UseSlot, true);
	fix.update(sf::milliseconds(499));
	// expect empty slot
	BOOST_CHECK(qslot.slots[2u].item == nullptr);
}

BOOST_AUTO_TEST_CASE(player_can_loot_corpse) {
	auto& fix = Singleton<GameplayFixture>::get();
	fix.reset();

	auto id = fix.createPlayer({2u, 2u}, {1, 0}, 1u);
	auto& item = fix.item.query(id);
	item.inventory[rpg::ItemType::Potion].emplace_back(fix.manapotion, 1u);
	auto other = fix.createCorpse({3u, 2u});
	auto& corpse = fix.interact.query(other);
	corpse.loot.resize(1u);
	corpse.loot[0].emplace_back(fix.icebow, 1u);
	corpse.loot[0].emplace_back(fix.manapotion, 5u);
	// interact with corpse
	fix.setInput(rpg::PlayerAction::Interact, true);
	fix.update(sf::milliseconds(800));
	// expect items looted
	BOOST_CHECK(rpg::hasItem(item, fix.icebow, 1u));
	BOOST_CHECK(rpg::hasItem(item, fix.manapotion, 6u));
}

BOOST_AUTO_TEST_SUITE_END()
