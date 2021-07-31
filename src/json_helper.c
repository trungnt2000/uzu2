#include "json_helper.h"
#include "read_all.h"
json_object* json_from_file(const char* filename)
{
  FILE*        file;
  char*        filedata = NULL;
  size_t       len;
  json_object* jObj = NULL;

  if ((file = fopen(filename, "r")) != NULL)
  {
    if (readall(file, &filedata, &len) == READALL_OK)
    {
      jObj = json_tokener_parse(filedata);
      free(filedata);
    }
    fclose(file);
  }
  return jObj;
}
