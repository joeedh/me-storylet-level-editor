# Milestone 1: package round trip and one unvoiced LE3 conversation

Status: pressure-tested and revised, 2026-11-25 (see "Review"). No code exists. Step 0
is done apart from the dump command: the game loads an uncompressed level.

## Goal

- Read an LE3 package, write it back unchanged, then build one unvoiced conversation and
  its start into a copy of a host level.
- The input is the `conversation-spec.json` that the study's `storylet compile` writes.
  The output replaces `me-gamedata-export`'s `add-conversation`, the first interim
  builder to be retired.
- The proof is in game: the conversation plays as the builder's did.

## Decided by the user

- **2026-11-25:** milestone 1 is this round trip, ending in one unvoiced LE3 conversation.
  LE2 comes straight after it. The visit query
  (`2026-11-24-level-tool-visit-slice.md`) moves to a later milestone.
- **2026-11-25, order of work** (study `docs/plans/2026-11-10-in-game-storylets.md`,
  `802fd06`): this tool does the round trip and LE3 conversation, then the LE2
  conversation route, in parallel with the study's engine, overlays and scene format.
  The study asked that an LE2 package join the round trip early if it is cheap (step 3).
- **2026-11-25, step 0:**
    - the LEC dump is a read-only `dump` command in `me-gamedata-export`;
    - `add-conversation` may be run for the baselines, into `build/` here;
    - the user runs the uncompressed-load test in game.
- **2026-11-25:** UnrealScript work is to use UDK, at `C:\UDK\UDK-2015-01`. No
  conditionals are written in this milestone.
- **2026-11-25:** an override of a base-game LE1 or LE2 conditional ships as `.pmu` text,
  which Mod Manager compiles. Not in this milestone.
- **Carried over** from the visit-slice plan and `CLAUDE.md`:
    - C++, LLM use first, through a CLI that prints JSON.
    - LEC isn't linked. It is used only outside the tool, to produce dumps for comparison.
    - The tool never writes to a game install. Oodle is loaded from the game's DLL
      (`Binaries\Win64\oo2core_8_win64.dll`).
    - The first edits bring coredcc in. The user says it has what this tool needs now.

## Test cases

Both are study change sets whose conversations `add-conversation` built and that were
played in game.

- **`changes/talitalks.yaml`:** Tali after peace. Host
  `DLC/DLC_EXP_Pack001/CookedPCConsole/BioD_Nor_405Engineering.pcc`, template conversation
  from its `_LOC_INT` package, spliced `after` conditional 847's False output,
  `stage: owner`, with diagnostic bools. Played on `Save_0038` (study
  `docs/plans/2026-11-21-tali-conversation.md`). The built level's sha256 starts
  `b5c17671`.
- **`changes/legion-ceasefire.yaml`:** spliced `beforeStart`, `stage: copy`, with
  transition 9000 on reply 9001402. Played on `Save_0032` (study
  `docs/plans/2026-11-24-legion-scene-transition.md`). Its built level isn't kept, so step
  0 rebuilds it.
- **Neither case covers:** a node with no bool, no diagnostics, given face sets,
  `REPLY_AUTOCONTINUE`, or a skippable entry. Step 8 deals with these.

## What the builder does

`me-gamedata-export`'s `ConversationBuilder.cs` (38 KB) is the reference. Its README
section "Adding a conversation to an LE3 level" has the rules.

- **The conversation:**
    - Copies the host level to the output folder and opens the copy.
    - Clones an entry struct and a `REPLY_STANDARD` reply struct from the template
      conversation, and edits them for each node.
    - Adds a `_D` package to the level with a `BioConversation` and its
      `Node_Data_Sequence`. It sets `m_StartingList`, `MatineeSequence`, the face sets,
      `m_pNonSpeakerFaceFXSet` and `m_nResRefID`.
- **Each node:** a `BioSeqEvt_ConvNode`, a `SeqAct_Interp` with an `InterpData` lasting
  the line's length, an `InterpGroup` (with `GroupName` and `GroupColor`) holding a
  `BioEvtSysTrackVOElements` track, a `BioSeqAct_EndCurrentConvNode`, and a
  `SeqAct_SetBool` if the node sets a bool.
- **The start:** splices a conditional check and a conversation start into the host's
  sequence.
- **Package details** (measured by the review on the builder's output):
    - New exports are appended. Source exports and imports keep their indices, and the
      source names are a prefix of the new name table.
    - About 10 new imports: the template's FaceFX anim sets, and the classes it creates
      (`SFXGame.BioConversation`, `Engine.SeqAct_SetBool` and others). It needs the host's
      existing `Engine.SeqVar_Object` import.
    - Flags: `ForcedExport` on everything under the `_D` package, the vanilla
      conversation, node-sequence and package flags, and 0x7000100000000 on sequence
      objects. NetIndex is 0 on new exports.
    - Object names are numbered package-wide, as the highest number in use plus one
      (LEC's `UnrealPackageFile.cs:325`), giving `SeqAct_Interp_44` and the like.
- **Defaults of new sequence objects** (LEC's `SequenceObjectCreator.cs:121-405`):
    - Only the four link arrays (input, output, variable and event links) come from the
      class-default chain.
    - Some values are hard-coded (`m_nIndex` 0 or -1, `bValue` 0).
    - `ObjInstanceVersion` comes from LEC's own table: `Sequence` 3, `SeqAct_Interp` 6,
      `SFXSeqAct_StartConversation` 5, the rest 1. No class package holds it. The
      vanilla instances have the same values.
    - The class defaults hold more than LEC copies; `Default__BioSeqEvt_ConvNode` sets
      `m_nNodeID` -1, for example. So "copy the defaults" wouldn't match the builder.
- **Checks:** reads the copy back against the spec, then compares it with the host export
  by export.
- **Saving:** `pcc.Save()` compresses LE packages by default (`PackageSaver.cs:53-61`).
  Every level played in game so far was Oodle-compressed.

## Package facts (LE3, measured by the review)

On the two hosts and their `_LOC_INT` templates:

- Every name is UTF-16. The dependency table is all zeros. Export data is in table order
  with no gaps. There is one generation and no `AdditionalPackagesToCook`.
- LEC's uncompressed save changes no export's data, keeps the name and import tables
  byte-equal, and changes only each export entry's data offset.
- Writing uncompressed means clearing the `Compressed` package flag (0x02000000) and
  dropping the chunk table.
- An unedited round trip never moves an offset: the uncompressed header is 142 bytes, the
  same as where the compressed file's chunks start.
- **Offsets that binary data records,** and which move once an edit grows the tables:
    - `ShaderCache`: LEC fixes these (`MEPackage.cs:1303`).
    - `Texture2D` mips stored in the package: 6 absolute offsets per texture (4 textures
      in `405Engineering`, 78 in `204Gth`). LEC fixes these only for ME1
      (`MEPackage.cs:1274`). The builder's Tali level has them stale, and it played.
    - Possibly `SkeletalMesh`, `StaticMesh`, `WwiseBank` and FaceFX bulk data: a
      heuristic scan found them, unconfirmed.
- **Schemas:** every class and struct the builder creates is defined in
  `CookedPCConsole\SFXGame.pcc` or `Engine.pcc`, with a default object. Struct
  inheritance must be walked (`BioDialogEntryNode` and `BioDialogReplyNode` extend
  `BioDialogNode`). Transient members such as `SeqVarLink.CachedProperty` are skipped.
  Immutable structs (`Color`, `Vector`, `Guid`) are known from their struct flags.
- **`Package`** is intrinsic: no class package defines it, and LEC gives it class index 0.

## Design choices

- **Output is uncompressed.** Step 0 showed the game loads an uncompressed level as it
  does a compressed one.
- **Offsets:** every confirmed kind is fixed (`ShaderCache` and `Texture2D` first), so the
  file is correct rather than matching LEC's stale values. A class with offsets the tool
  can't fix yet stops the write, naming the export. Comparisons mask the fixed fields.
- **Properties are written in the class's property order,** as the engine writes them, not
  in LEC's insertion order. The byte test is against vanilla, not against LEC: every
  vanilla export of the builder's classes, read and re-serialised, must come out
  byte-equal. Objects built by LEC are compared by value.
- **New objects follow the builder's rules exactly:** its default selection,
  `ObjInstanceVersion` table, flags, names and numbering, so that step 7 can pair every
  export by path.
- **Edits are coredcc commands that store deltas.** coredcc's history budget defaults to
  256 MB and drops the oldest commands past it (`executor.h:144,201`). Snapshots of a
  26 MB level would overflow it after about 10 commands.

## Steps

0. **Before any code:**
    - **Uncompressed load, in game.** Install an uncompressed copy of Tali's baseline
      level (below) through the talitalks kit, in place of the compressed one. On
      `Save_0038` the level must load and our conversation must play as it did. This
      decides the first design choice.
    - **Baselines.** Rebuild both cases with `add-conversation` into `build/`. Tali's must
      reproduce `b5c17671`, which also shows the builder is deterministic. Record
      Legion's hash.
      Done 2026-11-25 with the exporter's Release build of `084be93`, twice per case
      into `build/baselines/`: both runs were byte-identical, Tali's is `b5c17671`, and
      Legion's `BioD_Nor_204Gth.pcc` is `62e910ca`.
    - The uncompressed copy for the in-game check is LEC's save of Tali's baseline,
      `build/baselines/talitalks/uncompressed/` (sha256 `12ed2ca6`). LEC changed no
      export data, name or import, and cleared the compression flag.
    - Installed 2026-11-25 as the library mod `talitalksuncompressed` (a copy of
      `talitalks` with only the level replaced), with Mod Manager's `--installmod`.
      Mod Manager left the uncompressed level hash-identical.
    - **Result, 2026-11-25:** the user played it on `Save_0038`, and it behaved the same
      as the compressed level. PlotHarness wrote no log that run (its last entry is from
      2026-11-24), so the plot writes weren't compared. Output is uncompressed.
    - **Dumps.** A read-only `dump` command in `me-gamedata-export`: a package's tables,
      properties and binary offsets, to compare with.
1. **Build setup:** CMake 3.25 and C++20, like coredcc, with coredcc pinned to a commit.
   A test target runnable by `ctest`, using coredcc's test framework if it fits. Whether
   this tool builds without exceptions and RTTI, as coredcc does when it owns the build,
   is decided here.
2. **Read LE3 packages:** the header, Oodle chunks, the name, import and export tables,
   tagged properties, and class schemas from `SFXGame.pcc` and `Engine.pcc`. Compare
   with the LEC dump of both hosts and both templates.
3. **Write a package back unchanged,** uncompressed. Check:
    - each export's data, the name table and the import table equal the source's;
    - in game, both hosts still load and their usual talk plays.
    - **LE2, if cheap:** read and write back one LE2 level the LE2 route will use, with
      the same byte checks. The study's ME2 route needs LE2 writes early. Not needed to
      finish this milestone.
4. **Shifted round trip:** add one name and one empty export, so every offset moves. Check
   the `ShaderCache` and `Texture2D` offsets against their exports' new positions, and
   against LEC's save of the same edit for `ShaderCache`.
5. **Edits through coredcc:** adding names, imports and exports, and setting properties,
   are commands. Undoing every command gives back step 3's bytes, for both cases' full
   edit lists.
6. **Write tagged properties** from the schemas. Check: every vanilla export of the
   builder's classes in both hosts re-serialises byte-equal.
7. **The conversation writer:** everything in "What the builder does", from the same spec,
   with the same checks against the spec and the host. Then compare with step 0's
   baselines export by export: same paths and classes, equal property values, object
   references compared by path. Expected differences: property order, the fixed offsets,
   and compression.
8. **Paths the cases don't cover:** a node with no bool, no diagnostics, given face sets,
   `REPLY_AUTOCONTINUE`, and a skippable entry. Each is built by both tools from a small
   spec and compared as in step 7, or dropped from the writer and from the spec.
9. **In game:** each case on its save, with the interim builders' other outputs installed
   (strings from `add-strings`, the `.cnd`, and for Legion the startup package with
   transition 9000).
    - Compare PlotHarness's log with the builder's runs: which bools were written, and
      their order. Gaps between writes must match the `InterpData` lengths within a
      tolerance, since timing doesn't repeat exactly.
    - Tali's case first.
10. **Retire `add-conversation`:** study `docs/compiling.md` points at this tool. Removing
    the builder from the exporter happens in that repo, when asked.

## Done when

- **In-game load:** settled on 2026-11-25: the game loads an uncompressed level.
- **Round trip:** both hosts and both templates write back byte-equal in data and tables,
  both hosts load in game, and the shifted round trip has correct offsets.
- **Properties:** every vanilla export of the builder's classes in both hosts
  re-serialises byte-equal.
- **Conversation:** both cases pass the spec and host checks, match the baselines apart
  from the listed expected differences, and play in game with the same bool writes in the
  same order as the builder's runs.
- **Coverage:** every spec field the writer accepts was built and compared in step 7 or 8.

## Out of scope

- TLK strings, `.cnd` files and transitions: the interim builders keep writing them.
- LE1, LE2 conversations and conditionals. LE2 reading and writing may start (step 3).
- Voiced lines, cinematics, and the visit query.
- Writing compressed packages: step 0 showed they aren't needed.
- The scene format's new needs (several speakers, gendered text, conditions on links):
  the writer reads today's spec here, and the scene format extends it later.

## Open questions

- **The heuristic offset hits** (`SkeletalMesh`, `StaticMesh`, `WwiseBank`, FaceFX).
  Step 2 confirms or clears each, and the write refuses any it can't fix.
- **Whether coredcc's document model fits package edits.** The reviewer read only its
  command and undo API. Step 5 finds out.
- **One writer per package per build:** the TLK, `.cnd` and transition builders write
  other files, so no file is written by both serialisers here.

## Review

By an agent with fresh context, 2026-11-25. It read the builder, LEC and coredcc, and
measured the four packages and the builder's Tali output with a read-only LEC probe
(output in the session scratchpad only). The author checked findings 1 and 4 in LEC's
source.

| #   | Finding                                                                                  | Resolution                                                       |
| --- | ---------------------------------------------------------------------------------------- | ---------------------------------------------------------------- |
| 1   | LEC compresses LE saves by default; no uncompressed level has been played                | Step 0: the game loads it (2026-11-25)                           |
| 2   | An unedited round trip moves no offset, so the offset check couldn't fail                | Step 4, a shifted round trip                                     |
| 3   | `ShaderCache` and `Texture2D` record absolute offsets; LEC leaves `Texture2D` stale       | Fix every confirmed kind; refuse unknown ones; mask in compares  |
| 4   | New objects' defaults are LEC's selection plus a version table, not the class defaults   | "What the builder does" lists the rules; the writer copies them  |
| 5   | LEC writes properties in insertion order, so byte-matching LEC tests nothing useful      | Class order; byte test against vanilla re-serialised             |
| 6   | The builder does more than the plan listed (imports, flags, naming, conversation fields) | Listed under "Package details"                                   |
| 7   | Legion's built level isn't kept, and Tali's builder output may not be reproducible        | Step 0 rebuilds both and checks Tali's hash                      |
| 8   | In-game timing doesn't repeat                                                            | Step 9 compares bool writes and order, gaps within a tolerance   |
| 9   | Both cases leave several spec paths untested                                             | Step 8                                                           |
| 10  | Missing: build setup, the dump tool's home, install of the round-trip level              | Steps 0 and 1; the dump is an exporter command (user)            |
| 11  | coredcc's 256 MB history would drop commands if they hold snapshots                      | Commands store deltas                                            |

- **Survived:** a byte-preserving round trip is possible (LEC's uncompressed save of all
  four packages changed no export data); the schemas are in two class packages; new
  exports are appended; Oodle ships with the game; the test cases match their specs; one
  writer per file holds.
- **Not checked by anyone:** whether the game loads an uncompressed level; whether the
  builder is deterministic; the heuristic offset hits; coredcc's document model; Mod
  Manager's handling of an uncompressed file; anything written in C++.
