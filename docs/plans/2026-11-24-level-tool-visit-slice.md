# Level tool, milestone 1: the LE3 visit slice

Status: pressure-tested and revised, 2026-11-24. No code exists.

Moved from `mass-effect-storylet-study` (below, "the study repo"; its `level-tool-plan`
branch, `72b9577`) on 2026-11-25. Paths marked "study" are in that repo.

## Goal

A headless C++ tool that loads Mass Effect LE levels, analyses their Kismet and answers
spatial questions. It is used by LLMs first, with no frontend, and later makes undoable
edits. Milestone 1 is LE3 only and read-only, and answers one question: **for a given
conversation start, where, when and how can it be triggered?**

Study `docs/plans/2026-11-24-graph-export.md` defers this "model of visits". Today's
exports say what plot logic does, but not where it is or when its sublevel is loaded.

## Decided by the user

- C++, with an undoable command system. LLM use first, no frontend.
- `C:\dev\coredcc` is the place to look for the command system.
- Formats and logic are learned from Legendary Explorer (LEX) and written as our own code.
  LEX's core library, LEC, isn't linked.
- Kismet edits belong to this tool, in milestone 2.
- coredcc stays out of this milestone's main path. A side test of how it fits UE3 data
  follows step 1, and the first edits bring it in.
- This slice is the first milestone. Build setup isn't planned yet.
- The tool lives in its own repo, this one.
- Edits (milestone 2) come before spatial work (milestone 3). Walkable reachability is in
  milestone 3.

## Ownership (proposed, except Kismet edits)

| Area                                                            | Owner                                   |
| --------------------------------------------------------------- | --------------------------------------- |
| Level sets, streaming, actors, triggers, spatial                | this tool                               |
| Kismet edits, and later geometry, lighting and actor edits      | this tool                               |
| Conversation starts, splices and node Kismet                    | LEC builder now; this tool once matched |
| Conditionals, transitions, TLK, Coalesced, conversation content | `me-gamedata-export` and the study repo |
| Meaning of plot logic, order rules, gate walks                  | the study repo (`src/meplot/gates.ts`)  |
| Packaging, TOC and installing                                   | ME3Tweaks Mod Manager                   |

## Facts about LE3 (2026-11-24)

Marked **R** when only the reviewer measured it.

- **Streaming.**
    - 2,216 `BioTriggerStream` volumes, with 5,650 states (R), and 2,176
      `LevelStreamingKismet` entries in 89 packages, so levels nest (R). `BioP_Nor` itself
      has 117 entries and 7 volumes; more Normandy volumes are in `BioD_Nor` and
      `BioA_Nor`.
    - `BioWorldInfo.PlotStreaming` maps a virtual chunk name to chunks, each with a
      conditional and a fallback. It has 133 elements for 46 chunks, in four
      `BioP_Global*` packages (R). `BioD_Nor` shows `BioH_Tali`, and no such file exists.
    - Of 3,848 `SetStreamingState` ops, most take the state name (about 2,500) or the
      value (2,797) from a variable link.
    - Nothing exports `StreamingStates` or `PlotStreaming` today.
- **Kismet export.** `kismet.json` covers 2,043 used packages, and only those holding one
  of 11 classes (`Relevant` in `me-gamedata-export`'s `Graphs.cs`), which exclude ambient
  and face-only starts. It has no array properties or Matinee tracks.
- **Trigger events.** In those packages, 5,782 touch, use and door events: 4,192 with an
  `Originator`, 1,590 without. Of the 1,590 (R): 764 are templates in prefab packages, 752
  are attached by `SeqAct_AttachToEvent`, 74 have neither.
- **Starts (R).** Of 1,662 used conversations that touch plot, 704 have no
  `StartConversation`, but only 2 have no start reference at all once every object
  property is searched. Most of the rest are `SFXSeqAct_StartAmbientConv`. The sole start
  is `SFXInterpTrackPlayFaceOnlyVO` in Matinee for 18, and a `SeqVar_Object` for 13.
- **Navigation.** No `Pylon`. 25,187 `PathNode`s, none in `Bio[ADP]_Nor*`, where the only
  navigation points are 37 `BioPathPoint`s in five cutscene packages. The Normandy's decks
  are joined by teleports (26 in `BioD_Nor_001Global`), not by floor.
- **Packages.** 4,049 of 4,050 base-game packages are flagged compressed.
  `BioD_Nor_204Gth` has two copies, and the game uses `DLC_METR_Patch01`'s.

## Design

- **Layers:** `ue3` (packages, properties, class defaults, binary formats) → `level` →
  `kismet` → `app` → a CLI. `ue3` doesn't depend on coredcc, so packages can load on
  worker threads. `spatial` comes with milestone 3.
- **Decompression** uses the game's own Oodle DLL at run time. Nothing is redistributed.
- **Class schemas and defaults** come from the game's class packages, because array tags
  don't name their element type and unset properties come from defaults. Native classes
  whose definitions aren't in a package (`StaticMesh`, `Model`, `Level`; 33 in all, R) and
  the immutable structs are a hand-written table.
- **Stable ids:** game, DLC, package and instanced path, in a form that can become coredcc
  references.
- **Base packages are immutable once loaded,** so an edit layer can sit on top later.
- **Queries aren't commands.** A typed query returns paged rows with provenance.
- **Answers are yes, no or unknown,** with the evidence, in words aligned with the gate
  walker's "every path", "some path" and "capped". A Kismet class with no model is opaque.
  Native and script firers (galaxy map, GUI, death, damage) are named opaque sources.
- **Scope of events:** a remote event reaches only levels that can be loaded together.
- **Interface:** a CLI over an on-disk index, printing JSON. Batch output matches the v3
  schemas in study `docs/gamedata.md` where they overlap. No RPC until measurements call
  for it.
- **Data rule:** output stays in an ignored `build/`. Nothing derived from game files is
  committed (study `docs/data-sources.md`). The tool never writes to the game install.

## Steps

1. **Package reading for the Normandy:** decompress, tables, tagged properties, class
   defaults, for `BioP_Nor`'s level set and the class packages it needs. Compare with a
   LEC dump.
2. **Level sets:** sublevels, nested streaming entries, `PlotStreaming` virtual chunks and
   the copy of each file the game loads. Compare with the exporter's `used` flags.
3. **Check streaming in game.** Which state wins when volumes, tiers and priorities
   overlap is native code. Design a short run (study `docs/debugging.md`'s
   `Phase3_Docks_Confront` entry is a ready case) before relying on a rule.
4. **Streaming:** states, `SetStreamingState` with its variables evaluated or unknown, and
   the other streaming ops (`MultiLevelStreaming`, `FreezeLevelStreaming`). Output: for
   each sublevel, the volumes, states and conditionals that load or show it.
5. **Kismet:** remote events, named, scoped and external variables, `AttachToEvent`, with
   default link labels resolved. Compare with a fresh LEC dump of every sequence object,
   not with `kismet.json`.
6. **Trigger placement:** originator, attached actor, or `BioSeqVar_ObjectFindByTag` →
   position and bounds. Prefab templates are separated from world events.
7. **Starts:** every kind listed under "Starts", walked back to terminal events with the
   plot checks on the way. `BioSeqEvt_Conversation` fires from a conversation and isn't a
   start. Share the start kinds with graph export step 2 instead of finding them twice.
8. **Connections:** teleports, doors, area transitions, start locations and galaxy-map
   destinations (`BioPlanet.MapName`) as links between places. No walkable geometry.
9. **The visit query,** by conversation, level or plot id.
10. **Widen** to one combat level, then to every LE3 level, package class by package
    class.

### coredcc fit test (beside step 2)

- **Question:** can coredcc's `ApiStruct`s and datapaths describe UE3 objects, whose
  properties depend on their class?
- **Test:** after step 1, at a pinned coredcc commit, build `ApiStruct`s on demand for a
  few real Kismet and actor classes, including a nested struct, an array, a class default
  and an object reference. Set one value in memory through a command, undo it, and compare
  with the original. Nothing is written to disk.
- **Result:** what fits, what doesn't, the cost per class, and what would have to change
  in coredcc or here. Recorded in this plan before milestone 2 is planned.
- Nothing in steps 2-10 waits for it or links coredcc.

## Done when

- **Reading:** no unexplained difference from LEC, for the classes the tool uses.
- **Kismet:** no unexplained difference from the LEC dump on the level sets covered.
- **Trigger events:** every one is in a counted bucket (stored originator, static attach,
  find by tag, prefab template, runtime object, none), and the buckets sum to LEC's total.
- **Starts:** at least 1,660 of the 1,662 plot-touching conversations have a start op. The
  share that reaches a resolved place or load condition is measured on the Normandy, and a
  target for all of LE3 is set from that before step 10.
- **Known cases:** Legion's and Tali's starts and splice points come out as they were
  found by hand, with Tali's sublevel resolved through `PlotStreaming`.
- **Streaming:** predictions match step 3's run.
- **coredcc:** the fit test's result is recorded, whichever way it goes.

## Out of scope

Package edits, LE1 and LE2, rendering, player collision, walkable reachability, floor
maps, simulating Kismet over time, and anything that depends on how a save was made
(loading inside a level skips its load logic).

## Later milestones

Only the order of 2 and 3 is decided. Their contents are proposals, and each gets its own
plan.

- **2, edits:** coredcc (as the fit test allows), undo, a committable command journal,
  package writing compared export by export on read-back, and Kismet edits.
- **3, spatial:** collision geometry, spatial queries, walkable floors, reachability,
  floor maps and placement queries, checked in game.
- **4, new content:** geometry, lighting and whole levels, and LE1 and LE2.

## Review

By an agent with fresh context, 2026-11-24. It measured the exports, and the packages with
a read-only LEC probe. Checked again by the author: `PlotStreaming`'s existence, the
missing `BioH_Tali`, the teleports, the exported class list, the variable-linked state
names and the Normandy's path points. The rest is the reviewer's measurement.

| #   | Finding                                                                                                   | Resolution                                                               |
| --- | --------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| 1   | Streaming left out `PlotStreaming`, nested levels, variable-linked states and native precedence           | Steps 2–4 rewritten, with an in-game check first                         |
| 2   | Collision reachability is wrong on the Normandy (teleports), the largest step, and tested by no criterion | Step replaced by connections; walkable reachability is in milestone 3    |
| 3   | Most unresolved starts are found by searching every object property; "or a stated reason" couldn't fail   | Step 7 and the criterion rewritten; start kinds shared with graph export |
| 4   | `kismet.json` is partial, so matching it proves little                                                    | Compared with a full LEC dump                                            |
| 5   | The 1,590 events are mostly prefab templates and attachments                                              | Buckets in step 6 and the criteria                                       |
| 6   | Many chains don't end at a place, and remote events over-connect                                          | Opaque sources; events scoped to co-loaded levels                        |
| 7   | Session commands contradict a one-shot CLI; nothing in the question needs coredcc                         | Out of the main path, by the user's decision; a side fit test instead    |
| 8   | Matching every package first delays any answer, and its criterion couldn't fail                           | Normandy first; widened in step 10                                       |
| 9   | Entry points are partly data                                                                              | Step 8; the open question is closed                                      |
| 10  | "No nav nodes on the Normandy" was false: 37 `BioPathPoint`s                                              | Fact corrected                                                           |

- **Added after the review,** so not covered by it: the coredcc fit test.
- **Survived:** the event and streaming counts, no `Pylon`, the compression count, the
  Patch01 copy, and that class data can be read from packages.
- **Not checked by anyone:** anything in game, decompression or parsing in C++, coredcc's
  performance, whether prefab instances copy originators, and a combat level.
