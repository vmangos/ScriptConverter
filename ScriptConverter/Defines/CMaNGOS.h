namespace CMaNGOS
{

    enum EventAI_Type
    {
        EVENT_T_TIMER_IN_COMBAT         = 0,                    // InitialMin, InitialMax, RepeatMin, RepeatMax
        EVENT_T_TIMER_OOC               = 1,                    // InitialMin, InitialMax, RepeatMin, RepeatMax
        EVENT_T_HP                      = 2,                    // HPMax%, HPMin%, RepeatMin, RepeatMax
        EVENT_T_MANA                    = 3,                    // ManaMax%,ManaMin% RepeatMin, RepeatMax
        EVENT_T_AGGRO                   = 4,                    // NONE
        EVENT_T_KILL                    = 5,                    // RepeatMin, RepeatMax, PlayerOnly (1)
        EVENT_T_DEATH                   = 6,                    // ConditionId
        EVENT_T_EVADE                   = 7,                    // NONE
        EVENT_T_SPELLHIT                = 8,                    // SpellID, School, RepeatMin, RepeatMax
        EVENT_T_RANGE                   = 9,                    // MinDist, MaxDist, RepeatMin, RepeatMax
        EVENT_T_OOC_LOS                 = 10,                   // NoHostile, MaxRange, RepeatMin, RepeatMax, PlayerOnly, ConditionId
        EVENT_T_SPAWNED                 = 11,                   // Condition, CondValue1
        EVENT_T_TARGET_HP               = 12,                   // HPMax%, HPMin%, RepeatMin, RepeatMax
        EVENT_T_TARGET_CASTING          = 13,                   // RepeatMin, RepeatMax
        EVENT_T_FRIENDLY_HP             = 14,                   // HPDeficit, Radius, RepeatMin, RepeatMax
        EVENT_T_FRIENDLY_IS_CC          = 15,                   // DispelType, Radius, RepeatMin, RepeatMax
        EVENT_T_FRIENDLY_MISSING_BUFF   = 16,                   // SpellId, Radius, RepeatMin, RepeatMax
        EVENT_T_SUMMONED_UNIT           = 17,                   // CreatureId, RepeatMin, RepeatMax
        EVENT_T_TARGET_MANA             = 18,                   // ManaMax%, ManaMin%, RepeatMin, RepeatMax
        EVENT_T_QUEST_ACCEPT            = 19,                   // QuestID
        EVENT_T_QUEST_COMPLETE          = 20,                   //
        EVENT_T_REACHED_HOME            = 21,                   // NONE
        EVENT_T_RECEIVE_EMOTE           = 22,                   // EmoteId, ConditionId
        EVENT_T_AURA                    = 23,                   // Param1 = SpellID, Param2 = Number of time stacked, Param3/4 Repeat Min/Max
        EVENT_T_TARGET_AURA             = 24,                   // Param1 = SpellID, Param2 = Number of time stacked, Param3/4 Repeat Min/Max
        EVENT_T_SUMMONED_JUST_DIED      = 25,                   // CreatureId, RepeatMin, RepeatMax
        EVENT_T_SUMMONED_JUST_DESPAWN   = 26,                   // CreatureId, RepeatMin, RepeatMax
        EVENT_T_MISSING_AURA            = 27,                   // Param1 = SpellID, Param2 = Number of time stacked expected, Param3/4 Repeat Min/Max
        EVENT_T_TARGET_MISSING_AURA     = 28,                   // Param1 = SpellID, Param2 = Number of time stacked expected, Param3/4 Repeat Min/Max
        EVENT_T_TIMER_GENERIC           = 29,                   // InitialMin, InitialMax, RepeatMin, RepeatMax
        EVENT_T_RECEIVE_AI_EVENT        = 30,                   // AIEventType, Sender-Entry, unused, unused
        EVENT_T_ENERGY                  = 31,                   // EnergyMax%, EnergyMin%, RepeatMin, RepeatMax
        EVENT_T_SELECT_ATTACKING_TARGET = 32,                   // MinRange, MaxRange, RepeatMin, RepeatMax
        EVENT_T_FACING_TARGET           = 33,                   // Position, unused, RepeatMin, RepeatMax

        EVENT_T_END,
    };

    enum EventAI_ActionType
    {
        ACTION_T_NONE                       = 0,                // No action
        ACTION_T_TEXT                       = 1,                // TextId1, optionally -TextId2, optionally -TextId3(if -TextId2 exist). If more than just -TextId1 is defined, randomize. Negative values.
        ACTION_T_SET_FACTION                = 2,                // FactionId (or 0 for default)
        ACTION_T_MORPH_TO_ENTRY_OR_MODEL    = 3,                // Creature_template entry(param1) OR ModelId (param2) (or 0 for both to demorph)
        ACTION_T_SOUND                      = 4,                // SoundId
        ACTION_T_EMOTE                      = 5,                // EmoteId
        ACTION_T_RANDOM_SAY                 = 6,                // UNUSED
        ACTION_T_RANDOM_YELL                = 7,                // UNUSED
        ACTION_T_RANDOM_TEXTEMOTE           = 8,                // UNUSED
        ACTION_T_RANDOM_SOUND               = 9,                // SoundId1, SoundId2, SoundId3 (-1 in any field means no output if randomed that field)
        ACTION_T_RANDOM_EMOTE               = 10,               // EmoteId1, EmoteId2, EmoteId3 (-1 in any field means no output if randomed that field)
        ACTION_T_CAST                       = 11,               // SpellId, Target - default = 15, CastFlags
        ACTION_T_SPAWN                      = 12,               // CreatureID, Target, Duration in ms
        ACTION_T_THREAT_SINGLE_PCT          = 13,               // Threat%, Target
        ACTION_T_THREAT_ALL_PCT             = 14,               // Threat%
        ACTION_T_QUEST_EVENT                = 15,               // QuestID, Target, Group
        ACTION_T_CAST_EVENT                 = 16,               // QuestID, SpellId, Target - must be removed as hack?
        ACTION_T_SET_UNIT_FIELD             = 17,               // Field_Number, Value, Target
        ACTION_T_SET_UNIT_FLAG              = 18,               // Flags (may be more than one field OR'd together), Target
        ACTION_T_REMOVE_UNIT_FLAG           = 19,               // Flags (may be more than one field OR'd together), Target
        ACTION_T_AUTO_ATTACK                = 20,               // AllowAttackState (0 = stop attack, anything else means continue attacking)
        ACTION_T_COMBAT_MOVEMENT            = 21,               // AllowCombatMovement (0 = stop combat based movement, anything else continue attacking)
        ACTION_T_SET_PHASE                  = 22,               // Phase
        ACTION_T_INC_PHASE                  = 23,               // Value (may be negative to decrement phase, should not be 0)
        ACTION_T_EVADE                      = 24,               // No Params
        ACTION_T_FLEE_FOR_ASSIST            = 25,               // No Params
        ACTION_T_QUEST_EVENT_ALL            = 26,               // QuestID, UseThreatList (1 = true, 0 = false)
        ACTION_T_CAST_EVENT_ALL             = 27,               // CreatureId, SpellId
        ACTION_T_REMOVEAURASFROMSPELL       = 28,               // Target, Spellid
        ACTION_T_RANGED_MOVEMENT            = 29,               // Distance, Angle
        ACTION_T_RANDOM_PHASE               = 30,               // PhaseId1, PhaseId2, PhaseId3
        ACTION_T_RANDOM_PHASE_RANGE         = 31,               // PhaseMin, PhaseMax
        ACTION_T_SUMMON_ID                  = 32,               // CreatureId, Target, SpawnId
        ACTION_T_KILLED_MONSTER             = 33,               // CreatureId, Target
        ACTION_T_SET_INST_DATA              = 34,               // Field, Data
        ACTION_T_SET_INST_DATA64            = 35,               // Field, Target
        ACTION_T_UPDATE_TEMPLATE            = 36,               // Entry, Team
        ACTION_T_DIE                        = 37,               // No Params
        ACTION_T_ZONE_COMBAT_PULSE          = 38,               // No Params
        ACTION_T_CALL_FOR_HELP              = 39,               // Radius
        ACTION_T_SET_SHEATH                 = 40,               // Sheath (0-passive,1-melee,2-ranged)
        ACTION_T_FORCE_DESPAWN              = 41,               // Delay (0-instant despawn)
        ACTION_T_SET_INVINCIBILITY_HP_LEVEL = 42,               // MinHpValue, format(0-flat,1-percent from max health)
        ACTION_T_MOUNT_TO_ENTRY_OR_MODEL    = 43,               // Creature_template entry(param1) OR ModelId (param2) (or 0 for both to unmount)
        ACTION_T_CHANCED_TEXT               = 44,               // Chance to display the text, TextId1, optionally TextId2. If more than just -TextId1 is defined, randomize. Negative values.
        ACTION_T_THROW_AI_EVENT             = 45,               // EventType, Radius, Target
        ACTION_T_SET_THROW_MASK             = 46,               // EventTypeMask, unused, unused
        ACTION_T_SET_STAND_STATE            = 47,               // StandState, unused, unused
        ACTION_T_CHANGE_MOVEMENT            = 48,               // MovementType, WanderDistance if Movement Type 1 and PathId if Movement Type 2, unused
        ACTION_T_DYNAMIC_MOVEMENT           = 49,               // EnableDynamicMovement (1 = on; 0 = off)
        ACTION_T_SET_REACT_STATE            = 50,               // React state, unused, unused
        ACTION_T_PAUSE_WAYPOINTS            = 51,               // DoPause 0: unpause waypoint 1: pause waypoint, unused, unused
        ACTION_T_INTERRUPT_SPELL            = 52,               // SpellType enum CurrentSpellTypes, unused, unused
        ACTION_T_START_RELAY_SCRIPT         = 53,               // Relay script ID, target, unused
        ACTION_T_TEXT_NEW                   = 54,               // Text ID, target, template Id
        ACTION_T_ATTACK_START               = 55,               // Target, unused, unused
        ACTION_T_DESPAWN_GUARDIANS          = 56,               // Guardian Entry ID (or 0 to despawn all guardians), unused, unused
        ACTION_T_SET_RANGED_MODE            = 57,               // type of ranged mode, distance to chase at

        ACTION_T_END,
    };

    enum Target
    {
        // Self (m_creature)
        TARGET_T_SELF                           = 0,            // Self cast

        // Hostile targets
        TARGET_T_HOSTILE                        = 1,            // Our current target (ie: highest aggro)
        TARGET_T_HOSTILE_SECOND_AGGRO           = 2,            // Second highest aggro (generaly used for cleaves and some special attacks)
        TARGET_T_HOSTILE_LAST_AGGRO             = 3,            // Dead last on aggro (no idea what this could be used for)
        TARGET_T_HOSTILE_RANDOM                 = 4,            // Just any random target on our threat list
        TARGET_T_HOSTILE_RANDOM_NOT_TOP         = 5,            // Any random target except top threat

        // Invoker targets
        TARGET_T_ACTION_INVOKER                 = 6,            // Unit who caused this Event to occur (only works for EVENT_T_AGGRO, EVENT_T_KILL, EVENT_T_DEATH, EVENT_T_SPELLHIT, EVENT_T_OOC_LOS, EVENT_T_FRIENDLY_HP, EVENT_T_FRIENDLY_IS_CC, EVENT_T_FRIENDLY_MISSING_BUFF, EVENT_T_RECEIVE_EMOTE, EVENT_T_RECEIVE_AI_EVENT)
        TARGET_T_ACTION_INVOKER_OWNER           = 7,            // Unit who is responsible for Event to occur (only works for EVENT_T_AGGRO, EVENT_T_KILL, EVENT_T_DEATH, EVENT_T_SPELLHIT, EVENT_T_OOC_LOS, EVENT_T_FRIENDLY_HP, EVENT_T_FRIENDLY_IS_CC, EVENT_T_FRIENDLY_MISSING_BUFF, EVENT_T_RECEIVE_EMOTE, EVENT_T_RECEIVE_AI_EVENT)
        TARGET_T_EVENT_SENDER                   = 10,           // Unit who sent an AIEvent that was received with EVENT_T_RECEIVE_AI_EVENT

        // Hostile players
        TARGET_T_HOSTILE_RANDOM_PLAYER          = 8,            // Just any random player on our threat list
        TARGET_T_HOSTILE_RANDOM_NOT_TOP_PLAYER  = 9,            // Any random player from threat list except top threat

        // Summon targeting
        TARGET_T_SPAWNER                        = 11,           // Owner of unit if exists

        // Event specific targeting
        TARGET_T_EVENT_SPECIFIC                 = 12,           // Filled by specific event

        // Player associations
        TARGET_T_PLAYER_INVOKER                 = 13,           // Player who initiated hostile contact with this npc
        TARGET_T_PLAYER_TAPPED                  = 14,           // Player who currently holds to score the kill credit from the npc

        // Default Spell Target
        TARGET_T_NONE                           = 15,           // Default spell target - sets nullptr which should be most common spell fill

        TARGET_T_HOSTILE_RANDOM_MANA            = 16,           // Random target with mana
        TARGET_T_NEAREST_AOE_TARGET             = 17,           // Nearest target for aoe
        TARGET_T_HOSTILE_FARTHEST_AWAY          = 18,           // Farthest away target, excluding melee range
    };

    enum EventFlags
    {
        EFLAG_REPEATABLE            = 0x01,                     // Event repeats
        EFLAG_RESERVED_1            = 0x02,
        EFLAG_RESERVED_2            = 0x04,
        EFLAG_RESERVED_3            = 0x08,
        EFLAG_RESERVED_4            = 0x10,
        EFLAG_RANDOM_ACTION         = 0x20,                     // Event only execute one from existed actions instead each action.
        EFLAG_RESERVED_6            = 0x40,
        EFLAG_DEBUG_ONLY            = 0x80,                     // Event only occurs in debug build
        EFLAG_RANGED_MODE_ONLY      = 0x100,                    // Event only occurs in ranged mode
        EFLAG_MELEE_MODE_ONLY       = 0x200,                    // Event only occurs in melee mode
        EFLAG_COMBAT_ACTION         = 0x400,                    // First action must succeed
        // no free bits, uint8 field
    };

    enum CastFlags
    {
        CAST_INTERRUPT_PREVIOUS     = 0x01,                     // Interrupt any spell casting
        CAST_TRIGGERED              = 0x02,                     // Triggered (this makes spell cost zero mana and have no cast time)
        CAST_FORCE_CAST             = 0x04,                     // Forces cast even if creature is out of mana or out of range
        CAST_NO_MELEE_IF_OOM        = 0x08,                     // Prevents creature from entering melee if out of mana or out of range
        CAST_FORCE_TARGET_SELF      = 0x10,                     // Forces the target to cast this spell on itself
        CAST_AURA_NOT_PRESENT       = 0x20,                     // Only casts the spell if the target does not have an aura from the spell
        CAST_IGNORE_UNSELECTABLE_TARGET = 0x40,                 // Can target UNIT_FLAG_NOT_SELECTABLE - Needed in some scripts
        CAST_SWITCH_CASTER_TARGET   = 0x80,                     // Switches target and caster for spell cast
        CAST_MAIN_SPELL             = 0x100,                    // Marks main spell
        CAST_PLAYER_ONLY            = 0x200,                    // Selects only player targets - substitution for EAI not having more params
        CAST_DISTANCE_YOURSELF      = 0x400,                    // If spell with this cast flag hits main aggro target, caster distances himself - EAI only
        CAST_TARGET_CASTING         = 0x800,                    // Selects only player targets that are casting - EAI only
        CAST_ONLY_XYZ               = 0x1000,
    };

    struct EventAISummon
    {
        uint32 id;

        float position_x;
        float position_y;
        float position_z;
        float orientation;
        uint32 SpawnTimeSecs;
    };
}