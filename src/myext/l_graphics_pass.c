
static bool luax_checkendpoints(lua_State* L, int index, float transform[16], bool center) {
  return false;
}

// ViewCullTest(center, extend)
static int l_lovrPassViewCullTest(lua_State* L) {
  Pass* pass = luax_checktype(L, 1, Pass);
  float bound[6];
  int index = luax_readvec3(L, 2, bound, NULL);
  luax_readvec3(L, index, bound + 3, NULL);
  lua_pushboolean(L, lovrPassViewCullTest(pass, bound));
  return 1;
}