# `mapclear` — cleaning entities at a level transition

*English translation of [02-mapclear.md](02-mapclear.md), which is extracted from
`src/sample_mm.cpp`. Function addresses are kept exactly as in the source so they can be
cross-checked. If the two versions disagree, trust the Vietnamese.*

## `mapclearcarry` — why carry-over entities must NEVER be deleted

```
1 = delete ONLY entities with FCAP_ACROSS_TRANSITION. 0 = delete everything
(default, the original behaviour).

!!! LEAVE THE DEFAULT AT 0. MODE 1 WAS TRIED AT 09:44 ON AUG 14 AND KILLED THE
    SERVER IMMEDIATELY.

  MAPCLEAR #1 (mode 2, carry-only=1): total 1551 | carrying over 295
  | removed 200 (200 of them carry-over), kept 156, skipped as non-carrying 1153,
  hit cap 42
  -> "Server is hibernating" + restart. No ED_Alloc, no assert.

The correct rule, which explains ALL THREE deaths (replacing the wrong
"threshold 1300" rule):
    cap 100  -> of which    9 were carry-over -> SURVIVED
    >1300    -> of which ~270 were carry-over -> DIED
    carry=1  -> of which  200 were carry-over -> DIED
  IT IS NOT THE COUNT THAT KILLS. IT IS DELETING CARRY-OVER ENTITIES.
  The "1300 threshold" was a coincidence: the more you delete, the more
  carry-over entities you sweep up along the way.

Mechanism: the hook runs POST, so the original PrepareLevelChange has ALREADY
built the transition list. Entities with FCAP_ACROSS_TRANSITION are already in
that list. Deleting them after the list is built => the list points into freed
memory => crash when the engine processes the transition.
A PRE hook would not save it either: the engine still has to read those very
entities to build the list.

Consequence: only carry-over entities cost an edict on the next map, and
carry-over entities must not be touched => MAPCLEAR CANNOT, IN PRINCIPLE, SOLVE
"m3 -> m4". This switch is kept only to record the experiment, NOT to be enabled.
```

## `mapclear`: the full mechanism

```
MAPCLEAR - clean entities BEFORE THE ENGINE DOES, at a LEVEL TRANSITION
==========================================================================

XXX THIS IS NOT THE 4096 DIRECTION. No bigarray/snapshot/pinmax/pinglobals/markfree.
   Not a single byte is patched. It only hooks a vtable, exactly like wipeclear.

!!  IT DIFFERS FROM WIPECLEAR IN A LIFE-OR-DEATH WAY - READ BEFORE EDITING:
  wipeclear: same map; if you delete the wrong thing it is REBUILT from the lump
             => keeping the MINIMUM is correct (wipekeep.txt stays EMPTY)
  mapclear : different map; deleting the wrong thing is PERMANENT LOSS for the player
             => keep the MAXIMUM. When unsure, KEEP.
  => NEVER swap the keep sets of the two mechanisms.

MECHANISM (read from the binary):
  - The new map starts with a FRESH edict table. Junk OUTSIDE the transition
    volume disappears by itself => cleaning it is pointless. Only what is IN the
    carry-over list is worth cleaning.
  - CBaseEntity::ObjectCaps() 0x10056160 returns FCAP_ACROSS_TRANSITION BY
    DEFAULT => almost EVERYTHING inside the volume carries over, including
    corpses and debris.
  - Two carry-over paths:
      (a) items IN HAND -> CTerrorGameRules slot 38 serialises them to KeyValues
          (weaponID, currentMagazine, extraAmmo...). COSTS NO EDICT.
      (b) items ON THE GROUND -> Source's standard trigger_transition. COSTS EDICTS.
    => only (b) is a problem.
  - the_hive_m4 enters the map with only 31 free slots => carrying ~32 hits the ceiling.

HOOK POINT: CTerrorGameRules vtable slot 38 = 0x102B8140, hooked POST.
  Prints "Preparing player entities for changelevel". __thiscall, ret 4.
  It runs on the OLD MAP, AFTER the player snapshot, BEFORE the save machinery starts.
  Same vtable wipeclear already hooks (slot 178 = RestartRound = 0x102E0650).

  XXX SLOT 27 (BuildAdjacentMapList) WAS REJECTED: it runs in 3 places, 2 of them
     on the NEW MAP (CSaveRestore::LoadAdjacentEnts + the .HL2 load path). Hooking
     it blindly means deleting the new map's entities as it loads. And by slot 27
     SaveGameState has already called PreSave => the entity table is frozen =>
     deleting risks dangling pointers.

XXX DO NOT run g_debug_transitions to "see what the engine prints": that cvar
   BLOCKS the transition outright and sets m_pfnTouch = 0 => the safe room door
   becomes a dead door.

DEFAULT KEEP SET (conservative; extended from mapkeep.txt):
  - the GAME'S entire preserve list (same function wipeclear uses) - conservative
  - player, weapon_ (covers weapon_*_spawn, gascan/propanetank/oxygentank)
  - prop_fuel_barrel (covers _piece too)
  - transition infrastructure: info_landmark, trigger/info_changelevel,
    trigger_transition
    (deleting these breaks THE TRANSITION ITSELF)

SWITCH: mapclear = 0 off | 1 OBSERVE ONLY (count + log, delete nothing) | 2 clean
(g_MapClear is declared at the top of the file, next to the other switches)
```

## `WillCarryOver` — ask the engine directly

```
Ask the engine directly: will this entity be carried to the new map?
ObjectCaps() is virtual slot 40 (+0xA0). Bit 0x2 = FCAP_ACROSS_TRANSITION.
Both InTransitionVolume (0x101FEFB0) and ComputeEntitySaveFlags (0x101F8D80)
call exactly this, so asking it directly avoids having to pass a volume name.

*** THIS IS THE CORE DIFFERENCE FROM VERSION 1 (which KILLED the server):
   version 1 scanned ALL 1659 entities and deleted 1497 -> including things the
   engine still needed.
   this version only touches entities that will genuinely travel -> a far
   smaller blast radius.
```

## The must-not-delete list

```
1. MUST-NOT-DELETE LIST:
     - player-held items       -> weapon_ , prop_fuel_barrel*
     - transition points       -> info_landmark, trigger/info_changelevel,
                                  trigger_transition
     - safe room door          -> prop_door_rotating_checkpoint
     - players                 -> player
   + the GAME'S entire preserve list (worldspawn, terror_gamerules,
     soundent, scene_manager... deleting any of these kills the server instantly)
```

## Safety gate 1: the prologue

```
Safety gate 1: the prologue at 0x102B8140 must match.

!!  LESSON FROM AUG 9: this prologue CONTAINS AN ABSOLUTE ADDRESS, so all 16
    bytes must NOT be compared as one block.
  55 8B EC 56 8B 35 | E0 7A 89 10 | 8B 06 8B 50 68 8B
                      ^^^^^^^^^^^ mov esi,[0x10897AE0]
  Those four bytes are REWRITTEN BY THE LOADER when server.dll lands at a
  different base => comparing all 16 bytes NEVER matches, and the safety gate
  rejects a perfectly good target.
  (wipeclear is unaffected because its prologue has no absolute address.)
=> Use a mask: '?' = a relocated byte, skip it.
```
