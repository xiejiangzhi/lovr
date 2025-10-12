void luax_readobjarr(lua_State* L, int index, int n, float* out_arr, const char* name) {
  if (n <= 0) return;
  lovrAssert(luax_len(L, index) >= n, "length of %s table must >= %i", name, n);

  if (index < 0) index = lua_gettop(L) + index + 1;
  for (int i = 0; i < n; i++) {
    lua_rawgeti(L, index, i + 1);
    out_arr[i] = lua_tonumber(L, -1);
  }
  lua_pop(L, n);
}

void luax_writeobjarr(lua_State* L, int index, int n, float* in_arr) {
  for (int i = 0; i < n; i++) {
    lua_pushnumber(L, in_arr[i]);
    lua_rawseti(L, index, i + 1);
  }
}
