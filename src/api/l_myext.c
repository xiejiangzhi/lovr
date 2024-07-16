#include "api.h"
#include "util.h"
#include "physics/physics.h"
#include "core/maf.h"

#include <stdlib.h>
#include <string.h>


static int l_lovrMyExtTest(lua_State* L) {
  lua_pushnumber(L, 123);
  return 1;
}

// write to e.phy_pos, e.phy_rot, e.phy_tpos, e.phy_lv, e.phy_av
static int l_lovrSyncEntitiesPhyData(lua_State* L) {
  lovrCheck(lua_istable(L, 1), "Argument 1 must give a entities list");

  int len = luax_len(L, 1);
  for (int i = 1; i <= len; i ++) {
    lua_rawgeti(L, 1, i);
    int eidx = lua_gettop(L);
    lua_getfield(L, eidx, "collider");
    Collider* collider = luax_totype(L, -1, Collider);
    if (collider) {
      float pos[3], rot[4];
      lovrColliderGetPose(collider, pos, rot);
      lua_getfield(L, eidx, "phy_pos");
      vec3 v = luax_checkvector(L, -1, V_VEC3, NULL);
      vec3_set(v, pos[0], pos[1], pos[2]);

      lua_getfield(L, eidx, "phy_tpos");
      lua_pushnumber(L, pos[0]);
      lua_rawseti(L, -2, 1);
      lua_pushnumber(L, pos[1]);
      lua_rawseti(L, -2, 2);
      lua_pushnumber(L, pos[2]);
      lua_rawseti(L, -2, 3);

      lua_getfield(L, eidx, "phy_rot");
      quat q = luax_checkvector(L, -1, V_QUAT, NULL);
      quat_set(q, rot[0], rot[1], rot[2], rot[3]);
      lua_pop(L, 5);
    } else {
      lua_pop(L, 2);
    }

  }

  return 1;
}

static const luaL_Reg lovrMyExt[] = {
  { "test", l_lovrMyExtTest },
  { "syncEntitiesPhyData", l_lovrSyncEntitiesPhyData },
  { NULL, NULL }
};

int luaopen_lovr_myext(lua_State* L) {
  lua_newtable(L);
  luax_register(L, lovrMyExt);
  return 1;
}
