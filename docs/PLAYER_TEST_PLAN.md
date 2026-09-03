# Roadside Idiots — Player Test Plan

## Why this exists

A build can be technically correct and still not be fun. Roadside Idiots should be evaluated around the player fantasy:

> **I am a competent rider surviving and outsmarting idiots.**

The test plan therefore measures both function and player experience.

---

## Pre-test packaged-build smoke gate

Do this once on the exact ZIP that will be handed to outside testers. Do not use the Editor build for this gate.

### Input/controller contract
- keyboard arrows navigate setup/settings;
- Enter confirms menu selections;
- Xbox-style D-pad navigates setup/settings;
- A confirms menu selections;
- B returns from Settings and resumes from Pause;
- P/Start opens and closes Pause during an active race;
- Esc opens Pause during an active race and behaves as a back action in menus;
- the Pause menu contains an explicit **MAIN MENU** row;
- Pause → MAIN MENU reloads the setup screen and does **not** auto-start a race;
- during gameplay A still drops a peel, B still throws an egg, X recovers, LB/RB slap;
- finish a race and verify **Enter**, **A**, and **Y/top-face** can start the same configured race again;
- pressing Y during an unfinished race must **not** unexpectedly reload the map;
- after finish, **P/Start** must not replace the result screen with Pause;
- after finish, **Esc/B** must return to Main Menu/setup without auto-starting;
- after a finish restart, opponent/lap/traffic/chaos settings must be preserved.

Run `tools\verify_input_contract.ps1` before packaging. During a runtime test, `RI INPUT FLOW` should appear in the log; using Y after finish should produce `RI INPUT FINISH_RESTART source=Y`; returning to Main Menu should produce `RI INPUT MAIN_MENU source=...` when logging is available.

### Frozen-driving regression gate
- run at least one 5-opponent / 4+ traffic race;
- no recurring AI wall oscillation;
- no new flat-road bump/jump behavior;
- intentionally ride across several visible asphalt repair patches/skid streaks: they must have **zero** physical effect;
- traffic visual changes must not alter the existing impact volume/route behavior;
- finish the race and confirm no presentation physics/collision warning spam.

### Presentation/content gate
- approved free tropical/banana vegetation is visible and does not enter the racing corridor;
- `RI FREE VEGETATION` reports loaded approved assets when logging is available;
- market/bus-stop facade details and distant backdrop load;
- road markings/surface wear remain readable without overpowering hazards/items;
- no SankoolArts / `CompoundWall_Kit` content or references are present.

If any smoke-gate item fails, fix it before recruiting outside testers. Do not ask testers to evaluate fun while basic input/build integrity is broken.

---

## Who to test with

Do not overfit to one age or one existing fanbase. Recruit by play preference when possible.

Useful groups:
1. arcade-racing players,
2. Road Rash / combat-racing nostalgia players,
3. Wreckfest / funny-physics / destruction players,
4. casual PC players who rarely play racing games,
5. South-Asian/Bangladesh players who can judge whether the roadside identity feels recognizable and affectionate rather than generic or insulting.

A small mixed group is more useful than ten people who all play the same type of racing game.

---

## First-session test

Give the tester the packaged ZIP, not the Unreal Editor.

Do **not** explain the game verbally unless they become blocked. Observe whether the UI and game teach themselves.

### Task A — launch and configure
Ask the tester to:
- launch the game,
- choose any race setup they want,
- look at CLEAN / BALANCED / MAYHEM,
- open Settings,
- start a race.

Observe:
- Can they navigate without help?
- Do they understand what Race Chaos means?
- Do keyboard and controller prompts match what actually happens?
- Do they choose settings quickly or get stuck comparing meaningless options?

### Task B — first race
Do not tell them how to win.

Observe:
- Do they understand acceleration/braking/steering?
- Do they notice place/lap/minimap without staring at the HUD?
- Do they discover slap/peel/egg?
- When something hits them, do they understand what caused it?
- Can they recover emotionally and mechanically after a crash?
- Do they laugh/react to any emergent event?
- Do they blame themselves, a rival, traffic, or “the game” after a loss?

The worst answer for the project is repeated “the game just randomly screwed me.”

### Task C — second race
Ask them to deliberately change at least two setup dimensions, for example:
- lower/higher traffic,
- CLEAN vs MAYHEM,
- more opponents,
- different steering feel.

This checks whether player choice creates a noticeable experience difference.

---

## Post-race questions

Use a 1–5 score followed by one sentence explaining why.

### Core scores
1. **Riding control:** “I felt in control of the motorcycle.”
2. **Competence:** “When I drove well, the game rewarded it.”
3. **Fairness:** “When I lost position, I usually understood why.”
4. **Rival interest:** “The opponents felt like personalities rather than identical bots.”
5. **Chaos readability:** “The chaotic moments were understandable rather than random.”
6. **Recovery:** “A mistake/crash hurt, but I still felt I could continue the race.”
7. **Feedback:** “Hits, items, damage and important race events were clear.”
8. **Replay desire:** “I wanted to start another race.”
9. **Game identity:** “This felt different from a generic racing prototype.”
10. **Fun:** “I enjoyed the race.”

### Open questions
- What was the best moment of the race?
- What was the most annoying moment?
- Did any rival stand out? Why?
- Did anything feel unfair or impossible to understand?
- Was there a moment where you felt clever/skilled?
- Was there a moment where the game felt out of your control?
- What would make you immediately play one more race?
- What is the first thing that still makes this feel like a prototype?

---

## Behavioral metrics worth adding later

Technical telemetry should support observation, not replace it.

Per race:
- completion / restart / quit,
- finishing position and time gap,
- overtakes gained/lost,
- barrier collisions,
- traffic collisions,
- rival collisions,
- recovery uses,
- item pickups,
- peel drops and hits,
- egg throws and hits,
- slap attempts and hits,
- player condition at finish,
- deliberate AI chaos events per minute,
- time between major incidents,
- time spent within interaction range of another rider.

Useful derived signals:
- **incident density** — too low may feel empty; too high becomes noise,
- **recovery burden** — how much race time is lost to being helpless,
- **item clarity** — high usage with very low hit/understanding rate suggests poor feedback,
- **race closeness** — useful, but never optimize it through obvious cheating,
- **replay conversion** — whether players voluntarily start another race.

---

## Acceptance heuristics

These are product heuristics, not statistical claims.

### Driving
- clean-road AI should almost never hit barriers by itself,
- player steering should feel predictable at high speed,
- traffic collisions are allowed but should feel avoidable/readable,
- recovery should return the player to meaningful control quickly.

### Chaos
- the opening seconds establish racing before deliberate chaos,
- not every rival attacks at the same time,
- at least some incidents arise from traps, blocking, traffic and retaliation rather than constant slapping,
- a player should sometimes benefit from rivals hurting each other,
- MAYHEM should feel busier than BALANCED without destroying road competence.

### Fairness
Avoid features that create the visible impression of:
- teleporting rivals,
- impossible AI acceleration,
- hazards secretly targeting the player,
- perfect AI immunity to systems that affect the player,
- the same leading AI becoming magically unbeatable.

If future catch-up logic is introduced, keep it small, bounded and difficult to distinguish from normal race variation.

### UI
- menu can be operated with keyboard alone,
- menu can be operated with controller alone,
- Pause and Finish always expose an obvious route to Main Menu,
- focus movement is predictable,
- the road remains the visual priority while racing,
- permanent debug/build vocabulary is absent from Shipping UI,
- important temporary information disappears when no longer useful.

---

## Research basis

The questions and priorities are informed by:
- Ryan, Rigby & Przybylski (2006): perceived autonomy and competence are associated with game enjoyment/preferences.
- Przybylski, Rigby & Ryan (2010): competence, autonomy and relatedness provide a useful motivational model of engagement.
- Sweetser & Wyeth (2005), GameFlow: concentration, challenge, skill, control, clear goals, feedback, immersion and social interaction are useful enjoyment criteria.
- Microsoft Xbox Accessibility Guidelines: input choice, predictable UI navigation, clear context, remapping/sensitivity and single-input operability reduce barriers.

Competitive references are used as directional evidence, not templates to copy:
- Road Redemption demonstrates enduring motorcycle-combat / Road Rash-style appeal and supports keyboard + gamepad.
- Wreckfest demonstrates the value of solid driving plus memorable collision stories; player feedback also provides a warning against AI that feels unfair or overly grief-focused.

---

## Decision rule after every playtest

Fix problems in this order:
1. loss of control / broken driving,
2. unclear or unfair outcomes,
3. confusing onboarding/UI,
4. repetitive AI/chaos,
5. weak feedback/audio,
6. visual identity/polish,
7. new content.

Do not add content to hide a weak core loop.
