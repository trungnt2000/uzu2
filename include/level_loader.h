#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H
enum
{
  LOAD_LEVEL_OK,
  LOAD_LEVEL_FAIL_TO_LOAD_FILE,
  LOAD_LEVEL_FAIL_TO_PARSE,
};
int load_level(const char* level_name);
#endif // LEVEL_LOADER_H
