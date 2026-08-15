# `nonetkill` — renaming classnames in the lump (REJECTED)

*English translation of [04-nonetkill.md](04-nonetkill.md), which is extracted from
`src/sample_mm.cpp`. Function addresses are kept exactly as in the source so they can be
cross-checked. If the two versions disagree, trust the Vietnamese.*

```
nonetkill: rename a classname IN PLACE inside the entity lump, at LevelInit.

MECHANISM (verified against the binary):
  a bogus classname -> CEntityFactoryDictionary::Create (0x10206A40)
                       -> DevWarning("Attempted to create unknown entity type %s!")
                       -> returns NULL
                    -> MapEntity_ParseEntity (0x101198F0): DevWarning("Can't init %s"),
                       does NOT dereference NULL
                    -> MapEntity_ParseAllEntities (0x1011A600): skips the NULL
  => the entity is silently not spawned, COSTS NO EDICT, costs no quota at all.
  Matches Valve's documentation: "Entities... not recognized by the server do not
  create edicts... they are simply not spawned."

XXX TWO PATHS THAT KILL THE SERVER - MUST BE AVOIDED:
  0x1011A6C0  a block not opened with '{'       -> tier0!Error  (import 0x105C1224)
  0x10119943  a block missing the "classname" key -> tier0!Error
  => NEVER delete a block, NEVER change the string length. ONLY overwrite values.

HOW THE RENAME WORKS: replace EXACTLY ONE leading character with '~'.
  infodecal -> ~nfodecal
  Guarantees identical length, and no L4D2 classname begins with '~'
  (all 557 classnames were listed; not one does).

###########################################################################
XXX DEFAULT LIST: EMPTY. DO NOT ADD 'light*' OR 'infodecal' HERE.
###########################################################################
The hardcoded default { infodecal, light, light_spot } was WRONG:
  -> ch04_pripyat03 RENDERED WITH BROKEN LIGHTING.

ROOT CAUSE - nonetkill differs from nonethigh in one life-or-death way:
  nonethigh : the entity IS STILL CREATED, still runs Spawn()/Activate(), it
              simply gets no edict. EVERY SIDE EFFECT still happens.
              -> lighting CORRECT.
  nonetkill : the entity NEVER EXISTS. Spawn()/Activate() never run.
              -> ALL side effects lost.

=> nonetkill is FUNDAMENTALLY WRONG for any entity WHOSE ENTIRE VALUE LIES IN
   ITS SPAWN-TIME SIDE EFFECTS. Verified against the binary
   (output/binscan/step_light.py):

  CLight::Spawn 0x1010FA10  (shared by light / light_spot /
                             light_directional; light_environment jumps here)
    [esi+0x140] m_iszName == 0  -> UTIL_Remove(this)   // an "inert" light, self-deletes
    [esi+0x140] m_iszName != 0  -> if m_iStyle >= 32:
                                     engine->LightStyle(m_iStyle, pattern)
                                     (0x107F7698 = g_pEngineServer, vt +0xA0)
    A NAMED light is a SWITCHABLE light. VRAD bakes it into its own lightstyle
    at compile time; the runtime entity is the ONLY thing that sets the initial
    state of that lightmap layer. Cut the entity -> LightStyle() never runs ->
    the layer keeps its default -> WRONG LIGHTING. An unnamed light already
    self-deletes, so cutting it gains NOTHING.

  CDecal::Spawn 0x102362A0 / CDecal::Activate 0x10236D10
    Spawn:    m_nTexture < 0 or (deathmatch && lowprio) -> UTIL_Remove
              otherwise -> SURVIVES. A dedicated server is not deathmatch => SURVIVES.
    Activate: no targetname -> jmp StaticDecal() (paints the decal, then SELF-DELETES)
    => infodecal NEVER holds an edict long-term. Cutting it saves ALMOST NOTHING
       while costing EVERY decal in the map. A terrible trade.
    (infodecal created by VScript is spawned at runtime, not through the lump,
     so it is unaffected.)

!!  To reduce edicts for the light/infodecal family, use NONETHIGH, not this.

Read from nonetkill.txt if present (one classname per line). Before adding ANY
class you must be able to answer: "does its Spawn()/Activate() do anything?"
If yes -> it must NOT be cut.
XXX Do NOT add classes from the "long-lived" group (logic_auto,
   func_nav_attribute_region, info_gamemode, info_survivor_position...).
```
