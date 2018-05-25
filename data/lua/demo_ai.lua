MoveAgent = {};
function MoveAgent.new()
	local self = {};
	self.do_move = false;
	self.pathfind = false;
	self.pos = Position.new(0, 0);
	
	self.move = function(self, pos)
		self.do_move = true;
		if self.pos.x ~= pos.x or self.pos.y ~= pos.y then
			self.pathfind = false;
		end
		self.pos = pos;
	end
	
	self.stop = function(self)
		self.do_move = false;
	end
	
	self.update = function(self, api)
		if not self.do_move then
			return
		end
		
		if self.pathfind then
			-- pathfind to given position
			api:navigate(self.pos)
		else
			-- directly move towards given position
			api:lookTowards(self.pos);
			api:moveTowards(self.pos);
		end
	end
	
	return self;
end

-----------------------------------------------------------------------

Flock = {};
function Flock.new()
	local self = {};
	self.entities = {};
	self.enemy = 0;
	self.angry = false;
	
	self.cooldown = math.random() * 30; -- set cooldown for first update
	
	self.register = function(self, api)
		self.entities[api.id] = MoveAgent.new();
	end
	
	self.unregister = function(self, api)
		self.entities[api.id] = nil;
	end
	
	self.getLeader = function(self, api)
		for id, flag in pairs(self.entities) do
			if flag ~= nil and api:isAlive(id) then
				return id;
			end
		end
		return 0;
	end
	
	self.searchEnemy = function(self, api)
		self.enemy = 0;
		enemies = api:getEnemies();
		min_dist = api:getSight() + 1;
		for _, other in ipairs(enemies) do
			dist = api:getDistance(other)
			if dist < min_dist and api:isAlive(other) then
				min_dist = dist;
				self.enemy = other;
			end
		end
	end
	
	self.stayClose = function(self, api)
		local center = Position.new(0, 0);
		local origin = api:getPosition(api.id);
		local scene = api:getScene(api.id);
		n = 0;
		-- iterate all entities inside this flock to determine the flock's center
		for id, flag in pairs(self.entities) do
			if flag ~= nil and id ~= api.id and scene == api:getScene(id) then
				pos = api:getPosition(id);
				center.x = center.x + pos.x;
				center.y = center.y + pos.y;
				n = n + 1;
			end
		end
		-- trigger movement to direction
		if n > 0 then
			center.x = math.ceil(center.x / n);
			center.y = math.ceil(center.y / n);
			self.entities[api.id]:move(center);
		end
	end
	
	self.leadFlock = function(self, api)
		-- search new enemy if necessary
		if self.enemy == 0 then
			self:searchEnemy(api);
		end
	end
	
	self.setEnemy = function(self, api, id)
		if self.enemy > 0 then
			return;
		end
		if id == 0 then
			return;
		end
		if api:isHostile(api.id) == api:isHostile(id) then
			return;
		end
		self.enemy = id;
		self.angry = true;
	end
	
	self.update = function(self, api)
		if self.cooldown > 0 then
			self.cooldown = self.cooldown - 1;
			return;
		end
		
		local is_leader = self:getLeader(api) == api.id;
		if is_leader then
			self:leadFlock(api);
		end
		
		-- try to hunt enemy
		local hunt = false;
		if self.enemy > 0 then
			local dist = api:getDistance(self.enemy);
			local sight = api:getSight();
			if self.angry and dist <= sight * 0.5 then
				-- not angry anymore if got pretty close to enemy
				self.angry = false;
			end
			if self.angry or dist <= sight then
				-- hunt enemy!
				local mv = self.entities[api.id];
				mv:move(api:getPosition(self.enemy));
				mv.pathfind = true;
				hunt = true;
			else
				-- ignore enemy
				self.enemy = 0;
			end
		end
		
		if not is_leader and not hunt then
			-- stay close to the flock
			self:stayClose(api)
		end
	end
	
	return self;
end

-----------------------------------------------------------------------

-- shared state
memory = {};
flock = Flock.new();

-----------------------------------------------------------------------

onInit = function(api)
	memory[api.id] = {
		pathfind = false,
		best = {
			melee = nil,
			range = nil,
			armor = nil,
			shield = nil,
			helmet = nil,
			perk = nil
		}
	};
	
	local best = memory[api.id].best;
	best.melee = chooseWeapon(api, true);
	best.range = chooseWeapon(api, false);
	best.armor = chooseArmor(api, EquipmentSlot.Body);
	best.shield = chooseArmor(api, EquipmentSlot.Extension);
	best.helmet = chooseArmor(api, EquipmentSlot.Head);
	best.perk = choosePerk(api, PerkType.Enemy);
	
	local best_melee = 0;
	local best_range = 0;
	if best.melee ~= nil then
		best_melee = getTotalDamage(api:getWeaponDamage(best.melee));
	end
	if best.range ~= nil then
		best_range = getTotalDamage(api:getWeaponDamage(best.range));
	end
	if best_range > best_melee then
		-- => best_range > 0, thus best.range ~= nil
		api:useItem(best.range);
	elseif best.melee ~= nil then
		api:useItem(best.melee);
	end
	
	if best.armor ~= nil then
		api:useItem(best.armor);
	end
	if best.shield ~= nil then
		api:useItem(best.shield);
	end
	if best.helmet ~= nil then
		api:useItem(best.helmet);
	end
end

onObjectCollision = function(api, other, pos)
	flock.entities[api.id].pathfind = true;
end

onTileCollision = function(api, pos)
	flock.entities[api.id].pathfind = true;
end

onTeleport = function(api, src_scene, src_pos, dst_scene, dst_pos)
end

onIdle = function(api)
end

onStart = function(api)
end

onStop = function(api)
end

onEffectReceived = function(api, effect, actor)
end

onEffectInflicted = function(api, effect, target)
end

onEffectFaded = function(api, effect)
end

onStatsReceived = function(api, life, mana, stamina, causer)
	if life < 0 then
		flock:setEnemy(api, causer);
	end
end

onStatsInflicted = function(api, life, mana, stamina, target)
end

onEnemyKilled = function(api, target)
end

onDeath = function(api, enemy)
	flock:umregister(api);
end

onSpawned = function(api, allied)
	flock:register(api);
end

onCausedSpawn = function(api, allied)
end

onFeedback = function(api, type_)
	if type_ == FeedbackType.NotEnoughMana then
		-- reset perk decision
		memory[api.id].best.perk = nil;
	end
end

onPathFailed = function(api, pos)
end

onUpdate = function(api)
	flock:update(api);
	
	local walk = true;
	if flock.enemy > 0 then
		-- look towards enemy
		local pos = api:getPosition(flock.enemy);
		api:lookTowards(pos);
		
		-- determine combat distance
		local dist = api:getDistance(flock.enemy);
		local combat_dist = 2.5;
		local weapon = api:getEquipment(EquipmentSlot.Weapon);
		local changed_weapon = false;
		local best = memory[api.id].best;
		if dist > combat_dist then
			-- range weapon would be useful
			if best.range ~= nil and (weapon == nil or weapon.melee) then
				-- has range weapon and current weapon is melee (or fists)
				api:useItem(best.range);
				changed_weapon = true;
			end
		else
			-- melee weapon would be useful
			if best.melee ~= nil and (weapon == nil or not weapon.melee) then
				-- has melee weapon and current weapon is range weapon (or fists)
				api:useItem(best.melee);
				changed_weapon = true;
			end
		end
		-- note: changing weapon needs some time, cannot attack now
		if not changed_weapon and api:getFocus(api.id) == flock.enemy then
			local perk = best.perk;
			if perk ~= nil or (weapon ~= nil and not weapon.melee) then
				-- using perk or range weapon
				-- implies: combat distance == 3 * range of sight
				combat_dist = 3 * api:getSight();
				walk = dist > combat_dist * 0.5; -- walk until half combat distance
			end
			
			if dist <= combat_dist then
				-- face and attack enemy
				if perk ~= nil then
					api:usePerk(perk);
				else
					api:attack();
				end
			end
		end
	end
	
	if walk then
		flock.entities[api.id]:update(api);
	end
end

-----------------------------------------------------------------------

getTotalDamage = function(damage_map)
	local sum = 0;
	for _, value in pairs(damage_map) do
		sum = sum + value;
	end
	return sum;
end

chooseWeapon = function(api, is_melee)
	local weapons = api:getWeapons();
	local best_index = 0;
	local best_sum = 0;
	local found = false;
	for i, weapon in ipairs(weapons) do
		if weapon.item.melee == is_melee then
			local sum = getTotalDamage(api:getWeaponDamage(weapon.item));
			if (sum > best_sum) then
				found = true;
				best_sum = sum;
				best_index = i;
			end
		end
	end
	if found then
		return weapons[best_index].item;
	end
	return nil;
end

chooseArmor = function(api, slot)
	local armors = api:getArmors();
	local best_index = 0;
	local best_sum = 0;
	local found = false;
	for i, armor in ipairs(armors) do
		if (armor.item.slot == slot) then
			local sum = getTotalDamage(armor.item.boni.defense);
			if (sum > best_sum) then
				found = true;
				best_sum = sum
				best_index = i;
			end
		end
	end
	if found then
		return armors[best_index].item;
	end;
	return nil;
end

choosePerk = function(api)
	local perks = api:getPerks();
	local best_index = 0;
	local best_sum = 0;
	local found = false;
	for i, perk in ipairs(perks) do
		if (perk.perk.type == 1) then -- enum constant for perk towards enemy
			local sum = getTotalDamage(api:getPerkDamage(perk.perk));
			if (sum > best_sum) then
				found = true;
				best_sum = sum;
				best_index = i;
			end
		end
	end
	if found then
		return perks[best_index].perk;
	end
	return nil;
end
