#include "api.h"
#include "physics/physics.h"
#include "core/maf.h"
#include "util.h"
#include <stdbool.h>

static int l_lovrColliderDestroy(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lovrColliderDestroyData(collider);
  return 0;
}

static int l_lovrColliderIsDestroyed(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool destroyed = lovrColliderIsDestroyed(collider);
  lua_pushboolean(L, destroyed);
  return 1;
}

static int l_lovrColliderIsEnabled(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool enabled = lovrColliderIsEnabled(collider);
  lua_pushboolean(L, enabled);
  return 1;
}

static int l_lovrColliderSetEnabled(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool enable = lua_toboolean(L, 2);
  lovrColliderSetEnabled(collider, enable);
  return 1;
}

static int l_lovrColliderGetWorld(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  World* world = lovrColliderGetWorld(collider);
  luax_pushtype(L, World, world);
  return 1;
}

static int l_lovrColliderGetShape(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  Shape* shape = lovrColliderGetShape(collider);
  if (shape) {
    luax_pushshape(L, shape);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

static int l_lovrColliderSetShape(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  Shape* shape = lua_isnoneornil(L, 2) ? NULL : luax_checkshape(L, 2);
  lovrColliderSetShape(collider, shape);
  return 0;
}

static int l_lovrColliderGetJoints(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_newtable(L);
  int index = 1;
  Joint* joint = NULL;
  while ((joint = lovrColliderGetJoints(collider, joint)) != NULL) {
    luax_pushjoint(L, joint);
    lua_rawseti(L, -2, index++);
  }
  return 1;
}

static int l_lovrColliderGetUserData(lua_State* L) {
  luax_checktype(L, 1, Collider);
  luax_pushstash(L, "lovr.collider.userdata");
  lua_pushvalue(L, 1);
  lua_rawget(L, -2);
  return 1;
}

static int l_lovrColliderSetUserData(lua_State* L) {
  luax_checktype(L, 1, Collider);
  lua_settop(L, 2);
  luax_pushstash(L, "lovr.collider.userdata");
  lua_pushvalue(L, 1);
  lua_pushvalue(L, 2);
  lua_rawset(L, -3);
  return 0;
}

static int l_lovrColliderIsKinematic(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushboolean(L, lovrColliderIsKinematic(collider));
  return 1;
}

static int l_lovrColliderSetKinematic(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool kinematic = lua_toboolean(L, 2);
  lovrColliderSetKinematic(collider, kinematic);
  return 0;
}

static int l_lovrColliderIsSensor(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushboolean(L, lovrColliderIsSensor(collider));
  return 1;
}

static int l_lovrColliderSetSensor(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool sensor = lua_toboolean(L, 2);
  lovrColliderSetSensor(collider, sensor);
  return 0;
}

static int l_lovrColliderIsContinuous(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool continuous = lovrColliderIsContinuous(collider);
  lua_pushboolean(L, continuous);
  return 1;
}

static int l_lovrColliderSetContinuous(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool continuous = lua_toboolean(L, 2);
  lovrColliderSetContinuous(collider, continuous);
  return 0;
}

static int l_lovrColliderGetGravityScale(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float scale = lovrColliderGetGravityScale(collider);
  lua_pushnumber(L, scale);
  return 1;
}

static int l_lovrColliderSetGravityScale(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float scale = luax_checkfloat(L, 2);
  lovrColliderSetGravityScale(collider, scale);
  return 0;
}

static int l_lovrColliderIsAwake(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushboolean(L, lovrColliderIsAwake(collider));
  return 1;
}

static int l_lovrColliderSetAwake(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool awake = lua_toboolean(L, 2);
  lovrColliderSetAwake(collider, awake);
  return 0;
}

static int l_lovrColliderIsSleepingAllowed(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushboolean(L, lovrColliderIsSleepingAllowed(collider));
  return 1;
}

static int l_lovrColliderSetSleepingAllowed(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool allowed = lua_toboolean(L, 2);
  lovrColliderSetSleepingAllowed(collider, allowed);
  return 0;
}

static int l_lovrColliderGetMass(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushnumber(L, lovrColliderGetMass(collider));
  return 1;
}

static int l_lovrColliderSetMass(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  if (lua_isnoneornil(L, 2)) {
    lovrColliderSetMass(collider, NULL);
  } else {
    float mass = luax_checkfloat(L, 2);
    lovrColliderSetMass(collider, &mass);
  }
  return 0;
}

static int l_lovrColliderGetInertia(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float diagonal[3], rotation[4];
  lovrColliderGetInertia(collider, diagonal, rotation);
  lua_pushnumber(L, diagonal[0]);
  lua_pushnumber(L, diagonal[1]);
  lua_pushnumber(L, diagonal[2]);
  lua_pushnumber(L, rotation[0]);
  lua_pushnumber(L, rotation[1]);
  lua_pushnumber(L, rotation[2]);
  lua_pushnumber(L, rotation[3]);
  return 7;
}

static int l_lovrColliderSetInertia(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  if (lua_isnoneornil(L, 2)) {
    lovrColliderSetInertia(collider, NULL, NULL);
  } else {
    float diagonal[3], rotation[4];
    int index = luax_readvec3(L, 2, diagonal, NULL);
    luax_readquat(L, index, rotation, NULL);
    lovrColliderSetInertia(collider, diagonal, rotation);
  }
  return 0;
}

static int l_lovrColliderGetCenterOfMass(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float center[3];
  lovrColliderGetCenterOfMass(collider, center);
  lua_pushnumber(L, center[0]);
  lua_pushnumber(L, center[1]);
  lua_pushnumber(L, center[2]);
  return 3;
}

static int l_lovrColliderSetCenterOfMass(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  if (lua_isnoneornil(L, 2)) {
    lovrColliderSetCenterOfMass(collider, NULL);
  } else {
    float center[3];
    luax_readvec3(L, 2, center, NULL);
    lovrColliderSetCenterOfMass(collider, center);
  }
  return 0;
}

static int l_lovrColliderResetMassData(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lovrColliderResetMassData(collider);
  return 0;
}

static int l_lovrColliderGetEnabledAxes(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool translation[3];
  bool rotation[3];
  lovrColliderGetEnabledAxes(collider, translation, rotation);

  char string[3];
  size_t length;

  length = 0;
  for (size_t i = 0; i < 3; i++) {
    if (translation[i]) {
      string[length++] = 'x' + i;
    }
  }
  lua_pushlstring(L, string, length);

  length = 0;
  for (size_t i = 0; i < 3; i++) {
    if (rotation[i]) {
      string[length++] = 'x' + i;
    }
  }
  lua_pushlstring(L, string, length);

  return 2;
}

static int l_lovrColliderSetEnabledAxes(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool translation[3] = { false, false, false };
  bool rotation[3] = { false, false, false };
  const char* string;
  size_t length;

  string = lua_tolstring(L, 2, &length);
  for (size_t i = 0; i < length; i++) {
    if (string[i] >= 'x' && string[i] <= 'z') {
      translation[string[i] - 'x'] = true;
    }
  }

  string = lua_tolstring(L, 3, &length);
  for (size_t i = 0; i < length; i++) {
    if (string[i] >= 'x' && string[i] <= 'z') {
      rotation[string[i] - 'x'] = true;
    }
  }

  lovrColliderSetEnabledAxes(collider, translation, rotation);
  return 0;
}

static int l_lovrColliderGetPosition(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float position[3];
  lovrColliderGetPosition(collider, position);
  lua_pushnumber(L, position[0]);
  lua_pushnumber(L, position[1]);
  lua_pushnumber(L, position[2]);
  return 3;
}

static int l_lovrColliderSetPosition(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float position[3];
  luax_readvec3(L, 2, position, NULL);
  lovrColliderSetPosition(collider, position);
  return 0;
}

static int l_lovrColliderGetOrientation(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float orientation[4], angle, x, y, z;
  lovrColliderGetOrientation(collider, orientation);
  quat_getAngleAxis(orientation, &angle, &x, &y, &z);
  lua_pushnumber(L, angle);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  lua_pushnumber(L, z);
  return 4;
}

static int l_lovrColliderSetOrientation(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float orientation[4];
  luax_readquat(L, 2, orientation, NULL);
  lovrColliderSetOrientation(collider, orientation);
  return 0;
}

static int l_lovrColliderGetPose(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float position[3], orientation[4], angle, ax, ay, az;
  lovrColliderGetPosition(collider, position);
  lovrColliderGetOrientation(collider, orientation);
  quat_getAngleAxis(orientation, &angle, &ax, &ay, &az);
  lua_pushnumber(L, position[0]);
  lua_pushnumber(L, position[1]);
  lua_pushnumber(L, position[2]);
  lua_pushnumber(L, angle);
  lua_pushnumber(L, ax);
  lua_pushnumber(L, ay);
  lua_pushnumber(L, az);
  return 7;
}

static int l_lovrColliderSetPose(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float position[3], orientation[4];
  int index = luax_readvec3(L, 2, position, NULL);
  luax_readquat(L, index, orientation, NULL);
  lovrColliderSetPosition(collider, position);
  lovrColliderSetOrientation(collider, orientation);
  return 0;
}

static int l_lovrColliderGetRawPosition(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float position[3];
  lovrColliderGetRawPosition(collider, position);
  lua_pushnumber(L, position[0]);
  lua_pushnumber(L, position[1]);
  lua_pushnumber(L, position[2]);
  return 3;
}

static int l_lovrColliderGetRawOrientation(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float orientation[4], angle, x, y, z;
  lovrColliderGetRawOrientation(collider, orientation);
  quat_getAngleAxis(orientation, &angle, &x, &y, &z);
  lua_pushnumber(L, angle);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  lua_pushnumber(L, z);
  return 4;
}

static int l_lovrColliderGetLinearVelocity(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float velocity[3];
  lovrColliderGetLinearVelocity(collider, velocity);
  lua_pushnumber(L, velocity[0]);
  lua_pushnumber(L, velocity[1]);
  lua_pushnumber(L, velocity[2]);
  return 3;
}

static int l_lovrColliderSetLinearVelocity(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float velocity[3];
  luax_readvec3(L, 2, velocity, NULL);
  lovrColliderSetLinearVelocity(collider, velocity);
  return 0;
}

static int l_lovrColliderGetAngularVelocity(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float velocity[3];
  lovrColliderGetAngularVelocity(collider, velocity);
  lua_pushnumber(L, velocity[0]);
  lua_pushnumber(L, velocity[1]);
  lua_pushnumber(L, velocity[2]);
  return 3;
}

static int l_lovrColliderSetAngularVelocity(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float velocity[3];
  luax_readvec3(L, 2, velocity, NULL);
  lovrColliderSetAngularVelocity(collider, velocity);
  return 0;
}

static int l_lovrColliderGetLinearDamping(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float damping, threshold;
  lovrColliderGetLinearDamping(collider, &damping, &threshold);
  lua_pushnumber(L, damping);
  lua_pushnumber(L, threshold);
  return 2;
}

static int l_lovrColliderSetLinearDamping(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float damping = luax_checkfloat(L, 2);
  float threshold = luax_optfloat(L, 3, 0.0f);
  lovrColliderSetLinearDamping(collider, damping, threshold);
  return 0;
}

static int l_lovrColliderGetAngularDamping(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float damping, threshold;
  lovrColliderGetAngularDamping(collider, &damping, &threshold);
  lua_pushnumber(L, damping);
  lua_pushnumber(L, threshold);
  return 2;
}

static int l_lovrColliderSetAngularDamping(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float damping = luax_checkfloat(L, 2);
  float threshold = luax_optfloat(L, 3, 0.0f);
  lovrColliderSetAngularDamping(collider, damping, threshold);
  return 0;
}

static int l_lovrColliderApplyForce(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float force[3];
  int index = luax_readvec3(L, 2, force, NULL);

  if (lua_gettop(L) >= index) {
    float position[3];
    luax_readvec3(L, index, position, NULL);
    lovrColliderApplyForceAtPosition(collider, force, position);
  } else {
    lovrColliderApplyForce(collider, force);
  }

  return 0;
}

static int l_lovrColliderApplyTorque(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float torque[3];
  luax_readvec3(L, 2, torque, NULL);
  lovrColliderApplyTorque(collider, torque);
  return 0;
}

static int l_lovrColliderApplyLinearImpulse(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float impulse[3];
  int index = luax_readvec3(L, 2, impulse, NULL);
  if (lua_gettop(L) >= index) {
    float position[3];
    luax_readvec3(L, index, position, NULL);
    lovrColliderApplyLinearImpulseAtPosition(collider, impulse, position);
  } else {
    lovrColliderApplyLinearImpulse(collider, impulse);
  }
  return 0;
}

static int l_lovrColliderApplyAngularImpulse(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float impulse[3];
  luax_readvec3(L, 2, impulse, NULL);
  lovrColliderApplyAngularImpulse(collider, impulse);
  return 0;
}

static int l_lovrColliderGetLocalPoint(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float world[3], local[3];
  luax_readvec3(L, 2, world, NULL);
  lovrColliderGetLocalPoint(collider, world, local);
  lua_pushnumber(L, local[0]);
  lua_pushnumber(L, local[1]);
  lua_pushnumber(L, local[2]);
  return 3;
}

static int l_lovrColliderGetWorldPoint(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float local[3], world[3];
  luax_readvec3(L, 2, local, NULL);
  lovrColliderGetWorldPoint(collider, local, world);
  lua_pushnumber(L, world[0]);
  lua_pushnumber(L, world[1]);
  lua_pushnumber(L, world[2]);
  return 3;
}

static int l_lovrColliderGetLocalVector(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float world[3], local[3];
  luax_readvec3(L, 2, world, NULL);
  lovrColliderGetLocalVector(collider, world, local);
  lua_pushnumber(L, local[0]);
  lua_pushnumber(L, local[1]);
  lua_pushnumber(L, local[2]);
  return 3;
}

static int l_lovrColliderGetWorldVector(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float local[3], world[3];
  luax_readvec3(L, 2, local, NULL);
  lovrColliderGetWorldVector(collider, local, world);
  lua_pushnumber(L, world[0]);
  lua_pushnumber(L, world[1]);
  lua_pushnumber(L, world[2]);
  return 3;
}

static int l_lovrColliderGetLinearVelocityFromLocalPoint(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float point[3], velocity[3];
  luax_readvec3(L, 2, point, NULL);
  lovrColliderGetLinearVelocityFromLocalPoint(collider, point, velocity);
  lua_pushnumber(L, velocity[0]);
  lua_pushnumber(L, velocity[1]);
  lua_pushnumber(L, velocity[2]);
  return 3;
}

static int l_lovrColliderGetLinearVelocityFromWorldPoint(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float point[3], velocity[3];
  luax_readvec3(L, 2, point, NULL);
  lovrColliderGetLinearVelocityFromWorldPoint(collider, point, velocity);
  lua_pushnumber(L, velocity[0]);
  lua_pushnumber(L, velocity[1]);
  lua_pushnumber(L, velocity[2]);
  return 3;
}

static int l_lovrColliderGetAABB(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float aabb[6];
  lovrColliderGetAABB(collider, aabb);
  for (int i = 0; i < 6; i++) {
    lua_pushnumber(L, aabb[i]);
  }
  return 6;
}

static int l_lovrColliderGetFriction(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushnumber(L, lovrColliderGetFriction(collider));
  return 1;
}

static int l_lovrColliderSetFriction(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float friction = luax_checkfloat(L, 2);
  lovrColliderSetFriction(collider, friction);
  return 0;
}

static int l_lovrColliderGetRestitution(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushnumber(L, lovrColliderGetRestitution(collider));
  return 1;
}

static int l_lovrColliderSetRestitution(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  float restitution = luax_checkfloat(L, 2);
  lovrColliderSetRestitution(collider, restitution);
  return 0;
}

static int l_lovrColliderGetTag(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushstring(L, lovrColliderGetTag(collider));
  return 1;
}

static int l_lovrColliderSetTag(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lovrColliderSetTag(collider, lua_tostring(L, 2));
  return 0;
}

// Deprecated
static int l_lovrColliderGetShapes(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  Shape* shape = lovrColliderGetShape(collider);
  lua_createtable(L, 1, 0);
  luax_pushshape(L, shape);
  lua_rawseti(L, -2, 1);
  return 1;
}

// Deprecated
static int l_lovrColliderIsGravityIgnored(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  lua_pushboolean(L, lovrColliderGetGravityScale(collider) == 0.f);
  return 1;
}

// Deprecated
static int l_lovrColliderSetGravityIgnored(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  bool ignored = lua_toboolean(L, 2);
  lovrColliderSetGravityScale(collider, ignored ? 0.f : 1.f);
  return 0;
}

const luaL_Reg lovrCollider[] = {
  { "destroy", l_lovrColliderDestroy },
  { "isDestroyed", l_lovrColliderIsDestroyed },
  { "isEnabled", l_lovrColliderIsEnabled },
  { "setEnabled", l_lovrColliderSetEnabled },
  { "getWorld", l_lovrColliderGetWorld },
  { "getShape", l_lovrColliderGetShape },
  { "setShape", l_lovrColliderSetShape },
  { "getJoints", l_lovrColliderGetJoints },
  { "getUserData", l_lovrColliderGetUserData },
  { "setUserData", l_lovrColliderSetUserData },
  { "isKinematic", l_lovrColliderIsKinematic },
  { "setKinematic", l_lovrColliderSetKinematic },
  { "isSensor", l_lovrColliderIsSensor },
  { "setSensor", l_lovrColliderSetSensor },
  { "isContinuous", l_lovrColliderIsContinuous },
  { "setContinuous", l_lovrColliderSetContinuous },
  { "getGravityScale", l_lovrColliderGetGravityScale },
  { "setGravityScale", l_lovrColliderSetGravityScale },
  { "isSleepingAllowed", l_lovrColliderIsSleepingAllowed },
  { "setSleepingAllowed", l_lovrColliderSetSleepingAllowed },
  { "isAwake", l_lovrColliderIsAwake },
  { "setAwake", l_lovrColliderSetAwake },
  { "getMass", l_lovrColliderGetMass },
  { "setMass", l_lovrColliderSetMass },
  { "getInertia", l_lovrColliderGetInertia },
  { "setInertia", l_lovrColliderSetInertia },
  { "getCenterOfMass", l_lovrColliderGetCenterOfMass },
  { "setCenterOfMass", l_lovrColliderSetCenterOfMass },
  { "resetMassData", l_lovrColliderResetMassData },
  { "getEnabledAxes", l_lovrColliderGetEnabledAxes },
  { "setEnabledAxes", l_lovrColliderSetEnabledAxes },
  { "getPosition", l_lovrColliderGetPosition },
  { "setPosition", l_lovrColliderSetPosition },
  { "getOrientation", l_lovrColliderGetOrientation },
  { "setOrientation", l_lovrColliderSetOrientation },
  { "getPose", l_lovrColliderGetPose },
  { "setPose", l_lovrColliderSetPose },
  { "getRawPosition", l_lovrColliderGetRawPosition },
  { "getRawOrientation", l_lovrColliderGetRawOrientation },
  { "getLinearVelocity", l_lovrColliderGetLinearVelocity },
  { "setLinearVelocity", l_lovrColliderSetLinearVelocity },
  { "getAngularVelocity", l_lovrColliderGetAngularVelocity },
  { "setAngularVelocity", l_lovrColliderSetAngularVelocity },
  { "getLinearDamping", l_lovrColliderGetLinearDamping },
  { "setLinearDamping", l_lovrColliderSetLinearDamping },
  { "getAngularDamping", l_lovrColliderGetAngularDamping },
  { "setAngularDamping", l_lovrColliderSetAngularDamping },
  { "applyForce", l_lovrColliderApplyForce },
  { "applyTorque", l_lovrColliderApplyTorque },
  { "applyLinearImpulse", l_lovrColliderApplyLinearImpulse },
  { "applyAngularImpulse", l_lovrColliderApplyAngularImpulse },
  { "getLocalPoint", l_lovrColliderGetLocalPoint },
  { "getWorldPoint", l_lovrColliderGetWorldPoint },
  { "getLocalVector", l_lovrColliderGetLocalVector },
  { "getWorldVector", l_lovrColliderGetWorldVector },
  { "getLinearVelocityFromLocalPoint", l_lovrColliderGetLinearVelocityFromLocalPoint },
  { "getLinearVelocityFromWorldPoint", l_lovrColliderGetLinearVelocityFromWorldPoint },
  { "getAABB", l_lovrColliderGetAABB },
  { "getFriction", l_lovrColliderGetFriction },
  { "setFriction", l_lovrColliderSetFriction },
  { "getRestitution", l_lovrColliderGetRestitution },
  { "setRestitution", l_lovrColliderSetRestitution },
  { "getTag", l_lovrColliderGetTag },
  { "setTag", l_lovrColliderSetTag },

  // Deprecated
  { "getShapes", l_lovrColliderGetShapes },
  { "isGravityIgnored", l_lovrColliderIsGravityIgnored },
  { "setGravityIgnored", l_lovrColliderSetGravityIgnored },

  { NULL, NULL }
};
