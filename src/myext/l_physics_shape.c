
static bool queryCallback(Shape* shape, void* userdata) {
  lua_State* L = userdata;
  lua_pushvalue(L, -1);
  luax_pushshape(L, shape);
  lua_call(L, 1, 1);
  bool shouldStop = lua_type(L, -1) == LUA_TBOOLEAN && !lua_toboolean(L, -1);
  lua_pop(L, 1);
  return shouldStop;
}

static int l_lovrShapeQueryOverlapping(lua_State* L) {
  Shape* shape = luax_checkshape(L, 1);
  lovrAssert(lovrShapeGetCollider(shape) != NULL, "Shape must be attached to collider");
  bool function = lua_type(L, 2) == LUA_TFUNCTION;
  bool any = lovrShapeQueryOverlapping(shape, function ? queryCallback : NULL, L);
  lua_pushboolean(L, any);
  return 1;
}