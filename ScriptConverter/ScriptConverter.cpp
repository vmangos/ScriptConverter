// Converter for EventAI scripts from CMaNGOS to VMaNGOS
// Author: brotalnia
//

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <set>

#include "Database\Database.h"
#include "Defines\CMaNGOS.h"
#include "Defines\VMaNGOS.h"

std::string EscapeString(char const* unescapedString)
{
    char* escapedString = new char[strlen(unescapedString) * 2 + 1];
    mysql_escape_string(escapedString, unescapedString, strlen(unescapedString));
    std::string returnString = escapedString;
    delete[] escapedString;
    return returnString;
}

Database GameDb;

std::string MakeConnectionString()
{
    std::string mysql_host;
    std::string mysql_port;
    std::string mysql_user;
    std::string mysql_pass;
    std::string mysql_db;

    printf("Host: ");
    getline(std::cin, mysql_host);
    if (mysql_host.empty())
        mysql_host = "127.0.0.1";

    printf("Port: ");
    getline(std::cin, mysql_port);
    if (mysql_port.empty())
        mysql_port = "3306";

    printf("User: ");
    getline(std::cin, mysql_user);
    if (mysql_user.empty())
        mysql_user = "root";

    printf("Password: ");
    getline(std::cin, mysql_pass);
    if (mysql_pass.empty())
        mysql_pass = "root";

    printf("Database: ");
    getline(std::cin, mysql_db);
    if (mysql_db.empty())
        mysql_db = "mangos";

    return mysql_host + ";" + mysql_port + ";" + mysql_user + ";" + mysql_pass + ";" + mysql_db;
}

std::unordered_map<uint32, CMaNGOS::EventAISummon> g_summonPositions;
std::unordered_map<uint32, std::string> g_creatureNames;

bool ProcessEvent(uint32 id, uint32& event_type, int32& event_param1, int32& event_param2, int32& event_param3, int32& event_param4, uint32& condition_id)
{
    if (event_type == CMaNGOS::EVENT_T_DEATH)
    {
        condition_id = event_param1;
        event_param1 = 0;
    }
    else if (event_type == CMaNGOS::EVENT_T_SPAWNED)
    {
        if (event_param1 || event_param2)
        {
            event_param1 = 0;
            event_param2 = 0;
            printf("Warning: Entry %u uses EVENT_T_SPAWNED with unsupported parameters. You need to assign a condition to this entry.\n", id);
        }
    }
    else if (event_type == CMaNGOS::EVENT_T_RECEIVE_EMOTE)
    {
        condition_id = event_param2;
        event_param2 = 0;
    }
    else if (event_type == CMaNGOS::EVENT_T_TIMER_GENERIC)
    {
        event_type = VMaNGOS::EVENT_T_TIMER;
        printf("Warning: Entry %u uses unsupported EVENT_T_TIMER_GENERIC. Changing event type to EVENT_T_TIMER.\n", id);
    }
    else if (event_type > CMaNGOS::EVENT_T_TARGET_MISSING_AURA)
        return false;

    return true;
}

void ConvertTargetType(uint32 targetType, uint32& outTargetType, uint32& outTargetParam1, uint32 outTargetParam2, uint32* outFlags)
{
    switch (targetType)
    {
        case CMaNGOS::TARGET_T_SELF:
            outTargetType = VMaNGOS::TARGET_T_PROVIDED_TARGET;
            if (outFlags)
                *outFlags = VMaNGOS::SF_GENERAL_TARGET_SELF;
            break;
        case CMaNGOS::TARGET_T_HOSTILE:
        case CMaNGOS::TARGET_T_PLAYER_INVOKER:
        case CMaNGOS::TARGET_T_PLAYER_TAPPED:
        case CMaNGOS::TARGET_T_NONE:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE;
            break;
        case CMaNGOS::TARGET_T_HOSTILE_SECOND_AGGRO:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_SECOND_AGGRO;
            break;
        case CMaNGOS::TARGET_T_HOSTILE_LAST_AGGRO:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_LAST_AGGRO;
            break;
        case CMaNGOS::TARGET_T_HOSTILE_RANDOM:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_RANDOM;
            break;
        case CMaNGOS::TARGET_T_HOSTILE_RANDOM_NOT_TOP:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_RANDOM_NOT_TOP;
            break;
        case CMaNGOS::TARGET_T_ACTION_INVOKER:
        case CMaNGOS::TARGET_T_EVENT_SENDER:
        case CMaNGOS::TARGET_T_EVENT_SPECIFIC:
            // should not happen for timer events
            outTargetType = VMaNGOS::TARGET_T_PROVIDED_TARGET;
            break;
        case CMaNGOS::TARGET_T_ACTION_INVOKER_OWNER:
        case CMaNGOS::TARGET_T_SPAWNER:
            outTargetType = VMaNGOS::TARGET_T_OWNER;
            break;
        case CMaNGOS::TARGET_T_HOSTILE_RANDOM_PLAYER:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_RANDOM;
            outTargetParam1 = VMaNGOS::SELECT_FLAG_PLAYER;
            break;
        case CMaNGOS::TARGET_T_HOSTILE_RANDOM_NOT_TOP_PLAYER:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_RANDOM_NOT_TOP;
            outTargetParam1 = VMaNGOS::SELECT_FLAG_PLAYER;
            break;
        case CMaNGOS::TARGET_T_HOSTILE_RANDOM_MANA:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_RANDOM;
            outTargetParam1 = VMaNGOS::SELECT_FLAG_NO_TOTEM | VMaNGOS::SELECT_FLAG_POWER_MANA;
            break;
        case CMaNGOS::TARGET_T_NEAREST_AOE_TARGET:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_RANDOM;
            break;
        case CMaNGOS::TARGET_T_HOSTILE_FARTHEST_AWAY:
            outTargetType = VMaNGOS::TARGET_T_HOSTILE_RANDOM;
            outTargetParam1 = VMaNGOS::SELECT_FLAG_NO_TOTEM | VMaNGOS::SELECT_FLAG_NOT_IN_MELEE_RANGE;
            break;
        default:
            printf("Error: Cannot convert spell target type %u!\n", targetType);
            break;
    }
}

bool g_textsWarning = false;

VMaNGOS::ScriptInfo* ProcessAction(uint32 id, uint32 action_type, int32 action_param1, int32 action_param2, int32 action_param3)
{
    VMaNGOS::ScriptInfo* pScriptInfo;

    switch (action_type)
    {
        case CMaNGOS::ACTION_T_TEXT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_TALK;
            pScriptInfo->talk.textId[0] = action_param1;
            pScriptInfo->talk.textId[1] = action_param2;
            pScriptInfo->talk.textId[2] = action_param3;
            pScriptInfo->comment = "Say Text";
            g_textsWarning = true;
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_FACTION:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_FACTION;
            pScriptInfo->faction.factionId = action_param1;
            pScriptInfo->faction.flags = action_param2;
            pScriptInfo->comment = "Set Faction";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_MORPH_TO_ENTRY_OR_MODEL:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL;

            if (action_param2) // modelId
            {
                pScriptInfo->morph.creatureOrModelEntry = action_param2;
                pScriptInfo->morph.isDisplayId = 1;
            }
            else
                pScriptInfo->morph.creatureOrModelEntry = action_param1;

            pScriptInfo->comment = "Morph";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SOUND:
        case CMaNGOS::ACTION_T_RANDOM_SOUND:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_PLAY_SOUND;
            pScriptInfo->playSound.soundId = action_param1;
            pScriptInfo->comment = "Play Sound";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_EMOTE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_EMOTE;
            pScriptInfo->emote.emoteId[0] = action_param1;
            pScriptInfo->comment = "Emote";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_RANDOM_SAY:
            return nullptr;
        case CMaNGOS::ACTION_T_RANDOM_YELL:
            return nullptr;
        case CMaNGOS::ACTION_T_RANDOM_TEXTEMOTE:
            return nullptr;
        case CMaNGOS::ACTION_T_RANDOM_EMOTE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_EMOTE;
            pScriptInfo->emote.emoteId[0] = action_param1;
            pScriptInfo->emote.emoteId[1] = action_param2;
            pScriptInfo->emote.emoteId[2] = action_param3;
            pScriptInfo->comment = "Random Emote";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_CAST:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_CAST_SPELL;
            pScriptInfo->castSpell.spellId = action_param1;

            ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);

            if (action_param3 & CMaNGOS::CAST_INTERRUPT_PREVIOUS)
                pScriptInfo->castSpell.flags |= VMaNGOS::CF_INTERRUPT_PREVIOUS;

            if (action_param3 & CMaNGOS::CAST_TRIGGERED)
                pScriptInfo->castSpell.flags |= VMaNGOS::CF_TRIGGERED;

            if (action_param3 & CMaNGOS::CAST_FORCE_CAST)
                pScriptInfo->castSpell.flags |= VMaNGOS::CF_FORCE_CAST;

            if (action_param3 & CMaNGOS::CAST_FORCE_TARGET_SELF)
                pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_TARGET_SELF;

            if (action_param3 & CMaNGOS::CAST_AURA_NOT_PRESENT)
                pScriptInfo->castSpell.flags |= VMaNGOS::CF_AURA_NOT_PRESENT;

            if (action_param3 & CMaNGOS::CAST_SWITCH_CASTER_TARGET)
                pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_SWAP_FINAL_TARGETS;
            
            if (action_param3 & CMaNGOS::CAST_MAIN_SPELL)
                pScriptInfo->castSpell.flags |= VMaNGOS::CF_MAIN_RANGED_SPELL;

            pScriptInfo->comment = "Cast Spell";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SPAWN:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_TEMP_SUMMON_CREATURE;
            pScriptInfo->summonCreature.creatureEntry = action_param1;
            pScriptInfo->summonCreature.attackTarget = VMaNGOS::TARGET_T_PROVIDED_TARGET;
            ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);

            if (action_param3) // despawn delay
                pScriptInfo->summonCreature.despawnType = VMaNGOS::TEMPSUMMON_TIMED_OR_DEAD_DESPAWN;
            else
                pScriptInfo->summonCreature.despawnType = VMaNGOS::TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT;

            pScriptInfo->summonCreature.despawnDelay = action_param3;
            pScriptInfo->comment = "Summon Creature";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_THREAT_SINGLE_PCT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MODIFY_THREAT;
            pScriptInfo->x = float(action_param1); // threat

            // target
            pScriptInfo->modThreat.target = VMaNGOS::TARGET_T_PROVIDED_TARGET;
            ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            
            pScriptInfo->comment = "Modify Target Threat";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_THREAT_ALL_PCT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MODIFY_THREAT;
            pScriptInfo->x = float(action_param1); // threat
            pScriptInfo->modThreat.target = VMaNGOS::SO_MODIFYTHREAT_ALL_ATTACKERS;
            pScriptInfo->comment = "Modify All Threat";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_QUEST_EVENT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_QUEST_EXPLORED;
            pScriptInfo->questExplored.questId = action_param1;
            ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            pScriptInfo->questExplored.group = action_param3;
            pScriptInfo->comment = "Complete Quest";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_CAST_EVENT:
            return nullptr;
        case CMaNGOS::ACTION_T_SET_UNIT_FIELD:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_FIELD_SET;
            pScriptInfo->setField.fieldId = action_param1;
            pScriptInfo->setField.fieldValue = action_param2;
            ConvertTargetType(action_param3, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_SWAP_FINAL_TARGETS;
            pScriptInfo->comment = "Set Field";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_UNIT_FLAG:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MODIFY_FLAGS;
            pScriptInfo->modFlags.fieldId = UNIT_FIELD_FLAGS;
            pScriptInfo->modFlags.fieldValue = action_param1;
            pScriptInfo->modFlags.mode = VMaNGOS::SO_MODIFYFLAGS_SET;
            ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_SWAP_FINAL_TARGETS;
            pScriptInfo->comment = "Set Flag";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_REMOVE_UNIT_FLAG:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MODIFY_FLAGS;
            pScriptInfo->modFlags.fieldId = UNIT_FIELD_FLAGS;
            pScriptInfo->modFlags.fieldValue = action_param1;
            pScriptInfo->modFlags.mode = VMaNGOS::SO_MODIFYFLAGS_REMOVE;
            ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_SWAP_FINAL_TARGETS;
            pScriptInfo->comment = "Remove Flag";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_AUTO_ATTACK:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_MELEE_ATTACK;
            pScriptInfo->enableMelee.enabled = action_param1;
            
            if (action_param1)
                pScriptInfo->comment = "Enable Melee Attack";
            else
                pScriptInfo->comment = "Disable Melee Attack";

            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_COMBAT_MOVEMENT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_COMBAT_MOVEMENT;
            pScriptInfo->combatMovement.enabled = action_param1;
            
            if (action_param1)
                pScriptInfo->comment = "Enable Combat Movement";
            else
                pScriptInfo->comment = "Disable Combat Movement";

            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_PHASE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_PHASE;
            int32 step = action_param1;

            if (step > 0)
            {
                pScriptInfo->setPhase.phase = step;
                pScriptInfo->setPhase.mode = VMaNGOS::SO_SETPHASE_INCREMENT;
                pScriptInfo->comment = "Increment Phase";
            }
            else
            {
                pScriptInfo->setPhase.phase = -step;
                pScriptInfo->setPhase.mode = VMaNGOS::SO_SETPHASE_DECREMENT;
                pScriptInfo->comment = "Decrement Phase";
            }

            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_EVADE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_ENTER_EVADE_MODE;
            pScriptInfo->comment = "Enter Evade Mode";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_FLEE_FOR_ASSIST:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_FLEE;
            pScriptInfo->comment = "Flee";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_QUEST_EVENT_ALL:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_QUEST_EXPLORED;
            pScriptInfo->questExplored.questId = action_param1;
            pScriptInfo->questExplored.group = 1;
            pScriptInfo->comment = "Complete Quest For All";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_CAST_EVENT_ALL:
            return nullptr;
        case CMaNGOS::ACTION_T_REMOVEAURASFROMSPELL:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_REMOVE_AURA;
            pScriptInfo->removeAura.spellId = action_param2;
            ConvertTargetType(action_param1, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_SWAP_FINAL_TARGETS;
            pScriptInfo->comment = "Remove Aura";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_RANGED_MOVEMENT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MOVEMENT;
            pScriptInfo->movement.movementType = VMaNGOS::CHASE_MOTION_TYPE;
            pScriptInfo->x = float(action_param1); // distance
            pScriptInfo->o = float(action_param2); // angle
            pScriptInfo->comment = "Set Ranged Movement";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_RANDOM_PHASE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_PHASE_RANDOM;
            pScriptInfo->setPhaseRandom.phase[0] = action_param1;
            pScriptInfo->setPhaseRandom.phase[1] = action_param2;
            pScriptInfo->setPhaseRandom.phase[2] = action_param3;
            pScriptInfo->comment = "Random Phase";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_RANDOM_PHASE_RANGE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_PHASE_RANGE;
            pScriptInfo->setPhaseRange.phaseMin = action_param1;
            pScriptInfo->setPhaseRange.phaseMax = action_param2;
            pScriptInfo->comment = "Random Phase between " + std::to_string(action_param1) + " and " + std::to_string(action_param2);
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SUMMON_ID:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_TEMP_SUMMON_CREATURE;
            pScriptInfo->summonCreature.creatureEntry = action_param1;

            if (action_param2 != CMaNGOS::TARGET_T_SELF)
            {
                pScriptInfo->summonCreature.attackTarget = VMaNGOS::TARGET_T_PROVIDED_TARGET;
                ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            }
            else
                pScriptInfo->summonCreature.attackTarget = -1;
            
            auto i = g_summonPositions.find(action_param3);
            if (i == g_summonPositions.end())
                break;

            pScriptInfo->summonCreature.despawnDelay = (*i).second.SpawnTimeSecs; // despawn_delay

            // type different if no despawn delay
            if (pScriptInfo->summonCreature.despawnDelay)
                pScriptInfo->summonCreature.despawnType = VMaNGOS::TEMPSUMMON_TIMED_OR_DEAD_DESPAWN;
            else
                pScriptInfo->summonCreature.despawnType = VMaNGOS::TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT;

            pScriptInfo->x = (*i).second.position_x;
            pScriptInfo->y = (*i).second.position_y;
            pScriptInfo->z = (*i).second.position_z;
            pScriptInfo->o = (*i).second.orientation;

            pScriptInfo->comment = "Summon Creature";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_KILLED_MONSTER:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_KILL_CREDIT;
            pScriptInfo->killCredit.creatureEntry = action_param1;
            ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_SWAP_FINAL_TARGETS;
            pScriptInfo->comment = "Grant Kill Credit";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_INST_DATA:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_INST_DATA;
            pScriptInfo->setData.field = action_param1;
            pScriptInfo->setData.data = action_param2;
            pScriptInfo->setData.type = VMaNGOS::SO_INSTDATA_RAW;
            pScriptInfo->comment = "Set Data";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_INST_DATA64:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_INST_DATA64;
            pScriptInfo->setData64.field = action_param1;
            ConvertTargetType(action_param2, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_SWAP_FINAL_TARGETS;
            pScriptInfo->setData64.type = VMaNGOS::SO_INSTDATA64_SOURCE_GUID;
            pScriptInfo->comment = "Set Data 64";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_UPDATE_TEMPLATE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_UPDATE_ENTRY;
            pScriptInfo->updateEntry.creatureEntry = action_param1;
            pScriptInfo->comment = "Update Entry";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_DIE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_DEAL_DAMAGE;
            pScriptInfo->dealDamage.damage = 100;
            pScriptInfo->dealDamage.isPercent = 1;
            pScriptInfo->raw.data[4] |= VMaNGOS::SF_GENERAL_TARGET_SELF;
            pScriptInfo->comment = "Die";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_ZONE_COMBAT_PULSE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_ZONE_COMBAT_PULSE;
            pScriptInfo->combatPulse.initialPulse = 1;
            pScriptInfo->comment = "Combat Pulse";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_CALL_FOR_HELP:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_CALL_FOR_HELP;
            pScriptInfo->x = float(action_param1);
            pScriptInfo->comment = "Call For Help";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_SHEATH:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_SHEATH;
            pScriptInfo->setSheath.sheathState = action_param1;
            pScriptInfo->comment = "Set Sheath State";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_FORCE_DESPAWN:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_DESPAWN_CREATURE;
            pScriptInfo->despawn.despawnDelay = action_param1;
            pScriptInfo->comment = "Despawn Self";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_INVINCIBILITY_HP_LEVEL:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_INVINCIBILITY;
            pScriptInfo->invincibility.health = action_param1;
            pScriptInfo->invincibility.isPercent = action_param2;
            pScriptInfo->comment = "Set Invincibility at " + std::to_string(action_param1) + (action_param2 ? "%" : "HP");
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_MOUNT_TO_ENTRY_OR_MODEL:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL;

            if (action_param2) // modelId
            {
                pScriptInfo->mount.creatureOrModelEntry = action_param2;
                pScriptInfo->mount.isDisplayId = 1;
            }
            else
                pScriptInfo->mount.creatureOrModelEntry = action_param1;

            if (pScriptInfo->mount.creatureOrModelEntry)
                pScriptInfo->comment = "Mount";
            else
                pScriptInfo->comment = "Dismount";

            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_CHANCED_TEXT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_TALK;
            pScriptInfo->talk.textId[0] = action_param2;
            pScriptInfo->talk.textId[1] = action_param3;
            pScriptInfo->comment = "Say Text";
            g_textsWarning = true;
            printf("Warning: Entry %u uses unsupported ACTION_T_CHANCED_TEXT. Replacing with SCRIPT_COMMAND_TALK.\n", id);
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_THROW_AI_EVENT:
            return nullptr;
        case CMaNGOS::ACTION_T_SET_THROW_MASK:
            return nullptr;
        case CMaNGOS::ACTION_T_SET_STAND_STATE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_STAND_STATE;
            pScriptInfo->standState.standState = action_param1;
            pScriptInfo->comment = "Set Stand State";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_CHANGE_MOVEMENT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MOVEMENT;
            pScriptInfo->movement.movementType = action_param1;
            pScriptInfo->movement.clear = 1;

            if (action_param1 == VMaNGOS::RANDOM_MOTION_TYPE)
            {
                // Wander Distance
                if (action_param2 != 0)
                    pScriptInfo->x = float(action_param2);

                // From Current Position
                pScriptInfo->movement.boolParam = 1;
            }

            pScriptInfo->comment = "Set Movement Type";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_DYNAMIC_MOVEMENT:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_COMBAT_MOVEMENT;
            pScriptInfo->combatMovement.enabled = action_param1;
            
            if (action_param1)
                pScriptInfo->comment = "Enable Combat Movement";
            else
                pScriptInfo->comment = "Disable Combat Movement";

            printf("Warning: Entry %u uses unsupported ACTION_T_DYNAMIC_MOVEMENT. Replacing with SCRIPT_COMMAND_SET_COMBAT_MOVEMENT.\n", id);
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_REACT_STATE:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_SET_REACT_STATE;
            pScriptInfo->setReactState.state = action_param1;
            pScriptInfo->comment = "Set React State";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_PAUSE_WAYPOINTS:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_MOVEMENT;
            pScriptInfo->movement.clear = 1;

            if (action_param1)
            {
                pScriptInfo->movement.movementType = VMaNGOS::IDLE_MOTION_TYPE;
                pScriptInfo->comment = "Move Idle";
            }
            else
            {
                pScriptInfo->movement.movementType = VMaNGOS::WAYPOINT_MOTION_TYPE;
                pScriptInfo->comment = "Start Waypoint Movement";
            }

            printf("Warning: Entry %u uses unsupported ACTION_T_PAUSE_WAYPOINTS. Replacing with SCRIPT_COMMAND_MOVEMENT.\n", id);
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_INTERRUPT_SPELL:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_INTERRUPT_CASTS;
            pScriptInfo->comment = "Interrupt Casts";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_START_RELAY_SCRIPT:
            return nullptr;
        case CMaNGOS::ACTION_T_TEXT_NEW:
            return nullptr;
        case CMaNGOS::ACTION_T_ATTACK_START:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_ATTACK_START;
            ConvertTargetType(action_param1, pScriptInfo->target_type, pScriptInfo->target_param1, pScriptInfo->target_param2, &pScriptInfo->raw.data[4]);
            pScriptInfo->comment = "Attack Start";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_DESPAWN_GUARDIANS:
        {
            pScriptInfo = new VMaNGOS::ScriptInfo();
            pScriptInfo->command = VMaNGOS::SCRIPT_COMMAND_REMOVE_GUARDIANS;
            pScriptInfo->removeGuardian.creatureId = action_param1;
            pScriptInfo->comment = "Despawn Guardians";
            return pScriptInfo;
        }
        case CMaNGOS::ACTION_T_SET_RANGED_MODE:
            return nullptr;
    }

    return nullptr;
}

std::unordered_map<uint32 /*creatureId*/, std::vector<VMaNGOS::CreatureSpellsEntry>> g_creatureSpellLists;

void AddSpellCastToCreatureSpellsList(uint32 creatureId, uint32 eventChance, uint32 eventFlags, uint32 spellId, uint32 targetType, uint32 castFlags, uint32 delayInitialMin, uint32 delayInitialMax, uint32 delayRepeatMin, uint32 delayRepeatMax)
{
    VMaNGOS::CreatureSpellsEntry spellList;
    spellList.spellId = spellId;
    spellList.probability = eventChance;
    spellList.delayInitialMin = delayInitialMin / 1000;
    spellList.delayInitialMax = delayInitialMax / 1000;
    spellList.delayRepeatMin = delayRepeatMin / 1000;
    spellList.delayRepeatMax = delayRepeatMax / 1000;

    ConvertTargetType(targetType, spellList.castTarget, spellList.targetParam1, spellList.targetParam2, nullptr);

    if (castFlags & CMaNGOS::CAST_INTERRUPT_PREVIOUS)
        spellList.castFlags |= VMaNGOS::CF_INTERRUPT_PREVIOUS;
    if (castFlags & CMaNGOS::CAST_TRIGGERED)
        spellList.castFlags |= VMaNGOS::CF_TRIGGERED;
    if (castFlags & CMaNGOS::CAST_FORCE_CAST)
        spellList.castFlags |= VMaNGOS::CF_FORCE_CAST;
    if (castFlags & CMaNGOS::CAST_NO_MELEE_IF_OOM)
        printf("Error: Unsupported cast flag CAST_NO_MELEE_IF_OOM for spell %u!\n", spellId);
    if (castFlags & CMaNGOS::CAST_FORCE_TARGET_SELF)
    {
        spellList.castTarget = VMaNGOS::TARGET_T_PROVIDED_TARGET;
        spellList.targetParam1 = 0;
        spellList.targetParam2 = 0;
    }
    if (castFlags & CMaNGOS::CAST_AURA_NOT_PRESENT)
        spellList.castFlags |= VMaNGOS::CF_AURA_NOT_PRESENT;
    if (castFlags & CMaNGOS::CAST_IGNORE_UNSELECTABLE_TARGET)
        printf("Error: Unsupported cast flag CAST_IGNORE_UNSELECTABLE_TARGET for spell %u!\n", spellId);
    if (castFlags & CMaNGOS::CAST_SWITCH_CASTER_TARGET)
        printf("Error: Unsupported cast flag CAST_SWITCH_CASTER_TARGET for spell %u!\n", spellId);
    if (castFlags & CMaNGOS::CAST_MAIN_SPELL)
        spellList.castFlags |= VMaNGOS::CF_MAIN_RANGED_SPELL;
    if (castFlags & CMaNGOS::CAST_PLAYER_ONLY)
    {
        switch (spellList.castTarget)
        {
            case VMaNGOS::TARGET_T_HOSTILE_SECOND_AGGRO:
            case VMaNGOS::TARGET_T_HOSTILE_LAST_AGGRO:
            case VMaNGOS::TARGET_T_HOSTILE_RANDOM:
            case VMaNGOS::TARGET_T_HOSTILE_RANDOM_NOT_TOP:
                // these are the target types that support cast flags
                break;
            default:
                spellList.castTarget = VMaNGOS::TARGET_T_HOSTILE_RANDOM;
                spellList.targetParam1 = 0;
                spellList.targetParam2 = 0;
                break;
        }

        spellList.targetParam1 |= VMaNGOS::SELECT_FLAG_PLAYER;
    }
    if (castFlags & CMaNGOS::CAST_DISTANCE_YOURSELF)
        printf("Error: Unsupported cast flag CAST_DISTANCE_YOURSELF for spell %u!\n", spellId);
    if (castFlags & CMaNGOS::CAST_TARGET_CASTING)
        spellList.castFlags |= VMaNGOS::CF_TARGET_CASTING;
    if (castFlags & CMaNGOS::CAST_ONLY_XYZ)
        printf("Error: Unsupported cast flag CAST_ONLY_XYZ for spell %u!\n", spellId);

    if (eventFlags & CMaNGOS::EFLAG_RANGED_MODE_ONLY)
        spellList.castFlags |= VMaNGOS::CF_NOT_IN_MELEE;
    if (eventFlags & CMaNGOS::EFLAG_MELEE_MODE_ONLY)
    {
        spellList.castFlags |= VMaNGOS::CF_ONLY_IN_MELEE;
        if (spellList.castFlags & VMaNGOS::CF_MAIN_RANGED_SPELL)
            spellList.castFlags -= VMaNGOS::CF_MAIN_RANGED_SPELL;
    }

    g_creatureSpellLists[creatureId].push_back(spellList);
}

std::set<uint32> g_processedCreatures; // to know if comment was written at begin exporting data for creature id

void ExportCreatureSpellList(std::ofstream& myfile, uint32 creatureId)
{
    auto itr = g_creatureSpellLists.find(creatureId);
    if (itr != g_creatureSpellLists.end())
    {
        if (g_processedCreatures.find(creatureId) == g_processedCreatures.end())
        {
            myfile << "-- Spell list for " << g_creatureNames[creatureId] << "\n";
            g_processedCreatures.insert(creatureId);
        }

        myfile << "INSERT INTO `creature_spells` (`entry`, `name`, ";
        for (size_t i = 0; i < itr->second.size(); i++)
        {
            if (i != 0)
                myfile << ", ";
            myfile << "`spellId_" + std::to_string(i + 1) + "`, `probability_" + std::to_string(i + 1) + "`, `castTarget_" + std::to_string(i + 1) + "`, `targetParam1_" + std::to_string(i + 1) + "`, `targetParam2_" + std::to_string(i + 1) + "`, `castFlags_" + std::to_string(i + 1) + "`, `delayInitialMin_" + std::to_string(i + 1) + "`, `delayInitialMax_" + std::to_string(i + 1) + "`, `delayRepeatMin_" + std::to_string(i + 1) + "`, `delayRepeatMax_" + std::to_string(i + 1) + "`";
        }

        myfile << ") VALUES (" << creatureId * 10 << ", 'ZonePlaceholder - " << EscapeString(g_creatureNames[creatureId].c_str()) << "', ";
        for (size_t i = 0; i < itr->second.size(); i++)
        {
            if (i != 0)
                myfile << ", ";

            VMaNGOS::CreatureSpellsEntry& spellList = itr->second[i];
            myfile << spellList.spellId << ", " << spellList.probability << ", " << spellList.castTarget << ", " << spellList.targetParam1 << ", " << spellList.targetParam2 << ", " << spellList.castFlags << ", " << spellList.delayInitialMin << ", " << spellList.delayInitialMax << ", " << spellList.delayRepeatMin << ", " << spellList.delayRepeatMax;
        }
        myfile << ");\n";
    }
}

// We need separate non-coflicting ids for events that choose a random action. Hopefully no creature has more than 50 events.
#define RANDOM_ACTION_OFFSET 50

bool IsNumber(const std::string &s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

int main()
{
    std::ofstream myfile("converted_scripts.sql");
    if (!myfile.is_open())
        return 1;

    printf("This tool converts CMaNGOS EventAI scripts to the format used in VMaNGOS.\n");
    printf("If you only want to convert the script of a single creature, enter its id.\n");
    printf("Leave this empty if you wish to convert the entire scripts table instead.\n");
    printf("Creature Id: ");

    std::string chosen_creature;
    getline(std::cin, chosen_creature);

    //                           0     1              2             3                           4               5              6               7               8               9               10              11                12                13                14              15                16                17                18              19                20                21                22
    std::string query = "SELECT `id`, `creature_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_type`, `action1_param1`, `action1_param2`, `action1_param3`, `action2_type`, `action2_param1`, `action2_param2`, `action2_param3`, `action3_type`, `action3_param1`, `action3_param2`, `action3_param3`, `comment` FROM `creature_ai_scripts`";
    if (IsNumber(chosen_creature))
        query += " WHERE `creature_id` = " + chosen_creature;
    query += " ORDER BY creature_id, id";

    printf("\nEnter your database connection info.\n");
    std::string const connection_string = MakeConnectionString();

    printf("\nConnecting to database.\n");
    if (!GameDb.Initialize(connection_string.c_str()))
    {
        printf("\nError: Cannot connect to world database!\n");
        getchar();
        return 1;
    }

    // Load creature names to use in the action comment.
    printf("Loading creature names.\n");
    if (std::shared_ptr<QueryResult> result = GameDb.Query("SELECT `entry`, `name` FROM `creature_template`"))
    {
        do
        {
            DbField* pFields = result->fetchCurrentRow();

            uint32 entry = pFields[0].getUInt32();
            std::string name = pFields[1].getCppString();

            g_creatureNames[entry] = name;
        } while (result->NextRow());
    }

    // Load summons table.
    printf("Loading summons table.\n");
    if (std::shared_ptr<QueryResult> result = GameDb.Query("SELECT `id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs` FROM `creature_ai_summons`"))
    {
        do
        {
            DbField* pFields = result->fetchCurrentRow();

            CMaNGOS::EventAISummon temp;

            temp.id = pFields[0].getUInt32();
            temp.position_x = pFields[1].getFloat();
            temp.position_y = pFields[2].getFloat();
            temp.position_z = pFields[3].getFloat();
            temp.orientation = pFields[4].getFloat();
            temp.SpawnTimeSecs = pFields[5].getUInt32();

            g_summonPositions[temp.id] = temp;
        } while (result->NextRow());
    }

    uint32 totalCount = 0;
    uint32 lastCreatureId = 0;
    printf("Converting scripts.\n");
    if (std::shared_ptr<QueryResult> result = GameDb.Query(query.c_str()))
    {
        do
        {
            DbField* pFields = result->fetchCurrentRow();

            uint32 const id = pFields[0].getUInt32();
            uint32 const creature_id = pFields[1].getUInt32();

            // Export creature_spells data once we are done with all events for this creature id.
            if (lastCreatureId && lastCreatureId != creature_id)
            {
                ExportCreatureSpellList(myfile, lastCreatureId);
                myfile << "\n";
            }

            uint32 event_type = pFields[2].getUInt32();
            uint32 const event_inverse_phase_mask = pFields[3].getUInt32();
            uint32 const event_chance = pFields[4].getUInt32();
            uint32 event_flags = pFields[5].getUInt32();

            int32 event_param1 = pFields[6].getInt32();
            int32 event_param2 = pFields[7].getInt32();
            int32 event_param3 = pFields[8].getInt32();
            int32 event_param4 = pFields[9].getInt32();
            
            uint32 condition_id = 0;

            if (!ProcessEvent(id, event_type, event_param1, event_param2, event_param3, event_param4, condition_id))
            {
                printf("Error: Entry %u uses unsupported event %u, skipping. \n", id, event_type);
                lastCreatureId = creature_id;
                continue;
            }
            
            std::vector<std::unique_ptr<VMaNGOS::ScriptInfo>> vScriptActions;

            uint32 const action1_type = pFields[10].getUInt32();
            int32 const action1_param1 = pFields[11].getInt32();
            int32 const action1_param2 = pFields[12].getInt32();
            int32 const action1_param3 = pFields[13].getInt32();

            uint32 const action2_type = pFields[14].getUInt32();
            int32 const action2_param1 = pFields[15].getInt32();
            int32 const action2_param2 = pFields[16].getInt32();
            int32 const action2_param3 = pFields[17].getInt32();

            uint32 const action3_type = pFields[18].getUInt32();
            int32 const action3_param1 = pFields[19].getInt32();
            int32 const action3_param2 = pFields[20].getInt32();
            int32 const action3_param3 = pFields[21].getInt32();

            if (event_type == VMaNGOS::EVENT_T_TIMER && action1_type == CMaNGOS::ACTION_T_CAST && !action2_type && !action3_type && !event_inverse_phase_mask && (event_flags & VMaNGOS::EFLAG_REPEATABLE))
            {
                AddSpellCastToCreatureSpellsList(creature_id, event_chance, event_flags, action1_param1, action1_param2, action1_param3, event_param1, event_param2, event_param3, event_param4);
                lastCreatureId = creature_id;
                continue;
            }

            if (action1_type)
            {
                if (auto pScriptInfo = ProcessAction(id, action1_type, action1_param1, action1_param2, action1_param3))
                    vScriptActions.push_back(std::unique_ptr<VMaNGOS::ScriptInfo>(pScriptInfo));
                else
                    printf("Error: Entry %u uses unsupported action type %u for first action, skipping.\n", id, action1_type);
            }

            if (action2_type)
            {
                if (auto pScriptInfo = ProcessAction(id, action2_type, action2_param1, action2_param2, action2_param3))
                    vScriptActions.push_back(std::unique_ptr<VMaNGOS::ScriptInfo>(pScriptInfo));
                else
                    printf("Error: Entry %u uses unsupported action type %u for second action, skipping.\n", id, action2_type);
            }
                

            if (action3_type)
            {
                if (auto pScriptInfo = ProcessAction(id, action3_type, action3_param1, action3_param2, action3_param3))
                    vScriptActions.push_back(std::unique_ptr<VMaNGOS::ScriptInfo>(pScriptInfo));
                else
                    printf("Error: Entry %u uses unsupported action type %u for third action, skipping.\n", id, action3_type);
            }

            if (vScriptActions.empty())
            {
                printf("Error: Entry %u has no actions or all actions are unsupported.\n", id);
                lastCreatureId = creature_id;
                continue;
            }

            if (g_processedCreatures.find(creature_id) == g_processedCreatures.end())
            {
                myfile << "-- Events list for " << g_creatureNames[creature_id] << "\n";
                g_processedCreatures.insert(creature_id);
            }
            myfile << "INSERT INTO `creature_ai_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `datalong3`, `datalong4`, `target_param1`, `target_param2`, `target_type`, `data_flags`, `dataint`, `dataint2`, `dataint3`, `dataint4`, `x`, `y`, `z`, `o`, `condition_id`, `comments`) VALUES\n";
            
            uint8 i = 0;
            uint32 script_id = id;
            uint32 action_script[3] = { id, 0, 0 };
            uint8 actions_count = vScriptActions.size();
            bool random_actions = ((event_flags & CMaNGOS::EFLAG_RANDOM_ACTION) && (actions_count > 1));

            for (auto const& pScriptAction : vScriptActions)
            {
                pScriptAction->comment = g_creatureNames[creature_id] + " - " + pScriptAction->comment;
                pScriptAction->comment = EscapeString(pScriptAction->comment.c_str());

                if (random_actions)
                {
                    script_id = id + RANDOM_ACTION_OFFSET + i;
                    action_script[i] = script_id;
                }

                myfile << "(" << script_id << ", " <<
                    0 << ", " << // delay
                    pScriptAction->command << ", " <<
                    pScriptAction->raw.data[0] << ", " <<
                    pScriptAction->raw.data[1] << ", " <<
                    pScriptAction->raw.data[2] << ", " <<
                    pScriptAction->raw.data[3] << ", " <<
                    pScriptAction->target_param1 << ", " <<
                    pScriptAction->target_param2 << ", " <<
                    pScriptAction->target_type << ", " <<
                    pScriptAction->raw.data[4] << ", " <<
                    (int32)pScriptAction->raw.data[5] << ", " <<
                    (int32)pScriptAction->raw.data[6] << ", " <<
                    (int32)pScriptAction->raw.data[7] << ", " <<
                    (int32)pScriptAction->raw.data[8] << ", " <<
                    pScriptAction->x << ", " <<
                    pScriptAction->y << ", " <<
                    pScriptAction->z << ", " <<
                    pScriptAction->o << ", " <<
                    pScriptAction->condition << ", " <<
                    "'" << pScriptAction->comment << "')" << (actions_count == (i + 1) ? ";" : ",") << "\n";
                i++;
            }

            // Correct event flags since they have different values.
            uint32 new_flags = 0;
            if (event_flags & CMaNGOS::EFLAG_REPEATABLE)
                new_flags = VMaNGOS::EFLAG_REPEATABLE;
            if (event_flags & CMaNGOS::EFLAG_RANDOM_ACTION)
                new_flags |= VMaNGOS::EFLAG_RANDOM_ACTION;
            if (event_flags & CMaNGOS::EFLAG_DEBUG_ONLY)
                new_flags |= VMaNGOS::EFLAG_DEBUG_ONLY;

            // Escape the comment.
            std::string comment = pFields[22].getCppString();
            comment = EscapeString(comment.c_str());

            myfile << "INSERT INTO `creature_ai_events` (`id`, `creature_id`, `condition_id`, `event_type`, `event_inverse_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action1_script`, `action2_script`, `action3_script`, `comment`) VALUES (" <<
                id << ", " <<
                creature_id << ", " <<
                condition_id << ", " <<
                event_type << ", " <<
                event_inverse_phase_mask << ", " <<
                event_chance << ", " <<
                new_flags << ", " <<
                event_param1 << ", " <<
                event_param2 << ", " <<
                event_param3 << ", " <<
                event_param4 << ", " <<
                action_script[0] << ", " <<
                action_script[1] << ", " <<
                action_script[2] << ", " <<
                "'" << comment << "');\n";

            totalCount++;
            lastCreatureId = creature_id;
        } while (result->NextRow());
    }

    // Export spell list data for the last creature processed.
    if (lastCreatureId)
        ExportCreatureSpellList(myfile, lastCreatureId);

    printf("\nDone! Converted %u creature events and their associated actions.", totalCount);
    if (g_textsWarning)
        printf("\nWarning: Texts used in ACTION_T_TEXT need to be replaced with broadcast Ids.");
    if (!g_creatureSpellLists.empty())
        printf("\nWarning: Remember to assign zone name in `creature_spells` table.");
    getchar();

    myfile.close();
    GameDb.Uninitialise();
    return 0;
}

