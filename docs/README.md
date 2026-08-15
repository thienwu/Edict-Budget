# docs/

Detailed notes extracted from `src/sample_mm.cpp`, so the source keeps only short
comments with pointers here.

Every conclusion carries a function address and an instruction listing so it can be
re-checked against your own copy of `server.dll` / `engine.dll`. Anything that could not
be verified is labelled **not determined** rather than guessed.

| topic | Tiếng Việt | English |
|---|---|---|
| Overview — mission, limits, measurements across 3 campaigns, config files, build | [00-tong-quan.md](00-tong-quan.md) | [00-tong-quan.en.md](00-tong-quan.en.md) |
| The four live mechanisms — `noedict`, `freegate`, `wipeclear`, `swap` | [01-co-che.md](01-co-che.md) | [01-co-che.en.md](01-co-che.en.md) |
| `mapclear` — and why carry-over entities must never be deleted | [02-mapclear.md](02-mapclear.md) | [02-mapclear.en.md](02-mapclear.en.md) |
| The 4096 direction — the whole switch group, **disabled** | [03-huong-4096.md](03-huong-4096.md) | [03-huong-4096.en.md](03-huong-4096.en.md) |
| `nonetkill` — renaming classnames in the lump, **rejected** | [04-nonetkill.md](04-nonetkill.md) | [04-nonetkill.en.md](04-nonetkill.en.md) |
| CEF — dropped from the plan | [04-cef.md](04-cef.md) | [04-cef.en.md](04-cef.en.md) |
| Measurement — logging, inventory, traps, `heartbeat`, `loadprobe` | [05-do-dac.md](05-do-dac.md) | [05-do-dac.en.md](05-do-dac.en.md) |

## Notes

**The Vietnamese files are the originals** and are kept most current. Where a translation
disagrees with the Vietnamese, trust the Vietnamese.

**The text is a verbatim extraction.** It is wrapped in code fences on purpose: the
ASCII tables and assembly listings only line up in a monospace font, and keeping it
verbatim means the docs cannot drift away from what the source actually said.

**Vietnamese is written without diacritics** throughout, matching the source comments.

## Reading order

Start with `00-tong-quan` — it holds the mission statement, the hard limit that no
amount of patching removes, and the measured numbers.

Then `01-co-che` for what the plugin actually does.

`02-mapclear` is worth reading even if you never enable `mapclear`, because it contains
the single most expensive lesson in the project: **deleting a carry-over entity at a
level transition crashes the server**, and the "delete fewer things" rule that everyone
reaches first is the wrong rule.

`03-huong-4096` is history: that switch group is disabled and must stay disabled. It is
documented so nobody re-derives it and re-enables it.

`04-nonetkill` and `04-cef` are rejected directions, kept for the same reason.
