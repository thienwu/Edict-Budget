# CEF — dropped from the plan

*English translation of [04-cef.md](04-cef.md), which is extracted from
`src/sample_mm.cpp`. Function names are kept exactly as in the source so they can be
cross-checked. If the two versions disagree, trust the Vietnamese.*

```
CEF - DROPPED FROM THE PLAN (Aug 7). Recorded so nobody adds it back by mistake.

Original intent: fold CEF into this plugin, since the original CEF
(`mmscef-code`) IS ALREADY a Metamod plugin, not a SourceMod extension.

USER'S DECISION: do NOT copy CEF in here. The original CEF source is for
REFERENCE ONLY - it does NOT fully support L4D2, and would need a redesign if
this mechanism is ever wanted.

Technical reason:
  The original CEF uses `PEntityOfEntIndex` to find free slots. On L4D2, Valve
  REMOVED that function from IVEngineServer, so `engine_wrappers.h` substitutes
  plain pointer arithmetic - which is ALWAYS non-NULL => the loop runs all the
  way to maxEntities and bails.
  In other words the original CEF is a NO-OP on L4D2. It is "stable" because it
  does nothing at all.
  => Copying it in verbatim would be copying something that does not run.

If this mechanism is ever needed, it must be REDESIGNED for L4D2:
  - use the real `edict_t::IsFree()`, not PEntityOfEntIndex
  - and MEASURE FIRST: there is still no data showing a dangerous peak during
    ordinary play. Measured Aug 7: the highest slot ever used was 682/2048,
    with ~950 always free. Every burst measured happened on the wipe path, and
    `wipeclear` already handles that.
  - and remember the risk noted in section 0-AA: the CEF author himself warns
    "PROBABLY UNSTABLE... random crashing", and the sourcemod+0x13b63 crash
    appears exactly when an index is forced.
```
