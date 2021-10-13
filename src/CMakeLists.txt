include(src/system/CMakeLists.txt)
include(src/ecs/CMakeLists.txt)
include(src/toolbox/CMakeLists.txt)

set(SOURCE_FILES 
  src/constances.c
  src/json_helper.c
  src/level_loader.c
  src/map.c 
  src/map_render.c
  src/read_all.c
  src/scn_main.c
  src/scn_select_character.c
  src/scn_title.c
  src/component.c
  src/engine.c
  src/input.c
  src/enitity_factory.c
  src/inventory.c
  src/resources.c
  src/message.c
  src/item.c
  src/entity.c
  src/string_utils.c


  src/ui/inventory.c
  src/ui/menu.c
  src/ui/equipment.c

  src/main.c

  #TODO GRAPHICS_SOURCE_FILES
  src/graphics/shader.c
  src/graphics/texture.c
  src/graphics/view.c
  src/graphics/animation.c
  src/graphics/font.c
  src/graphics/text_renderer.c
  src/graphics/asset_table.c
  src/graphics/sprite_sheet.c
  src/graphics/material.c
  src/graphics/vertex_buffer.c
  src/graphics/framebuffer.c
  src/graphics/light.c
  src/graphics/sprite_renderer.c
  #${BEHAVIOR_NODE_SOURCE_FILES}
  #${GAME_EVENT_SOURCE_FILES}
  ${SYSTEM_SOURCE_FILES}
  ${ECS_SOURCE_FILES}
  ${TOOLBOX_SOURCE_FILES}
)
