#include "api.h"
#include "physics/physics.h"
#include "util.h"

static int l_lovrContactGetColliders(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  Collider* a = lovrContactGetColliderA(contact);
  Collider* b = lovrContactGetColliderB(contact);
  luax_pushtype(L, Collider, a);
  luax_pushtype(L, Collider, b);
  return 2;
}

static int l_lovrContactGetShapes(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  Shape* a = lovrContactGetShapeA(contact);
  Shape* b = lovrContactGetShapeB(contact);
  luax_pushshape(L, a);
  luax_pushshape(L, b);
  return 2;
}

static int l_lovrContactGetNormal(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  float normal[3];
  lovrContactGetNormal(contact, normal);
  lua_pushnumber(L, normal[0]);
  lua_pushnumber(L, normal[1]);
  lua_pushnumber(L, normal[2]);
  return 3;
}

static int l_lovrContactGetPenetration(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  float penetration = lovrContactGetPenetration(contact);
  lua_pushnumber(L, penetration);
  return 1;
}

static int l_lovrContactGetPoints(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  uint32_t count = lovrContactGetPointCount(contact);
  lua_checkstack(L, count * 3);
  float point[3];
  for (uint32_t i = 0; i < count; i++) {
    lovrContactGetPoint(contact, i, point);
    lua_pushnumber(L, point[0]);
    lua_pushnumber(L, point[1]);
    lua_pushnumber(L, point[2]);
  }
  return count * 3;
}

static int l_lovrContactGetFriction(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  float friction = lovrContactGetFriction(contact);
  lua_pushnumber(L, friction);
  return 1;
}

static int l_lovrContactSetFriction(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  float friction = luax_checkfloat(L, 2);
  lovrContactSetFriction(contact, friction);
  return 0;
}

static int l_lovrContactGetRestitution(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  float restitution = lovrContactGetRestitution(contact);
  lua_pushnumber(L, restitution);
  return 1;
}

static int l_lovrContactSetRestitution(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  float restitution = luax_checkfloat(L, 2);
  lovrContactSetRestitution(contact, restitution);
  return 0;
}

static int l_lovrContactIsEnabled(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  bool enabled = lovrContactIsEnabled(contact);
  lua_pushboolean(L, enabled);
  return 1;
}

static int l_lovrContactSetEnabled(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  lovrContactSetEnabled(contact, lua_toboolean(L, 2));
  return 0;
}

static int l_lovrContactGetSurfaceVelocity(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  float velocity[3];
  lovrContactGetSurfaceVelocity(contact, velocity);
  lua_pushnumber(L, velocity[0]);
  lua_pushnumber(L, velocity[1]);
  lua_pushnumber(L, velocity[2]);
  return 3;
}

static int l_lovrContactSetSurfaceVelocity(lua_State* L) {
  Contact* contact = luax_checktype(L, 1, Contact);
  float velocity[3];
  luax_readvec3(L, 2, velocity, NULL);
  lovrContactSetSurfaceVelocity(contact, velocity);
  return 0;
}

const luaL_Reg lovrContact[] = {
  { "getColliders", l_lovrContactGetColliders },
  { "getShapes", l_lovrContactGetShapes },
  { "getNormal", l_lovrContactGetNormal },
  { "getPenetration", l_lovrContactGetPenetration },
  { "getPoints", l_lovrContactGetPoints },
  { "getFriction", l_lovrContactGetFriction },
  { "setFriction", l_lovrContactSetFriction },
  { "getRestitution", l_lovrContactGetRestitution },
  { "setRestitution", l_lovrContactSetRestitution },
  { "isEnabled", l_lovrContactIsEnabled },
  { "setEnabled", l_lovrContactSetEnabled },
  { "getSurfaceVelocity", l_lovrContactGetSurfaceVelocity },
  { "setSurfaceVelocity", l_lovrContactSetSurfaceVelocity },
  { NULL, NULL }
};
