#include "api.h"
#include "physics/physics.h"
#include "util.h"

StringEntry lovrShapeType[] = {
  [SHAPE_SPHERE] = ENTRY("sphere"),
  [SHAPE_BOX] = ENTRY("box"),
  [SHAPE_CAPSULE] = ENTRY("capsule"),
  [SHAPE_CYLINDER] = ENTRY("cylinder"),
  [SHAPE_CONVEX] = ENTRY("convex"),
  [SHAPE_MESH] = ENTRY("mesh"),
  [SHAPE_TERRAIN] = ENTRY("terrain"),
  [SHAPE_COMPOUND] = ENTRY("compound"),
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
  WorldInfo info = {
    .maxColliders = 65536,
    .maxColliderPairs = 65536,
    .maxContacts = 16384,
    .allowSleep = true
  };

  if (lua_istable(L, 1)) {
    lua_getfield(L, 1, "maxColliders");
    if (!lua_isnil(L, -1)) info.maxColliders = luax_checku32(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "maxColliderPairs");
    if (!lua_isnil(L, -1)) info.maxColliderPairs = luax_checku32(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "maxContacts");
    if (!lua_isnil(L, -1)) info.maxContacts = luax_checku32(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "allowSleep");
    if (!lua_isnil(L, -1)) info.allowSleep = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "tags");
    if (!lua_isnil(L, -1)) {
      lovrCheck(lua_istable(L, -1), "World tag list should be a table");
      lovrCheck(info.tagCount <= MAX_TAGS, "Max number of world tags is %d", MAX_TAGS);
      info.tagCount = luax_len(L, 5);
      for (uint32_t i = 0; i < info.tagCount; i++) {
        lua_rawgeti(L, -1, (int) i + 1);
        if (lua_isstring(L, -1)) {
          info.tags[i] = lua_tostring(L, -1);
        } else {
          return luaL_error(L, "World tags must be a table of strings");
        }
        lua_pop(L, 1);
      }
    }
    lua_pop(L, 1);
  } else { // Deprecated
    info.allowSleep = lua_gettop(L) < 4 || lua_toboolean(L, 4);
    if (lua_type(L, 5) == LUA_TTABLE) {
      info.tagCount = luax_len(L, 5);
      lovrCheck(info.tagCount <= MAX_TAGS, "Max number of world tags is %d", MAX_TAGS);
      for (uint32_t i = 0; i < info.tagCount; i++) {
        lua_rawgeti(L, -1, (int) i + 1);
        if (lua_isstring(L, -1)) {
          info.tags[i] = lua_tostring(L, -1);
        } else {
          return luaL_error(L, "World tags must be a table of strings");
        }
        lua_pop(L, 1);
      }
    } else {
      info.tagCount = 0;
    }
  }

  World* world = lovrWorldCreate(&info);

  if (!lua_istable(L, 1)) {
    float gravity[3];
    gravity[0] = luax_optfloat(L, 1, 0.f);
    gravity[1] = luax_optfloat(L, 2, -9.81f);
    gravity[2] = luax_optfloat(L, 3, 0.f);
    lovrWorldSetGravity(world, gravity);
  }

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

static int l_lovrPhysicsNewConvexShape(lua_State* L) {
  ConvexShape* convex = luax_newconvexshape(L, 1);
  luax_pushtype(L, ConvexShape, convex);
  lovrRelease(convex, lovrShapeDestroy);
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

static int l_lovrPhysicsNewCompoundShape(lua_State* L) {
  CompoundShape* shape = luax_newcompoundshape(L, 1);
  luax_pushtype(L, CompoundShape, shape);
  lovrRelease(shape, lovrShapeDestroy);
  return 1;
}

static const luaL_Reg lovrPhysics[] = {
  { "newWorld", l_lovrPhysicsNewWorld },
  { "newBallJoint", l_lovrPhysicsNewBallJoint },
  { "newBoxShape", l_lovrPhysicsNewBoxShape },
  { "newCapsuleShape", l_lovrPhysicsNewCapsuleShape },
  { "newConvexShape", l_lovrPhysicsNewConvexShape },
  { "newCylinderShape", l_lovrPhysicsNewCylinderShape },
  { "newDistanceJoint", l_lovrPhysicsNewDistanceJoint },
  { "newHingeJoint", l_lovrPhysicsNewHingeJoint },
  { "newMeshShape", l_lovrPhysicsNewMeshShape },
  { "newSliderJoint", l_lovrPhysicsNewSliderJoint },
  { "newSphereShape", l_lovrPhysicsNewSphereShape },
  { "newTerrainShape", l_lovrPhysicsNewTerrainShape },
  { "newCompoundShape", l_lovrPhysicsNewCompoundShape },
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
extern const luaL_Reg lovrConvexShape[];
extern const luaL_Reg lovrMeshShape[];
extern const luaL_Reg lovrTerrainShape[];
extern const luaL_Reg lovrCompoundShape[];

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
  luax_registertype(L, ConvexShape);
  luax_registertype(L, MeshShape);
  luax_registertype(L, TerrainShape);
  luax_registertype(L, CompoundShape);
  lovrPhysicsInit();
  luax_atexit(L, lovrPhysicsDestroy);
  return 1;
}
