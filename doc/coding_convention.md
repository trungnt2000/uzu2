**Variables** 
  - Use camelCase ex: anItem
  - Prefix s use for global static variables. Ex: sTexture
  - Prefix g use for global external variables. Ex: gTextCmd
  - If global variables have namespace use '_'. Ex: gAnim_Chort_Died
  - Struct member also use camel case.
  - Variable should have meaningful name. If name to long it's ok to use some abbreviation like cnt, siz, mat, lst, ett

**Function**
  - Use snake_case. Ex: do_some_thing.
  - Function name should have a verb describe what is does.
  - Private function which use internally in one moudule should have static modifier
  - Function should do one thing only.
**Struct**
  - Use PascalCase
  - Struct name should be a noun describe what it represents for.
  - If struct have namespace use '_'. Ex: ecs_Pool

****
- Prefer using static, fixed size array instead of dyanamic allocated.
- Prefer writing code moudular over OOP-like way but if you have to it's ok.
- Prefer using array avoid linked list.
