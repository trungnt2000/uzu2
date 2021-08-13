#include "components.h"
#include "font_loader.h"
#include "graphics/gl.h"
#include "graphics.h"
#include "graphics/view.h"
#include "system_render.h"

static TextShader sDefaultShader;

void
system_rendering_text_init(ecs_Registry* registry)
{
  //text_renderer_init();
  /*if (text_shader_load(&sDefaultShader) != 0)
  {
    UZU_ERROR("Faild to load shader...\n");
  }*/
}

void
system_rendering_text_fini(void)
{
  //text_renderer_shutdown();
  //text_shader_destroy(&sDefaultShader);
}

void
system_rendering_text_update(ecs_Registry* registry)
{

}
