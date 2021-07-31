#ifndef MAIN_H
#define MAIN_H
/* scene */
typedef enum
{
  SCENE_UNDEFINED = -1,
  SCENE_TITLE,
  SCENE_MAIN,
  SELECT_CHARACTER,
  SCENE_CNT
} SceneId;

void set_scene(SceneId scnId);
#endif // MAIN_H
