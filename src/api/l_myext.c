#include "api.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>


static int l_lovrMyExtTest(lua_State* L) {
  lua_pushnumber(L, 123);
  return 1;
}

static const luaL_Reg lovrMyExt[] = {
  { "test", l_lovrMyExtTest },
  { NULL, NULL }
};

int luaopen_lovr_myext(lua_State* L) {
  lua_newtable(L);
  luax_register(L, lovrMyExt);
  return 1;
}
