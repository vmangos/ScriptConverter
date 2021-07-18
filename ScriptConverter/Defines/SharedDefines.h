#include "Common.h"

enum SpellTarget
{
    TARGET_NONE                                        = 0,
    TARGET_UNIT_CASTER                                 = 1,
    TARGET_UNIT_ENEMY_NEAR_CASTER                      = 2,
    TARGET_UNIT_FRIEND_NEAR_CASTER                     = 3,
    TARGET_UNIT_NEAR_CASTER                            = 4,
    TARGET_UNIT_CASTER_PET                             = 5,
    TARGET_UNIT_ENEMY                                  = 6,
    TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC            = 7,
    TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC           = 8,
    TARGET_LOCATION_CASTER_HOME_BIND                   = 9,
    TARGET_LOCATION_CASTER_DIVINE_BIND_NYI             = 10,
    TARGET_PLAYER_NYI                                  = 11,
    TARGET_PLAYER_NEAR_CASTER_NYI                      = 12,
    TARGET_PLAYER_ENEMY_NYI                            = 13,
    TARGET_PLAYER_FRIEND_NYI                           = 14,
    TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC             = 15,
    TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC            = 16,
    TARGET_LOCATION_DATABASE                           = 17,
    TARGET_LOCATION_CASTER_DEST                        = 18,
    TARGET_UNK_19                                      = 19,
    TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE        = 20,
    TARGET_UNIT_FRIEND                                 = 21,
    TARGET_LOCATION_CASTER_SRC                         = 22,
    TARGET_GAMEOBJECT                                  = 23,
    TARGET_ENUM_UNITS_ENEMY_IN_CONE_24                 = 24,
    TARGET_UNIT                                        = 25,
    TARGET_LOCKED                                      = 26,
    TARGET_UNIT_CASTER_MASTER                          = 27,
    TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC          = 28,
    TARGET_ENUM_UNITS_FRIEND_AOE_AT_DYNOBJ_LOC         = 29,
    TARGET_ENUM_UNITS_FRIEND_AOE_AT_SRC_LOC            = 30,
    TARGET_ENUM_UNITS_FRIEND_AOE_AT_DEST_LOC           = 31,
    TARGET_LOCATION_UNIT_MINION_POSITION               = 32,
    TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC             = 33,
    TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC            = 34,
    TARGET_UNIT_PARTY                                  = 35,
    TARGET_ENUM_UNITS_ENEMY_WITHIN_CASTER_RANGE        = 36, // TODO: only used with dest-effects - reinvestigate naming
    TARGET_UNIT_FRIEND_AND_PARTY                       = 37,
    TARGET_UNIT_SCRIPT_NEAR_CASTER                     = 38,
    TARGET_LOCATION_CASTER_FISHING_SPOT                = 39,
    TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER               = 40,
    TARGET_LOCATION_CASTER_FRONT_RIGHT                 = 41,
    TARGET_LOCATION_CASTER_BACK_RIGHT                  = 42,
    TARGET_LOCATION_CASTER_BACK_LEFT                   = 43,
    TARGET_LOCATION_CASTER_FRONT_LEFT                  = 44,
    TARGET_UNIT_FRIEND_CHAIN_HEAL                      = 45,
    TARGET_LOCATION_SCRIPT_NEAR_CASTER                 = 46,
    TARGET_LOCATION_CASTER_FRONT                       = 47,
    TARGET_LOCATION_CASTER_BACK                        = 48,
    TARGET_LOCATION_CASTER_LEFT                        = 49,
    TARGET_LOCATION_CASTER_RIGHT                       = 50,
    TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_SRC_LOC      = 51,
    TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC     = 52,
    TARGET_LOCATION_CASTER_TARGET_POSITION             = 53,
    TARGET_ENUM_UNITS_ENEMY_IN_CONE_54                 = 54,
    TARGET_LOCATION_CASTER_FRONT_LEAP                  = 55,
    TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE         = 56,
    TARGET_UNIT_RAID                                   = 57,
    TARGET_UNIT_RAID_NEAR_CASTER                       = 58,
    TARGET_ENUM_UNITS_FRIEND_IN_CONE                   = 59,
    TARGET_ENUM_UNITS_SCRIPT_IN_CONE_60                = 60,
    TARGET_UNIT_RAID_AND_CLASS                         = 61,
    TARGET_PLAYER_RAID_NYI                             = 62,
    TARGET_LOCATION_UNIT_POSITION                      = 63,

    MAX_SPELL_TARGETS
};

enum SpellEffectIndex
{
    EFFECT_INDEX_0 = 0,
    EFFECT_INDEX_1 = 1,
    EFFECT_INDEX_2 = 2
};

#define MAX_EFFECT_INDEX 3

inline bool IsAreaEffectTarget(SpellTarget target)
{
    switch (target)
    {
        case TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE:
        case TARGET_ENUM_UNITS_ENEMY_IN_CONE_24:
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC:
        case TARGET_ENUM_UNITS_FRIEND_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_FRIEND_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC:
        case TARGET_UNIT_FRIEND_AND_PARTY:
        case TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE:
        case TARGET_UNIT_RAID_AND_CLASS:
            return true;
        default:
            break;
    }
    return false;
}

std::unordered_map<uint32, std::pair<uint32, uint32>> g_spellRanges =
{
    { 1, { 0, 0 } },
    { 2, { 0, 5 } },
    { 3, { 0, 20 } },
    { 4, { 0, 30 } },
    { 5, { 0, 40 } },
    { 6, { 0, 100 } },
    { 7, { 0, 10 } },
    { 8, { 10, 20 } },
    { 9, { 10, 30 } },
    { 10, { 10, 40 } },
    { 11, { 0, 15 } },
    { 12, { 0, 5 } },
    { 13, { 0, 50000 } },
    { 14, { 0, 60 } },
    { 34, { 0, 25 } },
    { 35, { 0, 35 } },
    { 36, { 0, 45 } },
    { 37, { 0, 50 } },
    { 38, { 10, 25 } },
    { 54, { 5, 30 } },
    { 74, { 8, 30 } },
    { 94, { 8, 40 } },
    { 95, { 8, 25 } },
    { 96, { 0, 2 } },
    { 114, { 8, 35 } },
    { 134, { 0, 80 } },
    { 135, { 0, 100 } },
    { 136, { 30, 80 } },
};

enum SpellRangeIndex
{
    SPELL_RANGE_IDX_SELF_ONLY = 1,                          // 0.0
    SPELL_RANGE_IDX_COMBAT    = 2,                          // often ~5.5 (but infact dynamic melee combat range)
    SPELL_RANGE_IDX_ANYWHERE  = 13,                         // 500000 (anywhere)
};

class SpellEntry
{
public:
    uint32      Id = 0;
    uint32      RangeIndex = 1;
    uint32      EffectImplicitTargetA[MAX_EFFECT_INDEX];
    uint32      EffectImplicitTargetB[MAX_EFFECT_INDEX];
    std::string SpellName;
    
    bool IsAreaOfEffectSpell() const
    {
        return
            IsAreaEffectTarget(SpellTarget(EffectImplicitTargetA[EFFECT_INDEX_0])) ||
            IsAreaEffectTarget(SpellTarget(EffectImplicitTargetB[EFFECT_INDEX_0])) ||
            IsAreaEffectTarget(SpellTarget(EffectImplicitTargetA[EFFECT_INDEX_1])) ||
            IsAreaEffectTarget(SpellTarget(EffectImplicitTargetB[EFFECT_INDEX_1])) ||
            IsAreaEffectTarget(SpellTarget(EffectImplicitTargetA[EFFECT_INDEX_2])) ||
            IsAreaEffectTarget(SpellTarget(EffectImplicitTargetB[EFFECT_INDEX_2]));
    }

    inline uint32 GetSpellMinRange() const
    {
        auto itr = g_spellRanges.find(RangeIndex);
        if (itr != g_spellRanges.end())
            return itr->second.first;
        return 0;
    }

    inline uint32 GetSpellMaxRange() const
    {
        auto itr = g_spellRanges.find(RangeIndex);
        if (itr != g_spellRanges.end())
            return itr->second.second;
        return 0;
    }
};


