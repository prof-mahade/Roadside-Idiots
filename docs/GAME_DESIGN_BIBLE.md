# Roadside Idiots — Game Design Bible

## Core fantasy

**You are a competent motorcycle rider trying to win a race while surrounded by people who behave like complete idiots.**

The game must never invert that fantasy. The player should not feel that the controls, camera, AI pathing, or random systems are the real idiots.

Tagline: **The road is dangerous. The riders are worse.**

## Design promise

Roadside Idiots is an arcade motorcycle racing/combat game about:

1. satisfying high-speed riding,
2. readable petty combat,
3. believable traffic pressure,
4. emergent comedy,
5. short races that create stories worth replaying.

The game is **drive-first, chaos-second**. Chaos interrupts skill; it must not replace skill.

---

## Who this game is for

### Primary audience — arcade racing + chaos players
Players who want immediate controls, short sessions, overtaking, collisions, items, and funny race stories without learning a simulation.

### Secondary audience — Road Rash / combat-racing nostalgia
Players attracted by motorcycle racing plus physical interference, but who do not necessarily want a dark post-apocalyptic tone.

### Secondary audience — funny-physics / emergent-chaos players
Players who enjoy Wreckfest-like unexpected incidents, near misses, recovery moments, and sharing clips of ridiculous outcomes.

### Regional identity audience
Players who enjoy seeing recognizable South-Asian/Bangladesh-inspired roadside details represented with affection and humor rather than as a generic racetrack skin.

### Future social audience
Local/online friends who will eventually enjoy blaming each other, retaliating, making temporary alliances, and creating stories. Multiplayer is deferred, but systems should preserve this future direction.

---

## Psychology translated into design

### 1. Competence — "I am getting good at this"
The rider controls must be predictable. A good line, smart dodge, good slap timing, or clever item use should visibly pay off.

Rules:
- stable bike handling is more important than spectacle,
- racing AI must be competent enough to validate the player's skill,
- avoid invisible failures and unexplained damage,
- recovery should be quick enough that one mistake does not destroy the whole race,
- items need clear anticipation, hit feedback, and readable consequences.

### 2. Autonomy — "I chose how I want this race to feel"
Give meaningful choices without creating setup homework.

Near-term race choices:
- opponents,
- laps,
- traffic,
- **chaos intensity: CLEAN / BALANCED / MAYHEM**.

Longer-term choices:
- bike/rider style,
- item preference/loadout direction,
- race mode,
- route/environment variants.

### 3. Challenge/flow — "hard enough to care, fair enough to retry"
The strongest target experience is a race that creates pressure without obvious cheating.

Rules:
- preserve a fair-feeling base racing speed,
- prefer better decisions and situational pressure over blatant rubber-banding,
- if catch-up assistance is ever added, make it subtle and bounded,
- difficulty should create more interesting decisions, not just faster AI,
- the player should usually believe a loss was recoverable.

### 4. Feedback — "I instantly understand what just happened"
Every important action should answer three questions:
1. What happened?
2. Who caused it?
3. What can I do now?

Feedback stack:
- animation/pose,
- sound,
- short comic text,
- condition/item HUD,
- minimap/rival warning where useful.

Avoid permanent debug-like labels and excessive screen text.

### 5. Rival identity — "I know these idiots"
The rivals should feel like personalities, not BOT_01 through BOT_06.

Current archetypes:
- **LEECH** — blocks and hangs around useful lines,
- **HOTHEAD** — impulsive contact and retaliation,
- **PETTY** — weapon-oriented harassment,
- **GREMLIN** — traps and opportunism,
- **BRAWLER** — physical side pressure,
- **TRYHARD** — mostly races seriously.

Design rule: personalities should be recognizable from behavior before the player reads a label.

### 6. Tension and release
Constant chaos becomes noise. A memorable race needs quiet enough moments for the player to read the road and anticipate the next incident.

Target rhythm:

`race cleanly -> notice threat/opportunity -> incident -> recovery/overtake -> breathing room -> escalation -> finish pressure`

Rules:
- no deliberate chaos immediately off the start,
- limit simultaneous troublemakers,
- tactical actions are short,
- cooldown after an incident,
- late-race pressure may increase, but not by breaking driving competence.

### 7. Humour through consequences, not randomness alone
The funniest moments should usually have a cause the player can understand:
- somebody tried to block and got punished,
- a peel intended for one rider hit another,
- an angry rival forgot the racing line,
- traffic created a ridiculous chain reaction,
- the player survived something that looked impossible.

Pure arbitrary randomness is less satisfying because it weakens competence and agency.

---

## Core moment-to-moment loop

1. Read road + traffic.
2. Choose racing line.
3. Read nearby rival personality/threat.
4. Decide: race, dodge, attack, trap, or save item.
5. Commit.
6. Receive immediate feedback.
7. Recover position/condition.
8. Repeat under increasing race pressure.

The player should be making a meaningful micro-decision every few seconds without feeling constantly attacked.

---

## Race pacing targets

These are tuning targets, not hard laws.

### Opening — establish competence
First ~5–8 seconds after GO:
- racing first,
- avoid director-created chaos,
- let the player understand speed, traffic density, and initial rivals.

### Mid-race — stories emerge
- controlled tactical incidents,
- item opportunities,
- overtakes,
- traffic interactions,
- personality expression.

### Final phase — pressure
- close positional information becomes more prominent,
- rivals may take slightly more tactical risk,
- avoid fake last-second speed cheating,
- winning/losing should remain understandable.

---

## Fairness rules

Never intentionally:
- teleport a rival in front of the player,
- give an AI impossible steering authority solely to force a result,
- make hazards target the player with hidden knowledge,
- make the same player lose repeatedly to unavoidable randomness,
- allow combat logic to overwrite the stable racing-line follower.

Allowed game-AI cheats when subtle and bounded:
- AI-only stability assistance,
- slightly simplified perception,
- limited item assistance,
- recovery help,
- mild difficulty/catch-up tuning that does not visibly violate the race.

---

## UI/HUD principles

### While racing
Priority order:
1. road and rivals,
2. place/lap,
3. speed/condition/items,
4. minimap,
5. temporary threat/comic feedback,
6. controls only when useful.

Do not display internal VPR/build terminology in player-facing Shipping UI.

### Menus
Keep setup short. The player should be able to launch a race in seconds.

Default setup should represent the intended game:
- medium field,
- medium race length,
- some traffic,
- BALANCED chaos.

### Finish
The finish screen should celebrate outcome and immediately invite another race. It should feel playful, not like a debug report.

---

## Audio principles

Audio is gameplay feedback, not decoration.

Highest priority:
- engine/load feel,
- impacts,
- item throw/hit/miss,
- skid/slip,
- countdown/GO,
- lap/finish,
- traffic horn,
- short character/comic accents where affordable/free/custom.

Avoid constant loud comedy sounds. Important events need acoustic space.

---

## Environment identity

The long-term visual identity should be a recognizable South-Asian/Bangladesh-inspired roadside world rather than a generic closed racing circuit.

Useful motifs:
- tea stalls / roadside shops,
- buses, CNG/auto-rickshaw-inspired traffic, vans, small trucks,
- concrete walls, tin roofs, utility poles and tangled roadside infrastructure,
- banana/tropical vegetation,
- signs, market clutter, construction, livestock hazards,
- dense village/town transitions and occasional open stretches.

All content remains **free/custom only** under the permanent project constraint.

Humor should come from believable roadside situations exaggerated into racing problems, not from mocking real people or poverty.

---

## Progression direction after the polished demo

Avoid grind-first design. Progression should create new play styles and stories.

Good future unlocks:
- cosmetic rider/bike identity,
- side-grade handling profiles,
- item capacity/tradeoffs,
- new race routes/modes,
- rival modifiers,
- challenge objectives.

Avoid early pay-to-win or stat inflation logic. The prototype remains free-only and offline-first.

---

## Metrics worth logging later

For actual tuning, eventually log:
- race completion rate,
- average finishing position,
- wall/traffic collisions per minute,
- item pickup/use/hit rates,
- time spent near rivals,
- recovery uses,
- overtakes,
- time gap at finish,
- quit/restart points,
- chaos events per minute.

Do not optimize one metric alone. A fun race may contain crashes; the goal is understandable, recoverable chaos.

---

## Competitive references — what to learn, not copy

### Road Redemption
Useful lesson: motorcycle combat can be the main differentiator, but Roadside Idiots should stay lighter, more roadside-comedy focused, and less weapon-heavy.

### Wreckfest
Useful lesson: players enjoy breakneck racing partly because collisions create one-off stories. Roadside Idiots should create similar memorable incidents while keeping motorcycles controllable and recovery fast.

### Mario Kart-style arcade design
Useful lesson: accessible control, readable items, strong feedback, and comeback opportunities make chaotic racing approachable. Roadside Idiots should not become an item lottery; driving competence remains the foundation.

---

## Research basis

The design direction above is informed by, not mechanically dictated by:

- Ryan, Rigby & Przybylski (2006), *The Motivational Pull of Video Games: A Self-Determination Theory Approach*, Motivation and Emotion, DOI: 10.1007/s11031-006-9051-8.
- Przybylski, Rigby & Ryan (2010), *A Motivational Model of Video Game Engagement*, Review of General Psychology, DOI: 10.1037/a0019440.
- Sweetser & Wyeth (2005), *GameFlow: A Model for Evaluating Player Enjoyment in Games*, Computers in Entertainment, DOI: 10.1145/1077246.1077253.
- Sukhil & Behl (2021), *Adaptive Lookahead Pure-Pursuit for Autonomous Racing*, arXiv:2111.08873 — relevant to the stable racing controller, not player psychology.

## Permanent decision filter

Before adding a feature, ask:

1. Does it make riding more satisfying?
2. Does it create a readable story or decision?
3. Does it strengthen the game's identity?
4. Does it preserve player agency/fairness?
5. Can it be built with free/custom content?
6. Does it avoid destabilizing the accepted racing controller?

If the answer is mostly no, it is not a priority.
