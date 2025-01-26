
static int l_lovrPhysicsNewPlaneShape(lua_State* L) {
  PlaneShape* plane = luax_newplaneshape(L, 1);
  luax_pushtype(L, PlaneShape, plane);
  lovrRelease(plane, lovrPlaneShapeDestroy);
  return 1;
}
