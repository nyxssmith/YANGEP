# Spacegame MVP To-Do List

## MVP Features

2 tracks
Each has 1 level, get reward, 2nd level, end

Track A
Starts:
Grapple hook + sword
Gets: shotgun

Track B
Starts:
Laser gun and shield
Gets: grenade

Level Format:

Series of rooms, boss is at end
Tile has pre-defined spawn points for enemies
Items/Actions:

Sword:
Does N damage in either a 2 in front or 3 wide 1 in front pattern (provides 2 actions)
1s cooldown per action

Laser gun:
Shoots line 5 long, 1 wide in front
3s cooldown

Grapple hook:
Line 7 in front, if hits enemy, pulls to space in front and stuns for 1s
If hits wall, pulls player to it
5s cooldown

Shield: Adds shield, blocks N damage or 10s, 25s cooldown

Grenade: 5 tiles in front, does diamond pattern of 3 diameter “+” as base
10s cooldown

Shotgun:
Shoots a cone in front of player 3 long, 1, 3, 5 wide
5s cooldown

Enemy types:

Sword grunt
Has sword
Gun Grunt
Has laser gun
Tank
Has shotgun and shield actions

## Engine Features

Features needed to be added to the engine to support the MVP.
Each block of checkboxes is dependency ordered, but blocks can be done in parallel.

#### World and Level Management (Nyx)

- [ ] Multiple Levels combined into World
- [ ] Engine can re-load a world at any time (for level transitions / restarts)
- [ ] Main menu "world" that allows player to select which track to play

#### Item, Action, Health System (Nyx)

- [ ] Items Provide Actions
- [ ] UI for showing items on left side of screen
- [ ] UI for showing cooldowns on actions on bottom of screen
- [ ] Characters have health and shield values + on_damage functions etc
- [ ] UI for showing health and shield on top right of screen

#### Enemy Behavior

- [x] Enemies can be spawned at specific locations in the level
- [ ] Enemies have simple behavior patterns (e.g. move towards player, attack when in range)
- [ ] Enemies can use their actions (e.g. shoot, attack) (coordinator handles this part)

#### Level Design Tools

- [ ] Ability to place spawn points for enemies in the level editor
- [ ] Ability to pre-set a level with specific items for the player to start with

#### Animation and Art Pipeline (Aria)

- [ ] Scripts to adapt unity assets to yangep sprite animation format
- [ ] New character animation format that works with the new unity assets
- [ ] Character customization system that allows swapping out different body parts and colors (for player character)
