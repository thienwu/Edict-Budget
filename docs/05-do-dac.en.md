# Measurement — logging, inventory, traps

*English translation of [05-do-dac.md](05-do-dac.md), which is extracted from
`src/sample_mm.cpp`. Function addresses are kept exactly as in the source so they can be
cross-checked. If the two versions disagree, trust the Vietnamese.*

## Logging to a dedicated file

```
LOGGING TO A DEDICATED FILE
==========================================================================

META_LOG only pushes to the server console. If the server does not have
console.log enabled, every measurement is lost. So all plugin logging goes
straight to its own file:

    left4dead2\addons\edictbudget\edictbudget.log

Every line is timestamped. The file is opened in append mode, never truncated.
fflush after every line, so that if the server dies suddenly the log still
covers the final moments - exactly when it is needed most.

SWITCH logconsole = 1 also prints to the console (default 0).
```

## Counting allocations within ONE frame

```
Count edict allocations within ONE frame.

The last sample before every death was always num_edicts=2012 with ~904 free
slots - a state in which ED_Alloc CANNOT fail (2012 < 2048, so it just appends).
To reach the failure branch, the interval between two samples (<0.25s) must
contain ~940 allocations: 36 to push num_edicts to 2048, plus 904 to consume
every free slot. If that is right, a wipe is a burst of ~940 entities AT ONCE
and the map genuinely exceeds 2048 at its peak - in which case every
"reuse the slot" approach is meaningless, because there is nothing left to reuse.
```

## `loadprobe` — logging edict counts over the first N frames

```
Log the edict count over the first N frames after a map load, to catch a
TRANSIENT PEAK. 0 = off.

Why it is needed: `BASELINE` is logged at ServerActivate, and at that moment many
entities have NOT finished spawning. For example point_spotlight creates
spotlight_end + beam inside Activate()/Think(), i.e. AFTER ServerActivate. That
is why m4 logs num_edicts=1463 there while counting the lump gives 2067 - the
difference appears over the next few frames.
On top of that, ~35 weapon_*_spawn classes create the real entity then call
UTIL_Remove on themselves; UTIL_Remove is deferred to end of frame, so each one
holds 2 edicts simultaneously during the load frame.
Both hypotheses can only be tested by sampling FRAME BY FRAME.

swap: substitute an entity class for a cheaper one at creation time. See the
explanation block at InstallSwap().
0 = off | 1 = OBSERVE ONLY (count, do not substitute) | 2 = substitute for real
```

## Census of every class the map creates

```
Census of EVERY class the map creates, whether or not we relocate it.

Choosing an allow-list by intuition DOES NOT WORK: the conservative
logic_/math_/ai_ set freed only 10 slots on c1m1_hotel, because L4D2 keeps most
of its map logic in VScript rather than in entities. To choose usefully you have
to know what a map ACTUALLY spawns and in what quantity, sorted by count, so the
largest server-side groups become obvious immediately.
```

## `trap` — a trap on ED_Alloc's actual failure branch

```
A trap on ED_Alloc's ACTUAL failure branch
==========================================================================

Every measurement taken at IVEngineServer::CreateEdict was BLIND: the burst
counter never saw a frame with >=32 allocations, and the hook was never once
called for the failing allocation. That means ED_Alloc is being called through
an internal engine path.

The only remaining place to look is the failure branch itself:
    1E0247  85 DB              test ebx, ebx
    1E0249  0F 88 84 00 00 00  js   1E02D3      -> reports "no free edicts"
    1E024F  ...                                 -> reuses ebx

Replace those 8 bytes with a 5-byte JMP to our stub plus 3 NOPs. The stub logs,
then rejoins whichever of the two original branches applies. This is a COLD path
- it only runs when the engine is about to die - so the risk is far lower than a
detour on a hot path.

ebx = the index of the LAST free edict the scan loop saw (-1 = it saw none). That
is exactly the number we need: does the engine genuinely see no free slot, while
we count ~912?
```

## Inventory at the moment edicts run out

```
Inventory AT THIS EXACT MOMENT: what is occupying all 2048 slots?

Every earlier inventory counted during CALM PERIODS and produced a completely
different picture - which is what sent two whole days in the wrong direction.
This is the only moment that means anything: the engine has just confirmed there
is not one free slot left.

Last time this table FAILED TO PRINT: the header was logged AFTER the loop, and
the loop called the virtual GetClassName() on 2048 edicts while the engine was
already dying, so it hit a corrupt pointer and died before printing anything.
Now:
  - print the header FIRST
  - wrap every single edict read in SEH
  - print each line as soon as it is gathered, not at the end
```

## When the measurement contradicts the machine code

```
An earlier measurement produced a result that CONTRADICTS the machine code:
num_edicts=2048 with 880 edicts flagged FL_EDICT_FREE, and the engine still
reported "no free edicts".
ED_Alloc records EVERY free edict it walks past (mov ebx,esi at 0x1E0209) and
only errors when ebx is still -1; so with 880 free, that branch is unreachable.

There is only ONE way for both facts to hold: the scan loop NEVER RUNS. It starts
at
    esi = sv.GetMaxClients() + 1
and the instruction at 0x1E01E8 skips the whole loop when esi >= num_edicts. So
the number that matters is NOT how many slots are free - it is how many are free
INSIDE THE WINDOW the engine actually looks at.

Call the engine's own GetMaxClients (RVA 0x134640 on the sv object) to read
EXACTLY what ED_Alloc reads, instead of trusting gpGlobals.
It turns out sv.GetMaxClients() (RVA 0x134640) is a one-line getter:
    mov eax, [ecx+0x104] ; ret
so read the field directly - no call, no calling-convention risk.

Worth noting: L4DToolZ writes sv[+0x180] (its slots_idx 0x60), a COMPLETELY
DIFFERENT field. Whether those two agree is precisely the open question.
```

## Catching the exact moment ED_Alloc gives up

```
Catch the EXACT moment ED_Alloc gives up
--------------------------------------------------------------------------

Sampling from a throttled hook never caught the failure: every sample showed
num_edicts=2012 (below the 2048 ceiling) with 861 free edicts inside the scan
window - a state where ED_Alloc PROVABLY cannot fail. The wipe burst happens
ENTIRELY WITHIN one frame, i.e. between two samples.

A POST hook on CreateEdict sees the one thing that matters: the call that
returned NULL. Log the full state right there, unthrottled.
```

## `heartbeat` — periodic measurements

```
HEARTBEAT - periodically write entity measurements to the log
==========================================================================

Purpose: a production server running for days yields far more data than local
testing. trap=1 only measures the inventory AT DEATH - the outcome, never the
progression. Heartbeat shows which classes GROW OVER TIME, which is what is
needed to design any in-play entity reclamation.

LOG ONLY. It touches no entity.

On each beat:
  - one summary line:  live / num_edicts / free / spread
  - the classes that CHANGED since the previous beat, sorted by magnitude
    (only changes are printed, never the whole table => the log stays small)

SWITCH: heartbeat = SECONDS between writes. 0 = off.
        Recommended: 300 (5 minutes).
```
