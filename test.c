#include <stddefer:q>
void func()
{
  guard {
    int *s = malloc(6); 
    defer free(s);
  }
}
