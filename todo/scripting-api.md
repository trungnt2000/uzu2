# SCRIPTING API CHECKLIST

## Global
- [] load_dialogue
- [] load_action_script

local dialogue1 = load_dialogue('res/scripts/dialogue/brian1.json')

function \_on_level_load(msg)
  brian:set_dialogue(dialogue1)
end

## Event reciving functions
- [] conversation_finish
- [x] entity_died
- [x] level_load
- [x] level_unload
- [x] scene_load
- [x] scene_unload

## Entity
- [x] get_name
- [x] is_monster
- [x] get_position
- [x] get_dialogue
- [x] set_dialogue
- [] play_acrion_script
- [] set_mood
- [] die
- [] set_supply(entity, supply) -- set what are going to sell by this entity
- [] set_wellcome -- display before dialog showing up
- [] set_leave -- display affter dialog closing


## Scene
- [] find_entity
- [] get_player
- [] new_chort
- [] new_brian
- [] new_huge_demon
- [] new_imp
- [] new_wogol

## Inventory
- [x] get_item
- [x] add_item

## Item
- [x] get_quality
- [x] set_quality
- [x] drop
- [x] use
