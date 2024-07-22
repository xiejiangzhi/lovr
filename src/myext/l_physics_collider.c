
// addLinearVelocity(x, y, z, should_update_kinematic)
static int l_lovrColliderAddLinearVelocity(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float new_vel[3];
  int index = luax_readvec3(L, 2, new_vel, NULL);
  // default update dynamic body only
  bool update_kinematic = lua_toboolean(L, index);
  float cur_vel[3];
  lovrColliderGetLinearVelocity(collider, cur_vel);

  if (update_kinematic || !lovrColliderIsKinematic(collider)) {
    vec3_add(cur_vel, new_vel);
    lovrColliderSetLinearVelocity(collider, cur_vel);
    lua_pushnumber(L, cur_vel[0]);
    lua_pushnumber(L, cur_vel[1]);
    lua_pushnumber(L, cur_vel[2]);
    lua_pushboolean(L, true);
    return 4;
  } else {
    lua_pushnumber(L, cur_vel[0]);
    lua_pushnumber(L, cur_vel[1]);
    lua_pushnumber(L, cur_vel[2]);
    return 3;
  }
}

// scaleLinearVelocity(s)
static int l_lovrColliderScaleLinearVelocity(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lovrCheck(lua_isnumber(L, 2), "scale must be a number");
  float scale = lua_tonumber(L, 2);
  float cur_vel[3];
  lovrColliderGetLinearVelocity(collider, cur_vel);
  vec3_scale(cur_vel, scale);
  lovrColliderSetLinearVelocity(collider, cur_vel);
  lua_pushnumber(L, cur_vel[0]);
  lua_pushnumber(L, cur_vel[1]);
  lua_pushnumber(L, cur_vel[2]);
  return 3;
}

static int l_lovrColliderScaleAngularVelocity(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lovrCheck(lua_isnumber(L, 2), "scale must be a number");
  float scale = lua_tonumber(L, 2);
  float cur_vel[3];
  lovrColliderGetAngularVelocity(collider, cur_vel);

  vec3_scale(cur_vel, scale);
  lovrColliderSetAngularVelocity(collider, cur_vel);
  lua_pushnumber(L, cur_vel[0]);
  lua_pushnumber(L, cur_vel[1]);
  lua_pushnumber(L, cur_vel[2]);
  return 3;
}

static int l_lovrColliderScaleVelocity(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lovrCheck(lua_isnumber(L, 2), "scale must be a number");
  float scale = lua_tonumber(L, 2);

  float cur_vel[3];
  lovrColliderGetLinearVelocity(collider, cur_vel);
  vec3_scale(cur_vel, scale);
  lovrColliderSetLinearVelocity(collider, cur_vel);
  lua_pushnumber(L, cur_vel[0]);
  lua_pushnumber(L, cur_vel[1]);
  lua_pushnumber(L, cur_vel[2]);

  lovrColliderGetAngularVelocity(collider, cur_vel);
  vec3_scale(cur_vel, scale);
  lovrColliderSetAngularVelocity(collider, cur_vel);
  lua_pushnumber(L, cur_vel[0]);
  lua_pushnumber(L, cur_vel[1]);
  lua_pushnumber(L, cur_vel[2]);

  return 6;
}