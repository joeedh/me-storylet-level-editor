# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project rules

### Game data

- Generated output and anything else derived from game files stays in `build/`, which is
  gitignored. Never commit it.
- The tool never writes to a game install. Edited packages go to an output folder.
- Oodle is loaded from the game's own DLL at run time. Never commit or redistribute it.
- Formats and logic may be learned from Legendary Explorer (LEX), but the code here is our
  own. Its core library (LEC) isn't linked, and is used only outside the tool, to produce
  dumps for comparison.

### Plans

- One file per plan in `docs/plans/`, named `YYYY-MM-DD-short-topic.md`.
- Keep the plan updated as the work proceeds: its status, and decisions that changed.
- Record the pressure test in a "Review" section of the plan, before starting the work:
  each finding, how it was resolved, what survived, and what nobody checked.

### Docs

- The only Markdown files outside `docs/` are `Readme.md` and this file.
- Markdown is wrapped at 90 columns.
- Don't copy styling between docs. Write each doc in the structure and format its own
  content calls for.
- When you diagnose a non-obvious problem, add it to `docs/debugging.md`. Correct entries
  that turn out to be wrong instead of appending contradictions.

### Other

- Before finishing a task, list leftover temporary comments with
  `git grep -n "CLAUDENOTE:" -- ":!CLAUDE.md"`.
- Don't put model identifiers in commits or other repository content.

## Project status

A headless C++ tool that loads Mass Effect Legendary Edition levels, analyses their Kismet
and answers spatial questions, and makes undoable edits. LLMs use it first, through a CLI
that prints JSON.

It owns writing mods (the user's decisions, 2026-11-24 and 2026-11-25):

- conversation authoring. The MVP is custom unvoiced conversations; advanced dialogue and
  cinematic sequences come later;
- Kismet edits;
- conditionals: LE3's `.cnd` files, and LE1's and LE2's, which are compiled UnrealScript;
- transitions, TLK strings and a mod's Coalesced file;
- building the packages.

Until each writer exists, `me-gamedata-export`'s builders do that job.

- **Current plan:** `docs/plans/2026-11-25-conversation-round-trip.md`, milestone 1: an
  LE3 package read and write round trip ending in one unvoiced conversation. LE2 comes
  next (the user's decisions, 2026-11-25).
- **Later:** `docs/plans/2026-11-24-level-tool-visit-slice.md`, the read-only visit query,
  and the other later milestones it lists.
- **Layers planned:** `ue3` → `level` → `kismet` → `app` → CLI. `ue3` doesn't depend on
  coredcc.

### Layout

- `source/<layer>/`, include root `source/`. So far:
    - `ue3/`: packages. `result.h` is its error type (`Status`, `Result<T>`, `UE3_TRY`),
      since `ue3` can't use coredcc's.
    - `cli/`: `melevel`. Every command prints one JSON document; errors are
      `{"error": {"message": ...}}` with exit code 1.
- `extern/coredcc/`: coredcc as a submodule, pinned to a commit. Its own submodule is
  litestl. Clone with `--recursive`, or run `git submodule update --init --recursive`.
- `tests/`: one executable per test on litestl's harness (`test_init`, `test_assert`,
  `return test_end()`), registered with `melevel_test` (`cmake/MelevelHelpers.cmake`).
- **No exceptions and no RTTI,** in every target, coredcc and litestl included (the
  user's decision, 2026-11-25). Use the `error_code` overloads of `std::filesystem` and
  `std::from_chars`, never anything that reports failure by throwing.
- Our targets build with `/W4 /WX`.

### Build and test

Through coredcc's `extern\coredcc\tools\devshell.cmd`, which loads the Visual Studio 18
environment (from Git Bash, set `MSYS_NO_PATHCONV=1`, or use PowerShell). Presets are
`native-clang` (clang-cl) and `native-msvc`, Ninja Multi-Config, with `-debug` and
`-release` (RelWithDebInfo) build and test presets. Output goes to `build/<preset>/`.

- Configure: `extern\coredcc\tools\devshell.cmd cmake --preset native-clang`
- Build: `extern\coredcc\tools\devshell.cmd cmake --build --preset native-clang-debug`
- Test: `extern\coredcc\tools\devshell.cmd ctest --preset native-clang-debug`
- Before committing code, build and test all four presets.

## Related repos

All are local checkouts in `C:\dev`. Other sessions may be working in them, so make
changes there only when asked, and in a worktree.

- `mass-effect-storylet-study`: the narrative graph and storylets. It owns the meaning of
  plot logic and the storylet compiler, which produces the edits this tool makes.
  `docs/gamedata.md` there defines the JSON schemas our output matches.
- `me-gamedata-export`: the C# exporter built on LEC. It owns reading the games: the JSON
  exports and its inspection commands. Its mod builders (`add-conversation`,
  `write-transitions`, `override-transitions`, `add-strings`, `add-emails`,
  `compile --write-cnd`) are interim: they stay in use until this tool replaces each, and
  get no new features.
- `coredcc`: the C++20 DCC library that edits use for commands and undo. The user says it
  has everything this tool needs now (2026-11-25), so the fit test doesn't hold up
  designing the first writer.
- `LegendaryExplorer`: LEX and LEC source, for reference.

The user has also installed UDK, at `C:\UDK\UDK-2015-01`, for the UnrealScript tooling.

<!-- default-rules:start (generated by the create-claude-md skill; edit Project rules instead) -->
## Default rules

### Code comments

- Ordinary code comments (not doc comments) are at most 5 lines.
  - Exception: about one longer comment per 200 lines of code is allowed, for things like a
    section overview or an explanation of a tricky algorithm.
- Doc comments (docstrings, JSDoc, Doxygen, `///`, and the like) have no length limit.
- Math-heavy comments (derivations, formulas, numerical reasoning) have no length limit.
- Temporary comments have no length limit, but must start with `CLAUDENOTE:` so they can be
  found and stripped later (`grep -rn "CLAUDENOTE:"`).

### Documentation and plans

- Documentation goes in `docs/`.
- Plans go in `docs/plans/` and must be committed to the repo, not left in a scratch folder
  or only in the conversation.
- Pressure-test every plan before implementing it, which means trying to falsify it. Always
  do this in a fresh subagent: give it the plan and the code, but not your reasoning, so its
  attempt is not anchored on the author's view. Tell it to:
  - List the claims the plan depends on (about the code, the data, performance, APIs) and
    try to prove each one false: read the code, run a quick experiment, or find a
    counterexample.
  - Construct concrete inputs or scenarios that would make the plan fail.
- Record in the plan file which claims survived, which broke, and how the plan changed.
- Keep a running debugging guide in `docs/debugging.md`. When a bug takes real effort to
  track down, add an entry: symptom, cause, fix, and how to spot it next time.

### File size

- When a source file grows past 35 KB, split it into smaller files.
- Split along real boundaries of responsibility, and give the pieces a subdirectory structure
  that reflects them (for example `parser/lexer.ts`, `parser/ast.ts`), not `foo1.ts` and
  `foo2.ts`.
- Generated files, vendored code, lockfiles, and data files are exempt.
<!-- default-rules:end -->
