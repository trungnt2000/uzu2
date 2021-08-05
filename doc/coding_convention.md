**Variables** 
  - Use camelCase ex: anItem
  - Prefix s use for global static variables. Ex: sTexture
  - Prefix g use for global external variables. Ex: gTextCmd
  - If global variables have namespace use '_' to separate them. Ex: gAnim_Chort_Died
  - Struct member also use camel case.
  - Variable should have meaningful name. If name too long it's ok to use some abbreviation like cnt, siz, mat, lst, ett.

**Function**
  - Use snake_case. Ex: do_some_thing.
  - Function name should have a verb describe what is does.
  - Private function which use internally in one moudule should have static modifier
  - Function should do one thing only.

**Struct**
  - Use PascalCase
  - Struct name should be a noun describe what it represents for.
  - If struct have namespace use '_'. Ex: ecs_Pool
  - Function to create dynamic allocated struct name with __create__. Ex: Foo* foo_create(...params)
  - Function to free dynamic allocated struct name with __free__. Ex: void foo_destroy(Foo* foo)
  - Funtion to create stack allocated struct name same to struct name in snake_case. Ex: SomeStruct some_struct(...params)
  - Function to initialize struct name with __init__. Ex: SomeStruct* some_struct(SomeStruct* ss, ...prams)
  - Function to finalize struct but not free struct memory it self name with __destory__. Ex: some_struct_destroy(SomeStruct* some_struct)

**Modules**
  - moudule's initializer function name with __init__. Ex: system_renderer_sprite_init()
  - moudule's finalizer function name with __shutdown__. Ex: system_renderer_sprite_shutdown()
***
- Prefer using static, fixed size array instead of dyanamic allocated.
- Prefer writing code moudular over OOP-like way but if you have to it's ok.
- Prefer using array avoid linked list.
***
