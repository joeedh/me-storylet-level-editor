# Conversation speakers in LE2 and LE3

How vanilla conversations bind a line to a speaker, how Shepard speaks lines that aren't
choices, and how BioWare handles speakers that aren't visible in the level. Measured
2026-11-25 by an agent with a read-only LEC probe, over 1,147 LE2 and 2,396 LE3
conversations (one copy of each). The probe and its outputs stayed in the session
scratchpad. Marked **(inferred)** where it isn't measured.

## Speaker lists and indices

- **Indices:** -2 is the player, -1 the conversation's owner, and 0 and up index the
  speaker list. LEC adds -2 and -1 itself (`Dialogue/ConversationExtended.cs:536-560`).
- **The lists hold actor tags, not display names.**
    - LE2: `m_SpeakerList`, an array of `BioDialogSpeaker { Name sSpeakerTag; transient
      Actor aSpeaker }`.
    - LE3: `m_aSpeakerList`, an `array<Name>`.
- **Face sets** are indexed by speaker index + 2: player, owner, then the list. Vanilla
  names follow it (`FXA_<conv>_Player_M`, `_Owner_M`, `_<tag>_M`).
- **Empty lists are common:** 639 of 1,147 LE2 and 956 of 2,396 LE3 conversations.
- **Most tags are the party** (`hench_*`), spawned at run time. LE2 has 253 distinct
  tags, LE3 597.

## How an index becomes an actor

- **Native code resolves it** (`GetSpeaker`, `GetConversationObject`). Both executables
  hold the message "GetConversationObject: %s Failed to retrieve speaker list index %d
  with tag %s", so the lookup is by tag **(inferred from the string and the match
  rates)**.
- **The start op binds only the owner (-1).** The player pawn is -2. Speakers 0 and up
  aren't linked on the start.
    - LE2 `BioSeqAct_StartConversation` has `Owner` and `Stage` links.
    - `SFXSeqAct_StartConversation` adds puppet and stage-node links, and LE3 adds
      `Extras`.
    - Owners in LE2 (1,460 starts): 400 find-by-tag, 363 a pawn, 305 a point of interest,
      52 the player, 80 empty.
- **LE2 speaker slots against the actors in their level sets** (nested streaming
  followed): 2,371 party, 216 a pawn, 65 only a point of interest, 10 a skeletal mesh
  actor, 1 an emitter, 40 nothing.
- **LE3 named characters are mostly spawned from archetypes**
  (`biochar_global.Archetypes`) that carry the tag. 68 tags (364 entries) match
  nothing, mostly combat spawns.
- **Each node's matinee "Conversation" group is never linked in Kismet** (24,824 of
  24,845 in LE2), so the conversation binds it to the current speaker at run time
  **(inferred)**.

## Shepard's lines

- **No entry uses -2, and no tag means the player.** Shepard speaks through replies.
- **A line that isn't a choice** is a `REPLY_STANDARD` that is its entry's only reply, has
  text, and has a paraphrase id with no TLK string: 1,402 in LE2, 9,987 in LE3.
- **Variants** are conditional sibling replies with no paraphrase: 28 LE2 entries, 491
  LE3.
- **Wheel choices** have paraphrase text: 13,966 LE2 links, 9,363 LE3.
- **Every vanilla link has a positive paraphrase id.**
- **Two Shepard lines in a row, or Shepard first,** go through a silent entry: no text,
  and usually no node matinee. 383 of LE2's 1,402 follow one.
- **`REPLY_AUTOCONTINUE` and `REPLY_DIALOGEND` never have text** (0 of 10,224 and 25,263).
- Outside conversations, Shepard's barks are face-only VO actions with speaker tag
  `Player`.

## Speakers with no visible actor

- **An invisible `SFXPointOfInterest` with the speaker's tag** is the usual route.
    - LE2: EDI (`hench_ai`) on missions, and radio voices (`shdli4_liararadio`,
      `arv_commofficer`, `hench_radio_professor`). The points of interest carry only
      `Tag`.
    - LE3: about 60 `radio_*` tags, computer voices, announcers, and the dream voices
      (`dream_ashley`, `dream_kaidan`). Points of interest are the only actors for 3,291
      entries.
- **A hologram is a real pawn streamed in.** ME2's Illusive Man is `nor_illusive_man`, a
  `BioPawn` in `BioD_Nor_104aCommIM`.
- **The owner bound to the player:** some briefings (`n7bldinv1_fog_d`) have empty
  speaker lists, start with Owner = the player, and put EDI's voice on -1.
- **Face-only VO from Kismet:** `BioSeqAct_FaceOnlyVO` names a conversation, a speaker
  tag and an actor. Arrival's Harbinger plays this way on a point of interest. Only
  face-only VO can override the displayed name (`srActorNameOverride`); conversation
  entries can't.
- **Cutscenes** (ME2's Prologue) play lines from matinee tracks, with subtitles on
  separate `BioEvtSysTrackSubtitles` tracks.
- **A few vanilla speakers resolve to nothing** in their level set, all voiced: the last
  N7 banter line (`n7gen_awayteam_vi`), Leviathan in `expak1_m3_meet_daughter_m`. Kismet
  can spawn and tag actors, so this doesn't prove there is no actor.

## Unchecked until tested in game

- Whether an entry whose speaker resolves to nothing plays, shows its subtitle, or ends
  the conversation. One executable string says a null speaker on the last entry stops
  it.
- What name an LE2 subtitle shows for a point-of-interest speaker, an owner-bound line,
  or a null speaker. LE2 sets `m_bDisplayNonAmbientName`.
- Whether LE2 unvoiced lines behave like LE3's: up for the matinee's length, and
  skippable ones waiting for a skip. LE2 has `m_bAutoAdvanceSkippableLinesWithNoVO`
  false and `m_fSubtitleTimingMinimum` 2.0.
