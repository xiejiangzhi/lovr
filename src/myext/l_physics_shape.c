
// static bool queryCallback(Shape* shape, void* userdata) {
//   lua_State* L = userdata;
//   lua_pushvalue(L, -1);
//   luax_pushshape(L, shape);
//   lua_call(L, 1, 1);
//   bool shouldStop = lua_type(L, -1) == LUA_TBOOLEAN && !lua_toboolean(L, -1);
//   lua_pop(L, 1);
//   return shouldStop;
// }

// static int l_lovrShapeQueryOverlapping(lua_State* L) {
//   Shape* shape = luax_checkshape(L, 1);
//   lovrAssert(lovrShapeGetCollider(shape) != NULL, "Shape must be attached to collider");
//   bool function = lua_type(L, 2) == LUA_TFUNCTION;
//   bool any = lovrShapeQueryOverlapping(shape, function ? queryCallback : NULL, L);
//   lua_pushboolean(L, any);
//   return 1;
// }

// static int l_lovrShapeQueryOverlaps(lua_State* L) {
//   Shape* shape = luax_checkshape(L, 1);
//   Collider* collider = lovrShapeGetCollider(shape);
//   lovrAssert(collider != NULL, "Shape must be attached to collider");
//   float pose[7];
//   lovrShapeGetPose(shape, pose, pose + 3);
//   uint32_t filter = ~0u; // TODO gen filter by collider->tag
//   World* world = lovrColliderGetWorld(collider);
//   if (lua_isnoneornil(L, 2)) {
//     OverlapResult hit;
//     if (lovrWorldOverlapShape(world, shape, pose, filter, overlapFirstCallback, &hit)) {
//       return luax_pushoverlapresult(L, &hit);
//     }
//   } else {
//     luaL_checktype(L, 2, LUA_TFUNCTION);
//     lua_settop(L, 2);
//     lovrWorldOverlapShape(world, shape, pose, filter, overlapCallback, L);
//   }
//   return 0;
// }

static int l_lovrShapeCollidePoint(lua_State* L) {
  Shape* shape = luax_checkshape(L, 1);
  float pos[3];
  luax_readvec3(L, 2, pos, NULL);
  bool r = lovrShapeCollidePoint(shape, pos);
  lua_pushboolean(L, r);
  return 1;
}