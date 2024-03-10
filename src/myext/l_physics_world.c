
static int l_lovrWorldQueryTriangle(lua_State* L) {
  World* world = luax_checktype(L, 1, World);
  float vertices[9];
  int index = luax_readvec3(L, 2, vertices, NULL);
  index = luax_readvec3(L, index, vertices + 3, NULL);
  index = luax_readvec3(L, index, vertices + 6, NULL);
  const char* tag = NULL;
  if ((lua_gettop(L) - index) > 0) {
    tag = lua_tostring(L, index++);
  }
  bool function = lua_type(L, index) == LUA_TFUNCTION;
  lua_settop(L, index);
  bool any = lovrWorldQueryTriangle(world, vertices, tag, function ? queryCallback : NULL, L);
  lua_pushboolean(L, any);
  return 1;
}

/*
world:queryShape(shape, pos, rot, cb)
world:queryShape(shape, pos, rot, tag, cb)
*/
static int l_lovrWorldQueryShape(lua_State* L) {
  World* world = luax_checktype(L, 1, World);
  Shape* shape = luax_checkshape(L, 2);
  float position[3], orientation[4];
  int index = luax_readvec3(L, 3, position, NULL);
  index = luax_readquat(L, index, orientation, NULL);
  const char* tag = NULL;
  if ((lua_gettop(L) - index) > 0) {
    tag = lua_tostring(L, index++);
  }
  bool function = lua_type(L, index) == LUA_TFUNCTION;
  lua_settop(L, index);
  bool any = lovrWorldQueryShape(world, shape, position, orientation, tag, function ? queryCallback : NULL, L);
  lua_pushboolean(L, any);
  return 1;
}