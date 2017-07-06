called = "";
args = {};

onInit = function(self, is_hostile)
	called = "onInit";
	args = {
		is_hostile = is_hostile
	};
end

onObjectCollision = function(self, other, pos)
	called = "onObjectCollision";
	args = {
		other = other,
		pos = pos
	};
end

onTileCollision = function(self, pos)
	called = "onTileCollision";
	args = {
		pos = pos
	};
end

onIdle = function(self)
	called = "onIdle";
	args = {};
end

onTileLeft = function(self, pos)
	called = "onTileLeft";
	args = {
		pos = pos
	};
end

onTileReached = function(self, pos)
	called = "onTileReached";
	args = {
		pos = pos
	};
end

onGotFocus = function(self, target)
	called = "onGotFocus";
	args = {
		target = target
	};
end

onLostFocus = function(self, target)
	called = "onLostFocus";
	args = {
		target = target
	}
end

onWasFocused = function(self, observer)
	called = "onWasFocused";
	args = {
		observer = observer
	};
end

onWasUnfocused = function(self, observer)
	called = "onWasUnfocused";
	args = {
		observer = observer
	};
end

onEffectReceived = function(self, effect, causer)
	called = "onEffectReceived";
	args = {
		effect = effect.display_name,
		causer = causer
	};
end

onEffectInflicted = function(self, effect, target)
	called = "onEffectInflicted";
	args = {
		effect = effect.display_name,
		target = target
	};
end

onEffectFaded = function(self, effect)
	called = "onEffectFaded";
	args = {
		effect = effect.display_name
	};
end

onStatsReceived = function(self, life, mana, stamina, causer)
	called = "onStatsReceived";
	args = {
		life = life,
		mana = mana,
		stamina = stamina,
		causer = causer
	};
end

onStatsInflicted = function(self, life, mana, stamina, target)
	called = "onStatsInflicted";
	args = {
		life = life,
		mana = mana,
		stamina = stamina,
		target = target
	};
end

onEnemyKilled = function(self, target)
	called = "onEnemyKilled";
	args = {
		target = target
	};
end

onDeath = function(self, enemy)
	called = "onDeath";
	args = {
		enemy = enemy
	};
end

onUpdate = function(self)
	called = "onUpdate";
	args = {};
end
