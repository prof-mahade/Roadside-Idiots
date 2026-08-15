# Next milestone — VPR-13 AI Item Parity + Stink Gate

VPR-12 dog/cow hazards are locally proven to spawn and trigger. The user requested two upgrades before moving on: poop must visibly stink, and AI riders should be smarter and have the same item abilities as the player.

## Immediate goal
Verify that bots now participate in the same comedy-item ecosystem instead of behaving like race-line puppets, while dog/cow filth has an obvious lingering stink presentation.

## VPR-13 gate
After pulling/compiling latest `dev/mvp-foundation`:
- HUD shows `BUILD: VPR-13 | AI: ITEM PARITY | FILTH: STINKY`
- existing road/traffic/race systems still function

### Poop stink
- dog poop still produces the quick skid/wobble
- cow poop still produces the strong sticky slowdown
- dirty rider now has rising green/brown fume blobs
- persistent projected label says `DOG STINK!` or `COW STINK!`
- cow fumes are more obvious/longer because cow mess lifetime is longer
- poop itself still does not directly reduce Condition

### Shared inventory architecture
Every `ARIBikePawn` owns:
- banana peel inventory
- rotten egg inventory
- shared `DropBananaPeel()` action
- shared `ThrowRottenEggAt()` action

Player and AI call the same item actions.

### AI pickup/item behavior
- banana pickups can be collected by AI and heal/grant peel
- rotten eggs can be collected by AI
- projected bot label includes inventory: `P# E#`
- AI seeks a useful nearby pickup when not actively grudging
- AI can throw a rotten egg at a suitable rider ahead
- AI can drop a peel when a suitable victim is following behind
- HOTHEAD should be the easiest bot to observe throwing eggs
- PETTY should be the easiest bot to observe dropping peels

### AI awareness
- bots attempt to avoid civilian traffic
- bots attempt to dodge dog/cow piles and dropped peels
- bots give non-target bikes some forward clearance
- HOTHEAD deliberately becomes more reckless with hazard avoidance during an active grudge

## Regression checks
- player F peel still works
- player G egg still works and HUD count comes from the player's bike inventory
- egg source attribution/grudges still work
- banana self-hit immunity still works
- traffic remains circulating
- road remains flat
- Condition tuning remains stable
- Q/E slap/grudge behavior remains intact
- checkpoint/finish/restart remain intact

## If VPR-13 passes
Stop adding new item architecture for a while. Move to:
1. free/placeholder audio layer: slap, impact, honk, skid, splat, gross hazard reactions
2. stronger crash/dizzy comedy state
3. controlled VFX/presentation polish
4. then environment/map art and final asset replacement

## Still deferred
- final models/textures
- sophisticated vehicle physics
- perfect off-track recovery
- multiplayer networking
