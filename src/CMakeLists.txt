include(src/system/CMakeLists.txt)
include(src/ecs/CMakeLists.txt)
include(src/toolbox/CMakeLists.txt)

set(SOURCE_FILES 
  src/constances.c
  src/json_helper.c
  src/level_loader.c
  src/font_loader.c
  src/map.c 
  src/map_render.c
  src/read_all.c
  src/scn_main.c
  src/scn_select_character.c
  src/scn_title.c
  src/component.c
  src/engine.c
  src/input.c

  src/main.c

  src/graphics/shader.c
  src/graphics/render.c
  src/graphics/texture.c
  src/graphics/view.c
  src/graphics/anim.c
  #${BEHAVIOR_NODE_SOURCE_FILES}
  #${GAME_EVENT_SOURCE_FILES}
  ${SYSTEM_SOURCE_FILES}
  ${ECS_SOURCE_FILES}
  ${TOOLBOX_SOURCE_FILES}
)
