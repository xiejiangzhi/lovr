#include "api.h"
#include "physics/physics.h"
#include "util.h"

StringEntry lovrShapeType[] = {
  [SHAPE_SPHERE] = ENTRY("sphere"),
  [SHAPE_BOX] = ENTRY("box"),
  [SHAPE_CAPSULE] = ENTRY("capsule"),
  [SHAPE_CYLINDER] = ENTRY("cylinder"),
  [SHAPE_MESH] = ENTRY("mesh"),
  [SHAPE_TERRAIN] = ENTRY("terrain"),
  { 0 }
};

StringEntry lovrJointType[] = {
  [JOINT_BALL] = ENTRY("ball"),
  [JOINT_DISTANCE] = ENTRY("distance"),
  [JOINT_HINGE] = ENTRY("hinge"),
  [JOINT_SLIDER] = ENTRY("slider"),
  { 0 }
};

static int l_lovrPhysicsNewWorld(lua_State* L) {
  float xg = luax_optfloat(L, 1, 0.f);
  float yg = luax_optfloat(L, 2, -9.81f);
  float zg = luax_optfloat(L, 3, 0.f);
  bool allowSleep = lua_gettop(L) < 4 || lua_toboolean(L, 4);
  const char* tags[MAX_TAGS];
  int tagCount;
  if (lua_type(L, 5) == LUA_TTABLE) {
    tagCount = luax_len(L, 5);
    lovrCheck(tagCount <= MAX_TAGS, "Max number of world tags is %d", MAX_TAGS);
    for (int i = 0; i < tagCount; i++) {
      lua_rawgeti(L, -1, i + 1);
      if (lua_isstring(L, -1)) {
        tags[i] = lua_tostring(L, -1);
      } else {
        return luaL_error(L, "World tags must be a table of strings");
      }
      lua_pop(L, 1);
    }
  } else {
    tagCount = 0;
  }
  World* world = lovrWorldCreate(xg, yg, zg, allowSleep, tags, tagCount);
  luax_pushtype(L, World, world);
  lovrRelease(world, lovrWorldDestroy);
  return 1;
}

static int l_lovrPhysicsNewBallJoint(lua_State* L) {
  Collider* a = luax_checktype(L, 1, Collider);
  Collider* b = luax_checktype(L, 2, Collider);
  float anchor[3];
  luax_readvec3(L, 3, anchor, NULL);
  BallJoint* joint = lovrBallJointCreate(a, b, anchor);
  luax_pushtype(L, BallJoint, joint);
  lovrRelease(joint, lovrJointDestroy);
  return 1;
}

static int l_lovrPhysicsNewBoxShape(lua_State* L) {
  BoxShape* box = luax_newboxshape(L, 1);
  luax_pushtype(L, BoxShape, box);
  lovrRelease(box, lovrShapeDestroy);
  return 1;
}

static int l_lovrPhysicsNewCapsuleShape(lua_State* L) {
  CapsuleShape* capsule = luax_newcapsuleshape(L, 1);
  luax_pushtype(L, CapsuleShape, capsule);
  lovrRelease(capsule, lovrShapeDestroy);
  return 1;
}

static int l_lovrPhysicsNewCylinderShape(lua_State* L) {
  CylinderShape* cylinder = luax_newcylindershape(L, 1);
  luax_pushtype(L, CylinderShape, cylinder);
  lovrRelease(cylinder, lovrShapeDestroy);
  return 1;
}

static int l_lovrPhysicsNewDistanceJoint(lua_State* L) {
  Collider* a = luax_checktype(L, 1, Collider);
  Collider* b = luax_checktype(L, 2, Collider);
  float anchor1[3], anchor2[3];
  int index = luax_readvec3(L, 3, anchor1, NULL);
  luax_readvec3(L, index, anchor2, NULL);
  DistanceJoint* joint = lovrDistanceJointCreate(a, b, anchor1, anchor2);
  luax_pushtype(L, DistanceJoint, joint);
  lovrRelease(joint, lovrJointDestroy);
  return 1;
}

static int l_lovrPhysicsNewHingeJoint(lua_State* L) {
  Collider* a = luax_checktype(L, 1, Collider);
  Collider* b = luax_checktype(L, 2, Collider);
  float anchor[3], axis[3];
  int index = luax_readvec3(L, 3, anchor, NULL);
  luax_readvec3(L, index, axis, NULL);
  HingeJoint* joint = lovrHingeJointCreate(a, b, anchor, axis);
  luax_pushtype(L, HingeJoint, joint);
  lovrRelease(joint, lovrJointDestroy);
  return 1;
}

static int l_lovrPhysicsNewMeshShape(lua_State* L) {
  MeshShape* mesh = luax_newmeshshape(L, 1);
  luax_pushtype(L, MeshShape, mesh);
  lovrRelease(mesh, lovrShapeDestroy);
  return 1;
}

static int l_lovrPhysicsNewSliderJoint(lua_State* L) {
  Collider* a = luax_checktype(L, 1, Collider);
  Collider* b = luax_checktype(L, 2, Collider);
  float axis[3];
  luax_readvec3(L, 3, axis, NULL);
  SliderJoint* joint = lovrSliderJointCreate(a, b, axis);
  luax_pushtype(L, SliderJoint, joint);
  lovrRelease(joint, lovrJointDestroy);
  return 1;
}

static int l_lovrPhysicsNewSphereShape(lua_State* L) {
  SphereShape* sphere = luax_newsphereshape(L, 1);
  luax_pushtype(L, SphereShape, sphere);
  lovrRelease(sphere, lovrShapeDestroy);
  return 1;
}

static int l_lovrPhysicsNewTerrainShape(lua_State* L) {
  TerrainShape* terrain = luax_newterrainshape(L, 1);
  luax_pushtype(L, TerrainShape, terrain);
  lovrRelease(terrain, lovrShapeDestroy);
  return 1;
}

static const luaL_Reg lovrPhysics[] = {
  { "newWorld", l_lovrPhysicsNewWorld },
  { "newBallJoint", l_lovrPhysicsNewBallJoint },
  { "newBoxShape", l_lovrPhysicsNewBoxShape },
  { "newCapsuleShape", l_lovrPhysicsNewCapsuleShape },
  { "newCylinderShape", l_lovrPhysicsNewCylinderShape },
  { "newDistanceJoint", l_lovrPhysicsNewDistanceJoint },
  { "newHingeJoint", l_lovrPhysicsNewHingeJoint },
  { "newMeshShape", l_lovrPhysicsNewMeshShape },
  { "newSliderJoint", l_lovrPhysicsNewSliderJoint },
  { "newSphereShape", l_lovrPhysicsNewSphereShape },
  { "newTerrainShape", l_lovrPhysicsNewTerrainShape },
  { NULL, NULL }
};

extern const luaL_Reg lovrWorld[];
extern const luaL_Reg lovrCollider[];
extern const luaL_Reg lovrBallJoint[];
extern const luaL_Reg lovrDistanceJoint[];
extern const luaL_Reg lovrHingeJoint[];
extern const luaL_Reg lovrSliderJoint[];
extern const luaL_Reg lovrSphereShape[];
extern const luaL_Reg lovrBoxShape[];
extern const luaL_Reg lovrCapsuleShape[];
extern const luaL_Reg lovrCylinderShape[];
extern const luaL_Reg lovrMeshShape[];
extern const luaL_Reg lovrTerrainShape[];

int luaopen_lovr_physics(lua_State* L) {
  lua_newtable(L);
  luax_register(L, lovrPhysics);
  luax_registertype(L, World);
  luax_registertype(L, Collider);
  luax_registertype(L, BallJoint);
  luax_registertype(L, DistanceJoint);
  luax_registertype(L, HingeJoint);
  luax_registertype(L, SliderJoint);
  luax_registertype(L, SphereShape);
  luax_registertype(L, BoxShape);
  luax_registertype(L, CapsuleShape);
  luax_registertype(L, CylinderShape);
  luax_registertype(L, MeshShape);
  luax_registertype(L, TerrainShape);
  lovrPhysicsInit();
  luax_atexit(L, lovrPhysicsDestroy);
  return 1;
}
