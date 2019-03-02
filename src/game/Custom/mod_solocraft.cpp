#include <map>

#include "Config.h"
#include "ScriptMgr.h"
#include "Unit.h"
#include "Player.h"
#include "Pet.h"
#include "Map.h"
#include "Group.h"
#include "InstanceData.h"

/*
 * TODO:
 * 1. Dispel target regeneration
 * 2. Provide unlimited http://www.wowhead.com/item=17333/aqual-quintessence
 */

namespace {

	class solocraft_player_instance_handler : public Script 
	{
	public:

		void OnLogin(Player *player) 
		{
			ChatHandler(player->GetSession()).SendSysMessage("This server is running a Solocraft Module.");
		}

		void OnPlayerEnter(Player* player)
		{
				Map *map = player->GetMap();
				int difficulty = CalculateDifficulty(map, player);
				int numInGroup = GetNumInGroup(player);
				ApplyBuffs(player, map, difficulty, numInGroup);
		}
	private:
		std::map<uint32, int> _unitDifficulty;

		int CalculateDifficulty(Map *map, Player *player) {
			int difficulty = 1;
			if (map) 
			{
				if (map->IsRaid()) {
					difficulty = 40;
				}
				else if (map->IsDungeon()) {
					difficulty = 5;
				}
			}
			return difficulty;
		}

		int GetNumInGroup(Player *player) {
			int numInGroup = 1;
			Group *group = player->GetGroup();
			if (group) {
				Group::MemberSlotList const& groupMembers = group->GetMemberSlots();
				numInGroup = groupMembers.size();
			}
			return numInGroup;
		}

		void ApplyBuffs(Player *player, Map *map, int difficulty, int numInGroup) {
			ClearBuffs(player, map);
			if (difficulty > 1) {
				//InstanceMap *instanceMap = map->ToInstanceMap();
				//InstanceScript *instanceScript = instanceMap->GetInstanceScript();

				ChatHandler(player->GetSession()).PSendSysMessage("Entered %s (difficulty = %d, numInGroup = %d)",
					map->GetMapName(), difficulty, numInGroup);

				_unitDifficulty[player->GetGUIDLow()] = difficulty;
				for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i) {
					player->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(difficulty * 100), true);
				}
				player->SetMaxHealth(player->GetMaxHealth());
				if (player->GetPowerType() == POWER_MANA) {
					player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
				}
			}
		}

		void ClearBuffs(Player *player, Map *map) {
			std::map<uint32, int>::iterator unitDifficultyIterator = _unitDifficulty.find(player->GetGUIDLow());
			if (unitDifficultyIterator != _unitDifficulty.end()) {
				int difficulty = unitDifficultyIterator->second;
				_unitDifficulty.erase(unitDifficultyIterator);

				ChatHandler(player->GetSession()).PSendSysMessage("Left to %s (removing difficulty = %d)",
					map->GetMapName(), difficulty);

				for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i) {
					player->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(difficulty * 100), false);
				}
			}
		}
	};
}
