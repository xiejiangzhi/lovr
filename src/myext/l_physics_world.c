
static int l_lovrWorldQueryTriangle(lua_State* L) {
  World* world = luax_checktype(L, 1, World);
  float vertices[9];
  int index = luax_readvec3(L, 2, vertices, NULL);
  index = luax_readvec3(L, index, vertices + 3, NULL);
  index = luax_readvec3(L, index, vertices + 6, NULL);
  uint32_t filter = luax_checktagmask(L, index++, world);

  int ret_args = 0;
  if (lua_isnoneornil(L, index)) {
    OverlapResult hit;
    if (lovrWorldQueryTriangle(world, vertices, filter, overlapFirstCallback, &hit)) {
      ret_args = luax_pushoverlapresult(L, &hit);
    }
  } else {
    luaL_checktype(L, index, LUA_TFUNCTION);
    lua_settop(L, index);
    lovrWorldQueryTriangle(world, vertices, filter, overlapCallback, L);
  }
  return ret_args;
}

static int l_lovrWorldNewPlaneCollider(lua_State* L) {
  World* world = luax_checkworld(L, 1);
  PlaneShape* shape = luax_newplaneshape(L, 2);
  float position[3] = { 0.f, 0.f, 0.f };
  Collider* collider = lovrColliderCreate(world, position, shape);
  lovrRelease(shape, lovrShapeDestroy);
  luax_assert(L, collider);
  luax_pushtype(L, Collider, collider);
  lovrRelease(collider, lovrColliderDestroy);
  return 1;
}
