
static int l_lovrMat4SetPosition(lua_State* L) {
  mat4 m = luax_checkvector(L, 1, V_MAT4, NULL);
  float position[3];
  luax_readvec3(L, 2, position, "nil, number, vec3, or mat4");
  mat4_setPosition(m, position);
  lua_settop(L, 1);
  return 1;
}