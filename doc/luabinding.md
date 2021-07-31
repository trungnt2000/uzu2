# SCRIPTING API 
****
## Table of content
1. [Entity](#1-entity)
2. [Item](#2-item)


functions are marked by '**\***' mean it not is implemented yet 
## 1 Entity
### 1.1 Constances

#### Mood
**Mood.HAPPY**
> for use with set_mood
>
> express happiness

**Mood.CONFUSED**
> for use with set_mood
>
> express confusion

**Mood.SUPRISED**
> for use with set_mood
>
> express surprise

**Mood.SLEPPY**
> for use with set_mood
>
> express boring/sleepiness

**Mood.CONFUSED**
### 1.2 Functions
**Entity.get_name(entity)**
>get name of given entity
>
>***@param entity[Entity]*** an entity
>
>***@return*** name of given entity or nil

example: 
```lua
if entity:get_name() == 'brian' then
  print('you are Brian')
end
```

**Entity.get_dialogue(entity)**
>get current dialogue id of given entity
>
>***@param entity[Entity]*** an entity
>
>***@return[string]*** dialogue id of given entity or nil

example:
```lua
--print out dialogue id if exist
local dialogue_id = entity:get_dialogue()
if dialogue_id ~= nil then
  print('current dialogue is', dialogue_id)
end
```

**Entity.set_dialogue(entity, dialogue)** [*]
>set new dialogue for given entity by id
>
>***@param entity[Entity]*** an entity
>
>***@param dialogue[string]*** dialogue id
  
example:
```lua
entity:set_dialogue('brian_dialogue_2')
```

**Entity.set_mood(entity, mood, time_out)** [*]
>set emoji for given entity
>
>
>***@param entity[Entity]*** an entity
>
>***@param mood[number]*** one of values are listed in Mood table
>
>***@param time_out*** duration in game tick
>
>***@see*** [Mood](#mood) 

Example:
```lua
--- set happy mood for 180 ticks
entity:set_mood(Mood.HAPPY, 180)
```

**Entity.set_supply(entity, supply)** [*]
> set items are going to be sold and make "sell" availiable when interact to 
>
>***@param entity[Entity]*** an entity 
>
>***@param supply[table]*** a lua table describe each availiable item
>
> ***@see*** [Item](#21-constances)  

Example:
```lua
local supply = {
  { itemtype = Item.RED_FLASK    , price = 1, availiable = 5  }
  { itemtype = Item.BLUE_FLASK   , price = 2, availiable = 5  }
  { itemtype = Item.BIG_RED_FLASK, price = 1, availiable = 25 }
}

entity:set_supply(supply)
```

**Entity.set_wellcome(entity, dialogue)** [*]
> set dialogue which is show up before trading scene with merchant 


**Entity.set_goodbye(entity, dialogue)** [*]
> set dialogue which is show up after trading scene with merchant


## 2 Item
### 2.1 Constances
**Item.RED_FLASK**
>id of "RedFlask" item type

**Item.BLUE_FLASK**
>id of "BlueFlask" item type

**Item.BIG_RED_FLASK**
>id of "BigRedFlask" item type

### 2.2 Functions
**Item.get_name(item)**
>get name of given item
>
>***@param item[Item]*** an item
>
>***@return[string]*** item name

example:
```lua
local name = item:get_name()
```

**Item.get_quality(item)**
>get quality of given item
>
>***@param item[Item]*** an item
>
>***@return[number]*** item quality

**Item.set_quality(item, quality)**
>set quality of given item
>
>***@param item[Item]*** an item
>
>***@param quality[number]*** new item quality

**Item.use(item)**
>use given item and decrease it quality by one
>
>***@param item[Item]*** an item

**Item.drop(item, n = 1)** [*]
>drop item by n
>
>***@param item[Item]*** an item
>
>***@param n[number]*** how many you want to drop


## 3 Inventory
**inventory.add_item(type_id, n = 1)**
>add new item to player inventory

**inventory.get_item(type_id)**
>get item by given type_id in player inventory
>
>***@param type_id[number]*** the item type id which you want to retrive
>
>***@return [Item]*** the reference to item or nil if player do not have given item type

## 4 Scene
**scene.find_entity(name)** [*]

>find entity by given name in current game scene
>
>***@param name*** the entity name which you want to find
>
>***@return [Entity]*** entity for given name of nil if does not exist

**scene.new_chort(x, y, weapon)** [*]

**scene.new_brian(x, y, dialogue)** [*]

**scene.get_player()** [*]



