# Level formats what they are

## Level V1

The first version of the level format used in YANGEP. It supports basic game logic, entities, and level structure.
Loads entities from `entities.json`
Does not support saving or world space compositing

## Level V2

An extended version of Level V1
that is used by World and supports save and load of its entity file.
Loads entities from `entities.json` unless entity states are present in `entity_states.json`

## World

Has a single source of data for all current states of entities in each level
Supports loading and saving of entity states to levels
Composites levels in world space for seamless transitions between levels
