# Next milestone — Combat Slice 1

The second playtest confirmed the driving prototype is usable enough to move forward, although AI can still occasionally get confused after leaving the intended road. Perfect recovery/pathing is deferred to a later vehicle/AI mechanics pass.

## Goal
Make the existing Q/E side interaction produce the first recognizable Roadside Idiots gameplay moment.

## This pass
- keep Q/E as left/right prototype interactions
- preserve physical wobble and condition loss
- when a bot is provoked, temporarily prioritize the provoking rider instead of simply following the racing line
- allow the bot to attempt the same side interaction when it catches the provoking rider
- after a short rivalry window, return the bot to normal racing behavior
- keep the safe reset fallback for badly stuck bots

## Not part of this pass
- final slap/kick animation
- final sounds
- character ragdoll
- sophisticated personality/grudge memory
- perfect off-track pathfinding

## Test gate
The pass is successful if provoking a nearby bot can cause a short, readable chase/response without preventing the race from continuing afterward.
