#include "physics/physics.h"
#include "core/maf.h"
#include "util.h"
#include <stdlib.h>
#include <joltc.h>

struct World {
  uint32_t ref;
  JPH_PhysicsSystem* system;
  JPH_BodyInterface* bodies;
  JPH_ObjectLayerPairFilter* objectLayerPairFilter;
  Collider* colliders;
  Joint* joints;
  uint32_t jointCount;
  float defaultLinearDamping;
  float defaultAngularDamping;
  bool defaultIsSleepingAllowed;
  int collisionSteps;
  char* tags[MAX_TAGS];
};

struct Collider {
  uint32_t ref;
  JPH_BodyID id;
  JPH_Body* body;
  World* world;
  Joint* joints;
  Shape* shape;
  Collider* prev;
  Collider* next;
  uint32_t tag;
};

struct Shape {
  uint32_t ref;
  ShapeType type;
  JPH_Shape* shape;
};

typedef struct {
  Joint* prev;
  Joint* next;
} JointNode;

struct Joint {
  uint32_t ref;
  JointType type;
  JPH_Constraint* constraint;
  JointNode a, b, world;
};

static struct {
  bool initialized;
  Shape* pointShape;
  JPH_Shape* queryBox;
  JPH_Shape* querySphere;
  JPH_AllHit_CastShapeCollector* castShapeCollector;
} state;

// Broad phase and object phase layers
#define NUM_OP_LAYERS ((MAX_TAGS + 1) * 2)
#define NUM_BP_LAYERS 2

// UNTAGGED = 16 is mapped to object layers: 32 is kinematic untagged, and 33 is dynamic untagged
// (NO_TAG = ~0u is not used in jolt implementation)
#define UNTAGGED (MAX_TAGS)

// XXX slow, but probably fine (tag names are not on any critical path), could switch to hashing if needed
static uint32_t findTag(World* world, const char* name) {
  for (uint32_t i = 0; i < MAX_TAGS && world->tags[i]; i++) {
    if (!strcmp(world->tags[i], name)) {
      return i;
    }
  }
  return UNTAGGED;
}

bool lovrPhysicsInit(void) {
  if (state.initialized) return false;
  JPH_Init(32 * 1024 * 1024);
  state.pointShape = lovrSphereShapeCreate(FLT_EPSILON);
  state.querySphere = (JPH_Shape*) JPH_SphereShape_Create(1.f);
  state.queryBox = (JPH_Shape*) JPH_BoxShape_Create(&(const JPH_Vec3) { .5, .5f, .5f }, 0.f);
  state.castShapeCollector = JPH_AllHit_CastShapeCollector_Create();
  return state.initialized = true;
}

void lovrPhysicsDestroy(void) {
  if (!state.initialized) return;
  lovrRelease(state.pointShape, lovrShapeDestroy);
  JPH_Shutdown();
  state.initialized = false;
}

World* lovrWorldCreate(WorldInfo* info) {
  World* world = lovrCalloc(sizeof(World));

  world->ref = 1;
  world->collisionSteps = 1;
  world->defaultLinearDamping = .05f;
  world->defaultAngularDamping = .05f;
  world->defaultIsSleepingAllowed = info->allowSleep;

  JPH_BroadPhaseLayerInterface* broadPhaseLayerInterface = JPH_BroadPhaseLayerInterfaceTable_Create(NUM_OP_LAYERS, NUM_BP_LAYERS);
  world->objectLayerPairFilter = JPH_ObjectLayerPairFilterTable_Create(NUM_OP_LAYERS);
  for (uint32_t i = 0; i < NUM_OP_LAYERS; i++) {
    for (uint32_t j = i; j < NUM_OP_LAYERS; j++) {
    JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(broadPhaseLayerInterface, i, i % 2);
      if (i % 2 == 0 && j % 2 == 0) {
        JPH_ObjectLayerPairFilterTable_DisableCollision(world->objectLayerPairFilter, i, j);
      } else {
        JPH_ObjectLayerPairFilterTable_EnableCollision(world->objectLayerPairFilter, i, j);
      }
    }
  }
  JPH_ObjectVsBroadPhaseLayerFilter* broadPhaseLayerFilter = JPH_ObjectVsBroadPhaseLayerFilterTable_Create(
    broadPhaseLayerInterface, NUM_BP_LAYERS,
    world->objectLayerPairFilter, NUM_OP_LAYERS);

  JPH_PhysicsSystemSettings settings = {
    .maxBodies = info->maxColliders,
    .maxBodyPairs = info->maxColliderPairs,
    .maxContactConstraints = info->maxContacts,
    .broadPhaseLayerInterface = broadPhaseLayerInterface,
    .objectLayerPairFilter = world->objectLayerPairFilter,
    .objectVsBroadPhaseLayerFilter = broadPhaseLayerFilter
  };
  world->system = JPH_PhysicsSystem_Create(&settings);
  world->bodies = JPH_PhysicsSystem_GetBodyInterface(world->system);

  for (uint32_t i = 0; i < info->tagCount; i++) {
    size_t size = strlen(info->tags[i]) + 1;
    world->tags[i] = lovrMalloc(size);
    memcpy(world->tags[i], info->tags[i], size);
  }

  return world;
}

void lovrWorldDestroy(void* ref) {
  World* world = ref;
  lovrWorldDestroyData(world);
  for (uint32_t i = 0; i < MAX_TAGS - 1 && world->tags[i]; i++) {
    lovrFree(world->tags[i]);
  }
  lovrFree(world);
}

void lovrWorldDestroyData(World* world) {
  while (world->colliders) {
    Collider* collider = world->colliders;
    Collider* next = collider->next;
    lovrColliderDestroyData(collider);
    world->colliders = next;
  }
  JPH_PhysicsSystem_Destroy(world->system);
}

uint32_t lovrWorldGetColliderCount(World* world) {
  return JPH_PhysicsSystem_GetNumBodies(world->system);
}

uint32_t lovrWorldGetJointCount(World* world) {
  return world->jointCount; // Jolt doesn't expose this
}

Collider* lovrWorldGetColliders(World* world, Collider* collider) {
  return collider ? collider->next : world->colliders;
}

Joint* lovrWorldGetJoints(World* world, Joint* joint) {
  return joint ? joint->world.next : world->joints;
}

void lovrWorldUpdate(World* world, float dt, CollisionResolver resolver, void* userdata) {
  JPH_PhysicsSystem_Step(world->system, dt, world->collisionSteps);
}

void lovrWorldComputeOverlaps(World* world) {
  //
}

int lovrWorldGetNextOverlap(World* world, Shape** a, Shape** b) {
  return 0;
}

int lovrWorldCollide(World* world, Shape* a, Shape* b, float friction, float restitution) {
  return 0;
}

void lovrWorldGetContacts(World* world, Shape* a, Shape* b, Contact contacts[MAX_CONTACTS], uint32_t* count) {
  //
}

void lovrWorldRaycast(World* world, float start[3], float end[3], RaycastCallback callback, void* userdata) {
  const JPH_NarrowPhaseQuery* query = JPC_PhysicsSystem_GetNarrowPhaseQueryNoLock(world->system);
  const JPH_RVec3 origin = { start[0], start[1], start[2] };
  const JPH_Vec3 direction = { end[0] - start[0], end[1] - start[1], end[2] - start[2] };
  JPH_AllHit_CastRayCollector* collector = JPH_AllHit_CastRayCollector_Create();
  JPH_NarrowPhaseQuery_CastRayAll(query, &origin, &direction, collector, NULL, NULL, NULL);

  size_t count;
  JPH_RayCastResult* hits = JPH_AllHit_CastRayCollector_GetHits(collector, &count);

  for (size_t i = 0; i < count; i++) {
    Collider* collider = (Collider*) (uintptr_t) JPH_BodyInterface_GetUserData(world->bodies, hits[i].bodyID);
    uint32_t child = 0;

    if (collider->shape->type == SHAPE_COMPOUND) {
      JPH_SubShapeID id = hits[i].subShapeID2;
      JPH_SubShapeID remainder;
      child = JPH_CompoundShape_GetSubShapeIndexFromID((JPH_CompoundShape*) collider->shape, id, &remainder);
    }

    JPH_RVec3 position = {
      start[0] + hits[i].fraction * direction.x,
      start[1] + hits[i].fraction * direction.y,
      start[2] + hits[i].fraction * direction.z
    };

    JPH_Vec3 normal;
    JPH_Body_GetWorldSpaceSurfaceNormal(collider->body, hits[i].subShapeID2, &position, &normal);

    if (callback(collider, &position.x, &normal.x, child, userdata)) {
      break;
    }
  }

  JPH_AllHit_CastRayCollector_Destroy(collector);
}

static bool lovrWorldQueryShape(World* world, JPH_Shape* shape, float position[3], float scale[3], QueryCallback callback, void* userdata) {
  JPH_RMatrix4x4 transform;
  float* m = &transform.m11;
  mat4_identity(m);
  mat4_translate(m, position[0], position[1], position[2]);
  mat4_scale(m, scale[0], scale[1], scale[2]);

  JPH_Vec3 direction = { 0.f, 0.f, 0.f };
  JPH_RVec3 baseOffset = { 0.f, 0.f, 0.f };
  const JPH_NarrowPhaseQuery* query = JPC_PhysicsSystem_GetNarrowPhaseQueryNoLock(world->system);
  JPH_AllHit_CastShapeCollector_Reset(state.castShapeCollector);
  JPH_NarrowPhaseQuery_CastShape(query, shape, &transform, &direction, &baseOffset, state.castShapeCollector);

  size_t count;
  JPH_AllHit_CastShapeCollector_GetHits(state.castShapeCollector, &count);

  for (size_t i = 0; i < count; i++) {
    JPH_BodyID id = JPH_AllHit_CastShapeCollector_GetBodyID2(state.castShapeCollector, i);
    Collider* collider = (Collider*) (uintptr_t) JPH_BodyInterface_GetUserData(world->bodies, id);

    if (callback(collider, 0, userdata)) {
      break;
    }
  }

  return count > 0;
}

bool lovrWorldQueryBox(World* world, float position[3], float size[3], QueryCallback callback, void* userdata) {
  return lovrWorldQueryShape(world, state.queryBox, position, size, callback, userdata);
}

bool lovrWorldQuerySphere(World* world, float position[3], float radius, QueryCallback callback, void* userdata) {
  float scale[3] = { radius, radius, radius };
  return lovrWorldQueryShape(world, state.querySphere, position, scale, callback, userdata);
}

void lovrWorldGetGravity(World* world, float gravity[3]) {
  JPH_Vec3 g;
  JPH_PhysicsSystem_GetGravity(world->system, &g);
  vec3_set(gravity, g.x, g.y, g.z);
}

void lovrWorldSetGravity(World* world, float gravity[3]) {
  JPH_Vec3 g = { gravity[0], gravity[1], gravity[2] };
  JPH_PhysicsSystem_SetGravity(world->system, &g);
}

const char* lovrWorldGetTagName(World* world, uint32_t tag) {
  return (tag == UNTAGGED) ? NULL : world->tags[tag];
}

void lovrWorldDisableCollisionBetween(World* world, const char* tag1, const char* tag2) {
  uint32_t i = findTag(world, tag1);
  uint32_t j = findTag(world, tag2);
  if (i == UNTAGGED || j == UNTAGGED) {
    return;
  }
  uint32_t iStatic = i * 2;
  uint32_t jStatic = j * 2;
  uint32_t iDynamic = i * 2 + 1;
  uint32_t jDynamic = j * 2 + 1;
  JPH_ObjectLayerPairFilterTable_DisableCollision(world->objectLayerPairFilter, iDynamic, jDynamic);
  JPH_ObjectLayerPairFilterTable_DisableCollision(world->objectLayerPairFilter, iDynamic, jStatic);
  JPH_ObjectLayerPairFilterTable_DisableCollision(world->objectLayerPairFilter, iStatic, jDynamic);
}

void lovrWorldEnableCollisionBetween(World* world, const char* tag1, const char* tag2) {
  uint32_t i = findTag(world, tag1);
  uint32_t j = findTag(world, tag2);
  if (i == UNTAGGED || j == UNTAGGED) {
    return;
  }
  uint32_t iStatic = i * 2;
  uint32_t jStatic = j * 2;
  uint32_t iDynamic = i * 2 + 1;
  uint32_t jDynamic = j * 2 + 1;
  JPH_ObjectLayerPairFilterTable_EnableCollision(world->objectLayerPairFilter, iDynamic, jDynamic);
  JPH_ObjectLayerPairFilterTable_EnableCollision(world->objectLayerPairFilter, iDynamic, jStatic);
  JPH_ObjectLayerPairFilterTable_EnableCollision(world->objectLayerPairFilter, iStatic, jDynamic);
}

bool lovrWorldIsCollisionEnabledBetween(World* world, const char* tag1, const char* tag2) {
  uint32_t i = findTag(world, tag1);
  uint32_t j = findTag(world, tag2);
  if (i == UNTAGGED || j == UNTAGGED) {
    return true;
  }
  uint32_t iDynamic = i * 2 + 1;
  uint32_t jDynamic = j * 2 + 1;
  return JPH_ObjectLayerPairFilterTable_ShouldCollide(world->objectLayerPairFilter, iDynamic, jDynamic);
}

// Deprecated
int lovrWorldGetStepCount(World* world) { return world->collisionSteps; }
void lovrWorldSetStepCount(World* world, int iterations) { world->collisionSteps = iterations;}
float lovrWorldGetResponseTime(World* world) {}
void lovrWorldSetResponseTime(World* world, float responseTime) {}
float lovrWorldGetTightness(World* world) {}
void lovrWorldSetTightness(World* world, float tightness) {}
bool lovrWorldIsSleepingAllowed(World* world) { return world->defaultIsSleepingAllowed; }
void lovrWorldSetSleepingAllowed(World* world, bool allowed) { world->defaultIsSleepingAllowed = allowed; }
void lovrWorldGetLinearDamping(World* world, float* damping, float* threshold) { *damping = world->defaultLinearDamping, *threshold = 0.f; }
void lovrWorldSetLinearDamping(World* world, float damping, float threshold) { world->defaultLinearDamping = damping; }
void lovrWorldGetAngularDamping(World* world, float* damping, float* threshold) { *damping = world->defaultAngularDamping, *threshold = 0.f; }
void lovrWorldSetAngularDamping(World* world, float damping, float threshold) { world->defaultAngularDamping = damping; }

// Collider

Collider* lovrColliderCreate(World* world, Shape* shape, float position[3]) {
  uint32_t count = JPH_PhysicsSystem_GetNumBodies(world->system);
  uint32_t limit = JPH_PhysicsSystem_GetMaxBodies(world->system);
  lovrCheck(count < limit, "Too many colliders!");

  Collider* collider = lovrCalloc(sizeof(Collider));
  collider->ref = 1;
  collider->world = world;
  collider->shape = shape ? shape : state.pointShape;
  collider->tag = UNTAGGED;

  JPH_RVec3 p = { position[0], position[1], position[2] };
  JPH_Quat q = { 0.f, 0.f, 0.f, 1.f };
  JPH_MotionType type = JPH_MotionType_Dynamic;
  JPH_ObjectLayer objectLayer = UNTAGGED * 2 + 1;
  JPH_BodyCreationSettings* settings = JPH_BodyCreationSettings_Create3(collider->shape->shape, &p, &q, type, objectLayer);
  collider->body = JPH_BodyInterface_CreateBody(world->bodies, settings);
  collider->id = JPH_Body_GetID(collider->body);
  JPH_BodyCreationSettings_Destroy(settings);

  JPH_BodyInterface_AddBody(world->bodies, collider->id, JPH_Activation_Activate);
  JPH_BodyInterface_SetUserData(world->bodies, collider->id, (uint64_t) collider);

  lovrColliderSetLinearDamping(collider, world->defaultLinearDamping, 0.f);
  lovrColliderSetAngularDamping(collider, world->defaultAngularDamping, 0.f);
  lovrColliderSetSleepingAllowed(collider, world->defaultIsSleepingAllowed);

  if (world->colliders) {
    collider->next = world->colliders;
    collider->next->prev = collider;
  }

  world->colliders = collider;

  lovrRetain(collider->shape);
  lovrRetain(collider);
  return collider;
}

void lovrColliderDestroy(void* ref) {
  Collider* collider = ref;
  lovrColliderDestroyData(collider);
  lovrFree(collider);
}

void lovrColliderDestroyData(Collider* collider) {
  if (!collider->body) {
    return;
  }

  lovrRelease(collider->shape, lovrShapeDestroy);

  Joint* joint = collider->joints;

  while (joint) {
    Joint* next = lovrJointGetNext(joint, collider);
    lovrJointDestroyData(joint);
    joint = next;
  }

  World* world = collider->world;
  JPH_BodyInterface_RemoveBody(world->bodies, collider->id);
  JPH_BodyInterface_DestroyBody(world->bodies, collider->id);
  collider->body = NULL;

  if (collider->next) collider->next->prev = collider->prev;
  if (collider->prev) collider->prev->next = collider->next;
  if (world->colliders == collider) world->colliders = collider->next;
  collider->next = collider->prev = NULL;

  // If the Collider is destroyed, the world lets go of its reference to this Collider
  lovrRelease(collider, lovrColliderDestroy);
}

bool lovrColliderIsDestroyed(Collider* collider) {
  return !collider->body;
}

bool lovrColliderIsEnabled(Collider* collider) {
  return JPH_BodyInterface_IsAdded(collider->world->bodies, collider->id);
}

void lovrColliderSetEnabled(Collider* collider, bool enable) {
  if (enable && !lovrColliderIsEnabled(collider)) {
    JPH_BodyInterface_AddBody(collider->world->bodies, collider->id, JPH_Activation_DontActivate);
  } else if (!enable && lovrColliderIsEnabled(collider)) {
    JPH_BodyInterface_RemoveBody(collider->world->bodies, collider->id);
  }
}

void lovrColliderInitInertia(Collider* collider, Shape* shape) {
  //
}

World* lovrColliderGetWorld(Collider* collider) {
  return collider->world;
}

Joint* lovrColliderGetJoints(Collider* collider, Joint* joint) {
  return joint ? lovrJointGetNext(joint, collider) : collider->joints;
}

Shape* lovrColliderGetShape(Collider* collider, uint32_t child) {
  if (collider->shape == state.pointShape) {
    return NULL;
  }

  if (child == ~0u || collider->shape->type != SHAPE_COMPOUND) {
    return collider->shape;
  }

  return lovrCompoundShapeGetChild(collider->shape, child);
}

void lovrColliderSetShape(Collider* collider, Shape* shape) {
  shape = shape ? shape : state.pointShape;

  if (shape == collider->shape) {
    return;
  }

  float position[3], orientation[4];
  const JPH_Shape* parent = JPH_BodyInterface_GetShape(collider->world->bodies, collider->id);
  bool hasOffset = JPH_Shape_GetSubType(parent) == JPH_ShapeSubType_RotatedTranslated;
  if (hasOffset) lovrColliderGetShapeOffset(collider, position, orientation);

  lovrRelease(collider->shape, lovrShapeDestroy);
  collider->shape = shape;
  lovrRetain(shape);

  bool updateMass = true;
  if (shape->type == SHAPE_MESH || shape->type == SHAPE_TERRAIN) {
    lovrColliderSetKinematic(collider, true);
    updateMass = false;
  }

  JPH_BodyInterface_SetShape(collider->world->bodies, collider->id, shape->shape, updateMass, JPH_Activation_Activate);

  if (hasOffset) {
    lovrColliderSetShapeOffset(collider, position, orientation);
  }
}

void lovrColliderGetShapeOffset(Collider* collider, float position[3], float orientation[4]) {
  const JPH_Shape* shape = JPH_BodyInterface_GetShape(collider->world->bodies, collider->id);

  if (JPH_Shape_GetSubType(shape) == JPH_ShapeSubType_RotatedTranslated) {
    JPH_Vec3 p;
    JPH_Quat q;
    JPH_RotatedTranslatedShape_GetPosition((JPH_RotatedTranslatedShape*) shape, &p);
    JPH_RotatedTranslatedShape_GetRotation((JPH_RotatedTranslatedShape*) shape, &q);
    vec3_set(position, p.x, p.y, p.z);
    quat_set(orientation, q.x, q.y, q.z, q.w);
  } else {
    vec3_set(position, 0.f, 0.f, 0.f);
    quat_identity(orientation);
  }
}

void lovrColliderSetShapeOffset(Collider* collider, float position[3], float orientation[4]) {
  const JPH_Shape* shape = JPH_BodyInterface_GetShape(collider->world->bodies, collider->id);

  if (JPH_Shape_GetSubType(shape) == JPH_ShapeSubType_RotatedTranslated) {
    JPH_Shape_Destroy((JPH_Shape*) shape);
  }

  JPH_Vec3 p = { position[0], position[1], position[2] };
  JPH_Quat q = { orientation[0], orientation[1], orientation[2], orientation[3] };
  shape = (JPH_Shape*) JPH_RotatedTranslatedShape_Create(&p, &q, collider->shape->shape);
  bool updateMass = collider->shape && (collider->shape->type == SHAPE_MESH || collider->shape->type == SHAPE_TERRAIN);
  JPH_BodyInterface_SetShape(collider->world->bodies, collider->id, shape, updateMass, JPH_Activation_Activate);
}

const char* lovrColliderGetTag(Collider* collider) {
  return lovrWorldGetTagName(collider->world, collider->tag);
}

bool lovrColliderSetTag(Collider* collider, const char* tag) {
  if (!tag) {
    collider->tag = UNTAGGED;
  } else {
    collider->tag = findTag(collider->world, tag);
    if (collider->tag == UNTAGGED) {
      return false;
    }
  }
  bool kinematic = lovrColliderIsKinematic(collider);
  JPH_ObjectLayer objectLayer = collider->tag * 2 + (kinematic ? 0 : 1);
  JPH_BodyInterface_SetObjectLayer(collider->world->bodies, collider->id, objectLayer);
  return true;
}

float lovrColliderGetFriction(Collider* collider) {
  return JPH_BodyInterface_GetFriction(collider->world->bodies, collider->id);
}

void lovrColliderSetFriction(Collider* collider, float friction) {
  JPH_BodyInterface_SetFriction(collider->world->bodies, collider->id, friction);
}

float lovrColliderGetRestitution(Collider* collider) {
  return JPH_BodyInterface_GetRestitution(collider->world->bodies, collider->id);
}

void lovrColliderSetRestitution(Collider* collider, float restitution) {
  JPH_BodyInterface_SetRestitution(collider->world->bodies, collider->id, restitution);
}

bool lovrColliderIsKinematic(Collider* collider) {
  JPH_ObjectLayer objectLayer = JPH_BodyInterface_GetObjectLayer(collider->world->bodies, collider->id);
  return objectLayer % 2 == 0;
}

void lovrColliderSetKinematic(Collider* collider, bool kinematic) {
  JPH_ObjectLayer objectLayer = collider->tag * 2 + (kinematic ? 0 : 1);
  JPH_BodyInterface_SetObjectLayer(collider->world->bodies, collider->id, objectLayer);
  if (kinematic) {
    JPH_BodyInterface_DeactivateBody(collider->world->bodies, collider->id);
    JPH_BodyInterface_SetMotionType(
      collider->world->bodies,
      collider->id,
      JPH_MotionType_Kinematic,
      JPH_Activation_DontActivate);
  } else {
    JPH_BodyInterface_SetMotionType(
      collider->world->bodies,
      collider->id,
      JPH_MotionType_Dynamic,
      JPH_Activation_Activate);
  }
}

bool lovrColliderIsSensor(Collider* collider) {
  return JPH_Body_IsSensor(collider->body);
}

void lovrColliderSetSensor(Collider* collider, bool sensor) {
  JPH_Body_SetIsSensor(collider->body, sensor);
}

bool lovrColliderIsContinuous(Collider* collider) {
  return JPH_BodyInterface_GetMotionQuality(collider->world->bodies, collider->id) == JPH_MotionQuality_LinearCast;
}

void lovrColliderSetContinuous(Collider* collider, bool continuous) {
  JPH_MotionQuality quality = continuous ? JPH_MotionQuality_LinearCast : JPH_MotionQuality_Discrete;
  return JPH_BodyInterface_SetMotionQuality(collider->world->bodies, collider->id, quality);
}

float lovrColliderGetGravityScale(Collider* collider) {
  return JPH_BodyInterface_GetGravityFactor(collider->world->bodies, collider->id);
}

void lovrColliderSetGravityScale(Collider* collider, float scale) {
  return JPH_BodyInterface_SetGravityFactor(collider->world->bodies, collider->id, scale);
}

bool lovrColliderIsSleepingAllowed(Collider* collider) {
  return JPH_Body_GetAllowSleeping(collider->body);
}

void lovrColliderSetSleepingAllowed(Collider* collider, bool allowed) {
  JPH_Body_SetAllowSleeping(collider->body, allowed);
}

bool lovrColliderIsAwake(Collider* collider) {
  return JPH_BodyInterface_IsActive(collider->world->bodies, collider->id);
}

void lovrColliderSetAwake(Collider* collider, bool awake) {
  if (awake) {
    JPH_BodyInterface_ActivateBody(collider->world->bodies, collider->id);
  } else {
    JPH_BodyInterface_DeactivateBody(collider->world->bodies, collider->id);
  }
}

float lovrColliderGetMass(Collider* collider) {
  JPH_MotionProperties* motionProperties = JPH_Body_GetMotionProperties(collider->body);
  return 1.f / JPH_MotionProperties_GetInverseMassUnchecked(motionProperties);
}

void lovrColliderSetMass(Collider* collider, float mass) {
  JPH_MotionProperties* motionProperties = JPH_Body_GetMotionProperties(collider->body);
  Shape* shape = collider->shape;
  JPH_MassProperties* massProperties;
  JPH_Shape_GetMassProperties(shape->shape, massProperties);
  JPH_MassProperties_ScaleToMass(massProperties, mass);
  JPH_MotionProperties_SetMassProperties(motionProperties, JPH_AllowedDOFs_All, massProperties);
}

void lovrColliderGetMassData(Collider* collider, float centerOfMass[3], float* mass, float inertia[6]) {
  //
}

void lovrColliderSetMassData(Collider* collider, float centerOfMass[3], float mass, float inertia[6]) {
  //
}

void lovrColliderGetPosition(Collider* collider, float position[3]) {
  JPH_RVec3 p;
  JPH_Body_GetPosition(collider->body, &p);
  vec3_set(position, p.x, p.y, p.z);
}

void lovrColliderSetPosition(Collider* collider, float position[3]) {
  JPH_RVec3 p = { position[0], position[1], position[2] };
  JPH_BodyInterface_SetPosition(collider->world->bodies, collider->id, &p, JPH_Activation_Activate);
}

void lovrColliderGetOrientation(Collider* collider, float* orientation) {
  JPH_Quat q;
  JPH_Body_GetRotation(collider->body, &q);
  quat_set(orientation, q.x, q.y, q.z, q.w);
}

void lovrColliderSetOrientation(Collider* collider, float* orientation) {
  JPH_Quat q = { orientation[0], orientation[1], orientation[2], orientation[3] };
  JPH_BodyInterface_SetRotation(collider->world->bodies, collider->id, &q, JPH_Activation_Activate);
}

void lovrColliderGetLinearVelocity(Collider* collider, float velocity[3]) {
  JPH_Vec3 v;
  JPH_BodyInterface_GetLinearVelocity(collider->world->bodies, collider->id, &v);
  vec3_set(velocity, v.x, v.y, v.z);
}

void lovrColliderSetLinearVelocity(Collider* collider, float velocity[3]) {
  JPH_Vec3 v = { velocity[0], velocity[1], velocity[2] };
  JPH_BodyInterface_SetLinearVelocity(collider->world->bodies, collider->id, &v);
}

void lovrColliderGetAngularVelocity(Collider* collider, float velocity[3]) {
  JPH_Vec3 v;
  JPH_BodyInterface_GetAngularVelocity(collider->world->bodies, collider->id, &v);
  vec3_set(velocity, v.x, v.y, v.z);
}

void lovrColliderSetAngularVelocity(Collider* collider, float velocity[3]) {
  JPH_Vec3 v = { velocity[0], velocity[1], velocity[2] };
  JPH_BodyInterface_SetAngularVelocity(collider->world->bodies, collider->id, &v);
}

void lovrColliderGetLinearDamping(Collider* collider, float* damping, float* threshold) {
  JPH_MotionProperties* properties = JPH_Body_GetMotionProperties(collider->body);
  *damping = JPH_MotionProperties_GetLinearDamping(properties);
  *threshold = 0.f;
}

void lovrColliderSetLinearDamping(Collider* collider, float damping, float threshold) {
  JPH_MotionProperties* properties = JPH_Body_GetMotionProperties(collider->body);
  JPH_MotionProperties_SetLinearDamping(properties, damping);
  if (threshold != 0.f) {
    lovrLog(LOG_WARN, "PHY", "Jolt does not support velocity threshold parameter for damping");
  }
}

void lovrColliderGetAngularDamping(Collider* collider, float* damping, float* threshold) {
  JPH_MotionProperties* properties = JPH_Body_GetMotionProperties(collider->body);
  *damping = JPH_MotionProperties_GetAngularDamping(properties);
  *threshold = 0.f;
}

void lovrColliderSetAngularDamping(Collider* collider, float damping, float threshold) {
  JPH_MotionProperties* properties = JPH_Body_GetMotionProperties(collider->body);
  JPH_MotionProperties_SetAngularDamping(properties, damping);
  if (threshold != 0.f) {
    lovrLog(LOG_WARN, "PHY", "Jolt does not support velocity threshold parameter for damping");
  }
}

void lovrColliderApplyForce(Collider* collider, float force[3]) {
  JPH_Vec3 f = { force[0], force[1], force[2] };
  JPH_BodyInterface_AddForce(collider->world->bodies, collider->id, &f);
}

void lovrColliderApplyForceAtPosition(Collider* collider, float force[3], float position[3]) {
  JPH_Vec3 f = { force[0], force[1], force[2] };
  JPH_RVec3 p = { position[0], position[1], position[2] };
  JPH_BodyInterface_AddForce2(collider->world->bodies, collider->id, &f, &p);
}

void lovrColliderApplyTorque(Collider* collider, float torque[3]) {
  JPH_Vec3 t = { torque[0], torque[1], torque[2] };
  JPH_BodyInterface_AddTorque(collider->world->bodies, collider->id, &t);
}

void lovrColliderApplyLinearImpulse(Collider* collider, float impulse[3]) {
  JPH_Vec3 v = { impulse[0], impulse[1], impulse[2] };
  JPH_BodyInterface_AddImpulse(collider->world->bodies, collider->id, &v);
}

void lovrColliderApplyLinearImpulseAtPosition(Collider* collider, float impulse[3], float position[3]) {
  JPH_Vec3 v = { impulse[0], impulse[1], impulse[2] };
  JPH_Vec3 p = { position[0], position[1], position[2] };
  JPH_BodyInterface_AddImpulse2(collider->world->bodies, collider->id, &v, &p);
}

void lovrColliderApplyAngularImpulse(Collider* collider, float impulse[3]) {
  JPH_Vec3 v = { impulse[0], impulse[1], impulse[2] };
  JPH_BodyInterface_AddAngularImpulse(collider->world->bodies, collider->id, &v);
}

void lovrColliderGetLocalCenter(Collider* collider, float center[3]) {
  // todo: applicable for CompoundShape and OffsetCenterOfMassShape
  vec3_set(center, 0.f, 0.f, 0.f);
}

void lovrColliderGetLocalPoint(Collider* collider, float world[3], float local[3]) {
  JPH_RMatrix4x4 transform;
  JPH_Body_GetWorldTransform(collider->body, &transform);
  vec3_init(local, world);
  mat4_invert(&transform.m11);
  mat4_mulPoint(&transform.m11, local);
}

void lovrColliderGetWorldPoint(Collider* collider, float local[3], float world[3]) {
  JPH_RMatrix4x4 transform;
  JPH_Body_GetWorldTransform(collider->body, &transform);
  vec3_init(world, local);
  mat4_mulPoint(&transform.m11, world);
}

void lovrColliderGetLocalVector(Collider* collider, float world[3], float local[3]) {
  JPH_RMatrix4x4 transform;
  JPH_Body_GetWorldTransform(collider->body, &transform);
  vec3_init(local, world);
  mat4_invert(&transform.m11);
  mat4_mulDirection(&transform.m11, local);
}

void lovrColliderGetWorldVector(Collider* collider, float local[3], float world[3]) {
  JPH_RMatrix4x4 transform;
  JPH_Body_GetWorldTransform(collider->body, &transform);
  vec3_init(world, local);
  mat4_mulDirection(&transform.m11, world);
}

void lovrColliderGetLinearVelocityFromLocalPoint(Collider* collider, float point[3], float velocity[3]) {
  float wx, wy, wz;
  lovrColliderGetWorldPoint(collider, point, velocity);
  lovrColliderGetLinearVelocityFromWorldPoint(collider, point, velocity);
}

void lovrColliderGetLinearVelocityFromWorldPoint(Collider* collider, float point[3], float velocity[3]) {
  JPH_RVec3 p = { point[0], point[1], point[2] };
  JPH_Vec3 v;
  JPH_BodyInterface_GetPointVelocity(collider->world->bodies, collider->id, &p, &v);
  vec3_set(velocity, v.x, v.y, v.z);
}

void lovrColliderGetAABB(Collider* collider, float aabb[6]) {
  JPH_AABox box;
  JPH_Body_GetWorldSpaceBounds(collider->body, &box);
  aabb[0] = box.min.x;
  aabb[1] = box.max.x;
  aabb[2] = box.min.y;
  aabb[3] = box.max.y;
  aabb[4] = box.min.z;
  aabb[5] = box.max.z;
}

// Shapes

void lovrShapeDestroy(void* ref) {
  Shape* shape = ref;
  lovrShapeDestroyData(shape);
  lovrFree(shape);
}

void lovrShapeDestroyData(Shape* shape) {
  if (shape->shape) {
    if (shape->type == SHAPE_COMPOUND) {
      uint32_t count = lovrCompoundShapeGetChildCount(shape);
      for (uint32_t i = 0; i < count; i++) {
        Shape* child = lovrCompoundShapeGetChild(shape, i);
        lovrRelease(child, lovrShapeDestroy);
      }
    }

    JPH_Shape_Destroy(shape->shape);
    shape->shape = NULL;
  }
}

ShapeType lovrShapeGetType(Shape* shape) {
  return shape->type;
}

void lovrShapeGetMass(Shape* shape, float density, float centerOfMass[3], float* mass, float inertia[6]) {
  //
}

void lovrShapeGetAABB(Shape* shape, float position[3], float orientation[4], float aabb[6]) {
  JPH_AABox box;
  if (!position && !orientation) {
    JPH_Shape_GetLocalBounds(shape->shape, &box);
  } else {
    JPH_RMatrix4x4 transform;
    JPH_Vec3 scale = { 1.f, 1.f, 1.f };
    mat4_fromPose(&transform.m11, position, orientation);
    JPH_Shape_GetWorldSpaceBounds(shape->shape, &transform, &scale, &box);
  }
  aabb[0] = box.min.x;
  aabb[1] = box.max.x;
  aabb[2] = box.min.y;
  aabb[3] = box.max.y;
  aabb[4] = box.min.z;
  aabb[5] = box.max.z;
}

SphereShape* lovrSphereShapeCreate(float radius) {
  lovrCheck(radius > 0.f, "SphereShape radius must be positive");
  SphereShape* sphere = lovrCalloc(sizeof(SphereShape));
  sphere->ref = 1;
  sphere->type = SHAPE_SPHERE;
  sphere->shape = (JPH_Shape*) JPH_SphereShape_Create(radius);
  JPH_Shape_SetUserData(sphere->shape, (uint64_t) (uintptr_t) sphere);
  return sphere;
}

float lovrSphereShapeGetRadius(SphereShape* sphere) {
  return JPH_SphereShape_GetRadius((JPH_SphereShape*) sphere->shape);
}

BoxShape* lovrBoxShapeCreate(float dimensions[3]) {
  BoxShape* box = lovrCalloc(sizeof(BoxShape));
  box->ref = 1;
  box->type = SHAPE_BOX;
  const JPH_Vec3 halfExtent = { dimensions[0] / 2.f, dimensions[1] / 2.f, dimensions[2] / 2.f };
  box->shape = (JPH_Shape*) JPH_BoxShape_Create(&halfExtent, 0.f);
  JPH_Shape_SetUserData(box->shape, (uint64_t) (uintptr_t) box);
  return box;
}

void lovrBoxShapeGetDimensions(BoxShape* box, float dimensions[3]) {
  JPH_Vec3 halfExtent;
  JPH_BoxShape_GetHalfExtent((JPH_BoxShape*) box->shape, &halfExtent);
  vec3_set(dimensions, halfExtent.x * 2.f, halfExtent.y * 2.f, halfExtent.z * 2.f);
}

CapsuleShape* lovrCapsuleShapeCreate(float radius, float length) {
  lovrCheck(radius > 0.f && length > 0.f, "CapsuleShape dimensions must be positive");
  CapsuleShape* capsule = lovrCalloc(sizeof(CapsuleShape));
  capsule->ref = 1;
  capsule->type = SHAPE_CAPSULE;
  capsule->shape = (JPH_Shape*) JPH_CapsuleShape_Create(length / 2, radius);
  JPH_Shape_SetUserData(capsule->shape, (uint64_t) (uintptr_t) capsule);
  return capsule;
}

float lovrCapsuleShapeGetRadius(CapsuleShape* capsule) {
  return JPH_CapsuleShape_GetRadius((JPH_CapsuleShape*) capsule->shape);
}

float lovrCapsuleShapeGetLength(CapsuleShape* capsule) {
  return 2.f * JPH_CapsuleShape_GetHalfHeightOfCylinder((JPH_CapsuleShape*) capsule->shape);
}

CylinderShape* lovrCylinderShapeCreate(float radius, float length) {
  lovrCheck(radius > 0.f && length > 0.f, "CylinderShape dimensions must be positive");
  CylinderShape* cylinder = lovrCalloc(sizeof(CylinderShape));
  cylinder->ref = 1;
  cylinder->type = SHAPE_CYLINDER;
  cylinder->shape = (JPH_Shape*) JPH_CylinderShape_Create(length / 2.f, radius);
  JPH_Shape_SetUserData(cylinder->shape, (uint64_t) (uintptr_t) cylinder);
  return cylinder;
}

float lovrCylinderShapeGetRadius(CylinderShape* cylinder) {
  return JPH_CylinderShape_GetRadius((JPH_CylinderShape*) cylinder->shape);
}

float lovrCylinderShapeGetLength(CylinderShape* cylinder) {
  return JPH_CylinderShape_GetHalfHeight((JPH_CylinderShape*) cylinder->shape) * 2.f;
}

ConvexShape* lovrConvexShapeCreate(float points[], uint32_t count) {
  ConvexShape* convex = lovrCalloc(sizeof(ConvexShape));
  convex->ref = 1;
  convex->type = SHAPE_CONVEX;
  JPH_ConvexHullShapeSettings* settings = JPH_ConvexHullShapeSettings_Create((const JPH_Vec3*) points, count, .05f);
  convex->shape = (JPH_Shape*) JPH_ConvexHullShapeSettings_CreateShape(settings);
  JPH_ShapeSettings_Destroy((JPH_ShapeSettings*) settings);
  return convex;
}

MeshShape* lovrMeshShapeCreate(int vertexCount, float vertices[], int indexCount, uint32_t indices[]) {
  MeshShape* mesh = lovrCalloc(sizeof(MeshShape));
  mesh->ref = 1;
  mesh->type = SHAPE_MESH;

  int triangleCount = indexCount / 3;
  JPH_IndexedTriangle* indexedTriangles = lovrMalloc(triangleCount * sizeof(JPH_IndexedTriangle));
  for (int i = 0; i < triangleCount; i++) {
    indexedTriangles[i].i1 = indices[i * 3 + 0];
    indexedTriangles[i].i2 = indices[i * 3 + 1];
    indexedTriangles[i].i3 = indices[i * 3 + 2];
    indexedTriangles[i].materialIndex = 0;
  }
  JPH_MeshShapeSettings* shape_settings = JPH_MeshShapeSettings_Create2(
    (const JPH_Vec3*) vertices,
    vertexCount,
    indexedTriangles,
    triangleCount);
  mesh->shape = (JPH_Shape*) JPH_MeshShapeSettings_CreateShape(shape_settings);
  JPH_ShapeSettings_Destroy((JPH_ShapeSettings*) shape_settings);
  lovrFree(indexedTriangles);
  // Note that we're responsible for freeing the vertices/indices when we're done with them because
  // ODE took ownership of mesh data.  If ODE gets removed, we should probably get rid of this and
  // have the caller free the vertices/indices themselves.
  lovrFree(vertices);
  lovrFree(indices);
  return mesh;
}

TerrainShape* lovrTerrainShapeCreate(float* vertices, uint32_t n, float scaleXZ, float scaleY) {
  TerrainShape* terrain = lovrCalloc(sizeof(TerrainShape));
  terrain->ref = 1;
  terrain->type = SHAPE_TERRAIN;
  const JPH_Vec3 offset = {
    .x = -.5f * scaleXZ,
    .y = 0.f,
    .z = -.5f * scaleXZ
  };
  const JPH_Vec3 scale = {
    .x = scaleXZ / n,
    .y = scaleY,
    .z = scaleXZ / n
  };

  JPH_HeightFieldShapeSettings* shape_settings = JPH_HeightFieldShapeSettings_Create(vertices, &offset, &scale, n);
  terrain->shape = (JPH_Shape*) JPH_HeightFieldShapeSettings_CreateShape(shape_settings);
  JPH_ShapeSettings_Destroy((JPH_ShapeSettings*) shape_settings);
  return terrain;
}

CompoundShape* lovrCompoundShapeCreate(Shape** shapes, vec3 positions, quat orientations, uint32_t count, bool freeze) {
  lovrCheck(!freeze || count >= 2, "A frozen CompoundShape must contain at least two shapes");

  CompoundShape* shape = lovrCalloc(sizeof(CompoundShape));
  shape->ref = 1;
  shape->type = SHAPE_COMPOUND;

  JPH_CompoundShapeSettings* settings = freeze ?
    (JPH_CompoundShapeSettings*) JPH_StaticCompoundShapeSettings_Create() :
    (JPH_CompoundShapeSettings*) JPH_MutableCompoundShapeSettings_Create();

  for (uint32_t i = 0; i < count; i++) {
    lovrCheck(shapes[i]->type != SHAPE_COMPOUND, "Currently, nesting compound shapes is not supported");
    JPH_Vec3 position = { positions[3 * i + 0], positions[3 * i + 1], positions[3 * i + 2] };
    JPH_Quat rotation = { orientations[4 * i + 0], orientations[4 * i + 1], orientations[4 * i + 2], orientations[4 * i + 3] };
    JPH_CompoundShapeSettings_AddShape2(settings, &position, &rotation, shapes[i]->shape, 0);
    lovrRetain(shapes[i]);
  }

  if (freeze) {
    shape->shape = (JPH_Shape*) JPH_StaticCompoundShape_Create((JPH_StaticCompoundShapeSettings*) settings);
  } else {
    shape->shape = (JPH_Shape*) JPH_MutableCompoundShape_Create((JPH_MutableCompoundShapeSettings*) settings);
  }

  JPH_ShapeSettings_Destroy((JPH_ShapeSettings*) settings);
  return shape;
}

bool lovrCompoundShapeIsFrozen(CompoundShape* shape) {
  return JPH_Shape_GetSubType(shape->shape) == JPH_ShapeSubType_StaticCompound;
}

void lovrCompoundShapeAddChild(CompoundShape* shape, Shape* child, float* position, float* orientation) {
  lovrCheck(!lovrCompoundShapeIsFrozen(shape), "CompoundShape is frozen and can not be changed");
  lovrCheck(child->type != SHAPE_COMPOUND, "Currently, nesting compound shapes is not supported");
  JPH_Vec3 pos = { position[0], position[1], position[2] };
  JPH_Quat rot = { orientation[0], orientation[1], orientation[2], orientation[3] };
  JPH_MutableCompoundShape_AddShape((JPH_MutableCompoundShape*) shape->shape, &pos, &rot, child->shape, 0);
  lovrRetain(child);
}

void lovrCompoundShapeReplaceChild(CompoundShape* shape, uint32_t index, Shape* child, float* position, float* orientation) {
  lovrCheck(!lovrCompoundShapeIsFrozen(shape), "CompoundShape is frozen and can not be changed");
  lovrCheck(child->type != SHAPE_COMPOUND, "Currently, nesting compound shapes is not supported");
  lovrCheck(index < lovrCompoundShapeGetChildCount(shape), "CompoundShape has no child at index %d", index + 1);
  JPH_Vec3 pos = { position[0], position[1], position[2] };
  JPH_Quat rot = { orientation[0], orientation[1], orientation[2], orientation[3] };
  lovrRelease(lovrCompoundShapeGetChild(shape, index), lovrShapeDestroy);
  JPH_MutableCompoundShape_ModifyShape2((JPH_MutableCompoundShape*) shape->shape, index, &pos, &rot, child->shape);
  lovrRetain(child);
}

void lovrCompoundShapeRemoveChild(CompoundShape* shape, uint32_t index) {
  lovrCheck(!lovrCompoundShapeIsFrozen(shape), "CompoundShape is frozen and can not be changed");
  lovrCheck(index < lovrCompoundShapeGetChildCount(shape), "CompoundShape has no child at index %d", index + 1);
  Shape* child = lovrCompoundShapeGetChild(shape, index);
  JPH_MutableCompoundShape_RemoveShape((JPH_MutableCompoundShape*) shape->shape, index);
  lovrRelease(child, lovrShapeDestroy);
}

Shape* lovrCompoundShapeGetChild(CompoundShape* shape, uint32_t index) {
  if (index < lovrCompoundShapeGetChildCount(shape)) {
    const JPH_Shape* child;
    JPH_CompoundShape_GetSubShape((JPH_CompoundShape*) shape->shape, index, &child, NULL, NULL, NULL);
    return (Shape*) (uintptr_t) JPH_Shape_GetUserData(child);
  } else {
    return NULL;
  }
}

uint32_t lovrCompoundShapeGetChildCount(CompoundShape* shape) {
  return JPH_CompoundShape_GetNumSubShapes((JPH_CompoundShape*) shape->shape);
}

void lovrCompoundShapeGetChildOffset(CompoundShape* shape, uint32_t index, float position[3], float orientation[4]) {
  lovrCheck(index < lovrCompoundShapeGetChildCount(shape), "CompoundShape has no child at index %d", index + 1);
  const JPH_Shape* child;
  JPH_Vec3 pos;
  JPH_Quat rot;
  uint32_t userData;
  JPH_CompoundShape_GetSubShape((JPH_CompoundShape*) shape->shape, index, &child, &pos, &rot, &userData);
  vec3_init(position, &pos.x);
  quat_init(orientation, &rot.x);
}

void lovrCompoundShapeSetChildOffset(CompoundShape* shape, uint32_t index, float position[3], float orientation[4]) {
  lovrCheck(!lovrCompoundShapeIsFrozen(shape), "CompoundShape is frozen and can not be changed");
  lovrCheck(index < lovrCompoundShapeGetChildCount(shape), "CompoundShape has no child at index %d", index + 1);
  JPH_Vec3 pos = { position[0], position[1], position[2] };
  JPH_Quat rot = { orientation[0], orientation[1], orientation[2], orientation[3] };
  JPH_MutableCompoundShape_ModifyShape((JPH_MutableCompoundShape*) shape->shape, index, &pos, &rot);
}

// Joints

static void lovrJointGetAnchors(Joint* joint, float anchor1[3], float anchor2[3]) {
  JPH_Body* body1 = JPH_TwoBodyConstraint_GetBody1((JPH_TwoBodyConstraint*) joint->constraint);
  JPH_Body* body2 = JPH_TwoBodyConstraint_GetBody2((JPH_TwoBodyConstraint*) joint->constraint);
  JPH_RMatrix4x4 centerOfMassTransform1;
  JPH_RMatrix4x4 centerOfMassTransform2;
  JPH_Body_GetCenterOfMassTransform(body1, &centerOfMassTransform1);
  JPH_Body_GetCenterOfMassTransform(body2, &centerOfMassTransform2);
  JPH_Matrix4x4 constraintToBody1;
  JPH_Matrix4x4 constraintToBody2;
  JPH_TwoBodyConstraint_GetConstraintToBody1Matrix((JPH_TwoBodyConstraint*) joint->constraint, &constraintToBody1);
  JPH_TwoBodyConstraint_GetConstraintToBody2Matrix((JPH_TwoBodyConstraint*) joint->constraint, &constraintToBody2);
  mat4_mulVec4(&centerOfMassTransform1.m11, &constraintToBody1.m41);
  mat4_mulVec4(&centerOfMassTransform2.m11, &constraintToBody2.m41);
  anchor1[0] = constraintToBody1.m41;
  anchor1[1] = constraintToBody1.m42;
  anchor1[2] = constraintToBody1.m43;
  anchor2[0] = constraintToBody2.m41;
  anchor2[1] = constraintToBody2.m42;
  anchor2[2] = constraintToBody2.m43;
}

static JointNode* lovrJointGetNode(Joint* joint, Collider* collider) {
  return collider == lovrJointGetColliderA(joint) ? &joint->a : &joint->b;
}

void lovrJointInit(Joint* joint, Collider* a, Collider* b) {
  World* world = a->world;

  if (a->joints) {
    joint->a.next = a->joints;
    lovrJointGetNode(a->joints, a)->prev = joint;
  }

  a->joints = joint;

  if (b->joints) {
    joint->b.next = b->joints;
    lovrJointGetNode(b->joints, b)->prev = joint;
  }

  b->joints = joint;

  if (world->joints) {
    joint->world.next = world->joints;
    world->joints->world.prev = joint;
  }

  world->joints = joint;
  world->jointCount++;
}

void lovrJointDestroy(void* ref) {
  Joint* joint = ref;
  lovrJointDestroyData(joint);
  lovrFree(joint);
}

void lovrJointDestroyData(Joint* joint) {
  if (!joint->constraint) {
    return;
  }

  JPH_TwoBodyConstraint* constraint = (JPH_TwoBodyConstraint*) joint->constraint;
  Collider* a = (Collider*) (uintptr_t) JPH_Body_GetUserData(JPH_TwoBodyConstraint_GetBody1(constraint));
  Collider* b = (Collider*) (uintptr_t) JPH_Body_GetUserData(JPH_TwoBodyConstraint_GetBody2(constraint));
  World* world = a->world;
  JointNode* node;

  node = &joint->a;
  if (node->next) lovrJointGetNode(node->next, a)->prev = node->prev;
  if (node->prev) lovrJointGetNode(node->prev, a)->next = node->next;
  else a->joints = node->next;

  node = &joint->b;
  if (node->next) lovrJointGetNode(node->next, b)->prev = node->prev;
  if (node->prev) lovrJointGetNode(node->prev, b)->next = node->next;
  else b->joints = node->next;

  node = &joint->world;
  if (node->next) node->next->world.prev = node->prev;
  if (node->prev) node->prev->world.next = node->next;
  else world->joints = joint->world.next;

  JPH_PhysicsSystem_RemoveConstraint(world->system, joint->constraint);
  JPH_Constraint_Destroy(joint->constraint);
  joint->constraint = NULL;
  world->jointCount--;

  lovrRelease(joint, lovrJointDestroy);
}

bool lovrJointIsDestroyed(Joint* joint) {
  return !joint->constraint;
}

JointType lovrJointGetType(Joint* joint) {
  return joint->type;
}

Collider* lovrJointGetColliderA(Joint* joint) {
  JPH_TwoBodyConstraint* constraint = (JPH_TwoBodyConstraint*) joint->constraint;
  return (Collider*) (uintptr_t) JPH_Body_GetUserData(JPH_TwoBodyConstraint_GetBody1(constraint));
}

Collider* lovrJointGetColliderB(Joint* joint) {
  JPH_TwoBodyConstraint* constraint = (JPH_TwoBodyConstraint*) joint->constraint;
  return (Collider*) (uintptr_t) JPH_Body_GetUserData(JPH_TwoBodyConstraint_GetBody2(constraint));
}

Joint* lovrJointGetNext(Joint* joint, Collider* collider) {
  return lovrJointGetNode(joint, collider)->next;
}

bool lovrJointIsEnabled(Joint* joint) {
  return JPH_Constraint_GetEnabled(joint->constraint);
}

void lovrJointSetEnabled(Joint* joint, bool enable) {
  JPH_Constraint_SetEnabled(joint->constraint, enable);
}

BallJoint* lovrBallJointCreate(Collider* a, Collider* b, float anchor[3]) {
  lovrCheck(a->world == b->world, "Joint bodies must exist in same World");
  BallJoint* joint = lovrCalloc(sizeof(BallJoint));
  joint->ref = 1;
  joint->type = JOINT_BALL;

  JPH_PointConstraintSettings* settings = JPH_PointConstraintSettings_Create();
  JPH_RVec3 point1 = { anchor[0], anchor[1], anchor[2] };
  JPH_RVec3 point2 = { anchor[0], anchor[1], anchor[2] };
  JPH_PointConstraintSettings_SetPoint1(settings, &point1);
  JPH_PointConstraintSettings_SetPoint2(settings, &point2);
  joint->constraint = (JPH_Constraint*) JPH_PointConstraintSettings_CreateConstraint(settings, a->body, b->body);
  JPH_ConstraintSettings_Destroy((JPH_ConstraintSettings*) settings);
  JPH_PhysicsSystem_AddConstraint(a->world->system, joint->constraint);
  lovrJointInit(joint, a, b);
  lovrRetain(joint);
  return joint;
}

void lovrBallJointGetAnchors(BallJoint* joint, float anchor1[3], float anchor2[3]) {
  lovrJointGetAnchors((Joint*) joint, anchor1, anchor2);
}

void lovrBallJointSetAnchor(BallJoint* joint, float anchor[3]) {
  JPH_RVec3 point;
  point.x = anchor[0];
  point.y = anchor[1];
  point.z = anchor[2];
  JPH_PointConstraint_SetPoint1((JPH_PointConstraint*) joint->constraint, JPH_ConstraintSpace_WorldSpace, &point);
  JPH_PointConstraint_SetPoint2((JPH_PointConstraint*) joint->constraint, JPH_ConstraintSpace_WorldSpace, &point);
}

float lovrBallJointGetResponseTime(Joint* joint) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support BallJoint response time");
}

void lovrBallJointSetResponseTime(Joint* joint, float responseTime) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support BallJoint response time");
}

float lovrBallJointGetTightness(Joint* joint) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support BallJoint tightness");
}

void lovrBallJointSetTightness(Joint* joint, float tightness) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support BallJoint tightness");
}

DistanceJoint* lovrDistanceJointCreate(Collider* a, Collider* b, float anchor1[3], float anchor2[3]) {
  lovrCheck(a->world == b->world, "Joint bodies must exist in same World");
  DistanceJoint* joint = lovrCalloc(sizeof(DistanceJoint));
  joint->ref = 1;
  joint->type = JOINT_DISTANCE;

  JPH_DistanceConstraintSettings* settings = JPH_DistanceConstraintSettings_Create();
  JPH_RVec3 point1 = { anchor1[0], anchor1[1], anchor1[2] };
  JPH_RVec3 point2 = { anchor2[0], anchor2[1], anchor2[2] };
  JPH_DistanceConstraintSettings_SetPoint1(settings, &point1);
  JPH_DistanceConstraintSettings_SetPoint2(settings, &point2);
  joint->constraint = (JPH_Constraint*) JPH_DistanceConstraintSettings_CreateConstraint(settings, a->body, b->body);
  JPH_ConstraintSettings_Destroy((JPH_ConstraintSettings*) settings);
  JPH_PhysicsSystem_AddConstraint(a->world->system, joint->constraint);
  lovrJointInit(joint, a, b);
  lovrRetain(joint);
  return joint;
}

void lovrDistanceJointGetAnchors(DistanceJoint* joint, float anchor1[3], float anchor2[3]) {
  lovrJointGetAnchors((Joint*) joint, anchor1, anchor2);
}

void lovrDistanceJointSetAnchors(DistanceJoint* joint, float anchor1[3], float anchor2[3]) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support modifying joint anchors after creation");
  // todo: no setter available, but the constraint could be removed and re-added
}

float lovrDistanceJointGetDistance(DistanceJoint* joint) {
  return JPH_DistanceConstraint_GetMaxDistance((JPH_DistanceConstraint*) joint->constraint);
}

void lovrDistanceJointSetDistance(DistanceJoint* joint, float distance) {
  JPH_DistanceConstraint_SetDistance((JPH_DistanceConstraint*) joint->constraint, distance, distance);
}

float lovrDistanceJointGetResponseTime(Joint* joint) {
  JPH_SpringSettings* settings = JPH_DistanceConstraint_GetLimitsSpringSettings((JPH_DistanceConstraint*) joint->constraint);
  return 1.f / JPH_SpringSettings_GetFrequency(settings);
}

void lovrDistanceJointSetResponseTime(Joint* joint, float responseTime) {
  JPH_SpringSettings* settings = JPH_SpringSettings_Create(1.f / responseTime, 0.f);
  JPH_DistanceConstraint_SetLimitsSpringSettings((JPH_DistanceConstraint*) joint->constraint, settings);
  JPH_SpringSettings_Destroy(settings);
}

float lovrDistanceJointGetTightness(Joint* joint) {
  // todo: jolt has spring damping instead of tightness, not compatible with lovr API
  // (but body's damping is not that different)
  lovrLog(LOG_WARN, "PHY", "Jolt does not support DistanceJoint tightness");
  return 0.f;
}

void lovrDistanceJointSetTightness(Joint* joint, float tightness) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support DistanceJoint tightness");
}

HingeJoint* lovrHingeJointCreate(Collider* a, Collider* b, float anchor[3], float axis[3]) {
  lovrCheck(a->world == b->world, "Joint bodies must exist in the same World");

  HingeJoint* joint = lovrCalloc(sizeof(HingeJoint));
  joint->ref = 1;
  joint->type = JOINT_HINGE;

  JPH_HingeConstraintSettings* settings = JPH_HingeConstraintSettings_Create();

  JPH_RVec3 point = { anchor[0], anchor[1], anchor[2] };
  JPH_Vec3 axisVec = { axis[0], axis[1], axis[2] };
  JPH_HingeConstraintSettings_SetPoint1(settings, &point);
  JPH_HingeConstraintSettings_SetPoint2(settings, &point);
  JPH_HingeConstraintSettings_SetHingeAxis1(settings, &axisVec);
  JPH_HingeConstraintSettings_SetHingeAxis2(settings, &axisVec);
  joint->constraint = (JPH_Constraint*) JPH_HingeConstraintSettings_CreateConstraint(settings, a->body, b->body);
  JPH_ConstraintSettings_Destroy((JPH_ConstraintSettings*) settings);
  JPH_PhysicsSystem_AddConstraint(a->world->system, joint->constraint);
  lovrJointInit(joint, a, b);
  lovrRetain(joint);
  return joint;
}

void lovrHingeJointGetAnchors(HingeJoint* joint, float anchor1[3], float anchor2[3]) {
  lovrJointGetAnchors(joint, anchor1, anchor2);
}

void lovrHingeJointSetAnchor(HingeJoint* joint, float anchor[3]) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support modifying joint anchors after creation");
  // todo: no setter available, but the constraint could be removed and re-added
}

void lovrHingeJointGetAxis(HingeJoint* joint, float axis[3]) {
  JPH_Vec3 resultAxis;
  JPH_HingeConstraintSettings* settings = JPH_HingeConstraint_GetSettings((JPH_HingeConstraint*) joint->constraint);
  JPH_HingeConstraintSettings_GetHingeAxis1(settings, &resultAxis);
  JPH_Body* body1 = JPH_TwoBodyConstraint_GetBody1((JPH_TwoBodyConstraint*) joint->constraint);
  JPH_RMatrix4x4 centerOfMassTransform;
  JPH_Body_GetCenterOfMassTransform(body1, &centerOfMassTransform);
  JPH_Matrix4x4 constraintToBody;
  JPH_TwoBodyConstraint_GetConstraintToBody1Matrix((JPH_TwoBodyConstraint*) joint->constraint, &constraintToBody);
  float translation[4] = {
    resultAxis.x,
    resultAxis.y,
    resultAxis.z,
    0.f
  };
  mat4_mulVec4(&centerOfMassTransform.m11, translation);
  axis[0] = translation[0];
  axis[1] = translation[1];
  axis[2] = translation[2];
}

void lovrHingeJointSetAxis(HingeJoint* joint, float axis[3]) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support modifying joint axis after creation");
  // todo: no setter available, but the constraint could be removed and re-added
}


float lovrHingeJointGetAngle(HingeJoint* joint) {
  return -JPH_HingeConstraint_GetCurrentAngle((JPH_HingeConstraint*) joint->constraint);
}

float lovrHingeJointGetLowerLimit(HingeJoint* joint) {
  return JPH_HingeConstraint_GetLimitsMin((JPH_HingeConstraint*) joint->constraint);
}

void lovrHingeJointSetLowerLimit(HingeJoint* joint, float limit) {
  float upper_limit = JPH_HingeConstraint_GetLimitsMax((JPH_HingeConstraint*) joint->constraint);
  JPH_HingeConstraint_SetLimits((JPH_HingeConstraint*) joint->constraint, limit, upper_limit);
}

float lovrHingeJointGetUpperLimit(HingeJoint* joint) {
  return JPH_HingeConstraint_GetLimitsMax((JPH_HingeConstraint*) joint->constraint);
}

void lovrHingeJointSetUpperLimit(HingeJoint* joint, float limit) {
  float lower_limit = JPH_HingeConstraint_GetLimitsMin((JPH_HingeConstraint*) joint->constraint);
  JPH_HingeConstraint_SetLimits((JPH_HingeConstraint*) joint->constraint, lower_limit, limit);
}

SliderJoint* lovrSliderJointCreate(Collider* a, Collider* b, float axis[3]) {
  lovrCheck(a->world == b->world, "Joint bodies must exist in the same World");

  SliderJoint* joint = lovrCalloc(sizeof(SliderJoint));
  joint->ref = 1;
  joint->type = JOINT_SLIDER;

  JPH_SliderConstraintSettings* settings = JPH_SliderConstraintSettings_Create();
  const JPH_Vec3 axisVec = { axis[0], axis[1], axis[2] };
  JPH_SliderConstraintSettings_SetSliderAxis(settings, &axisVec);
  joint->constraint = (JPH_Constraint*) JPH_SliderConstraintSettings_CreateConstraint(settings, a->body, b->body);
  JPH_ConstraintSettings_Destroy((JPH_ConstraintSettings*) settings);
  JPH_PhysicsSystem_AddConstraint(a->world->system, joint->constraint);
  lovrJointInit(joint, a, b);
  lovrRetain(joint);
  return joint;
}

void lovrSliderJointGetAxis(SliderJoint* joint, float axis[3]) {
  JPH_Vec3 resultAxis;
  JPH_SliderConstraintSettings* settings = JPH_SliderConstraint_GetSettings((JPH_SliderConstraint*) joint->constraint);
  JPH_SliderConstraintSettings_GetSliderAxis(settings, &resultAxis);
  JPH_Body* body1 = JPH_TwoBodyConstraint_GetBody1((JPH_TwoBodyConstraint*) joint->constraint);
  JPH_RMatrix4x4 centerOfMassTransform;
  JPH_Body_GetCenterOfMassTransform(body1, &centerOfMassTransform);
  JPH_Matrix4x4 constraintToBody;
  JPH_TwoBodyConstraint_GetConstraintToBody1Matrix((JPH_TwoBodyConstraint*) joint->constraint, &constraintToBody);
  float translation[4] = {
    resultAxis.x,
    resultAxis.y,
    resultAxis.z,
    0.f
  };
  mat4_mulVec4(&centerOfMassTransform.m11, translation);
  axis[0] = translation[0];
  axis[1] = translation[1];
  axis[2] = translation[2];
}

void lovrSliderJointSetAxis(SliderJoint* joint, float axis[3]) {
  lovrLog(LOG_WARN, "PHY", "Jolt does not support modifying joint axis after creation");
  // todo: no setter available, but the constraint could be removed and re-added
}

float lovrSliderJointGetPosition(SliderJoint* joint) {
  return JPH_SliderConstraint_GetCurrentPosition((JPH_SliderConstraint*) joint->constraint);
}

float lovrSliderJointGetLowerLimit(SliderJoint* joint) {
  return JPH_SliderConstraint_GetLimitsMin((JPH_SliderConstraint*) joint->constraint);
}

void lovrSliderJointSetLowerLimit(SliderJoint* joint, float limit) {
  float upper_limit = JPH_SliderConstraint_GetLimitsMax((JPH_SliderConstraint*) joint->constraint);
  JPH_SliderConstraint_SetLimits((JPH_SliderConstraint*) joint->constraint, limit, upper_limit);
}

float lovrSliderJointGetUpperLimit(SliderJoint* joint) {
  return JPH_SliderConstraint_GetLimitsMax((JPH_SliderConstraint*) joint->constraint);
}

void lovrSliderJointSetUpperLimit(SliderJoint* joint, float limit) {
  float lower_limit = JPH_SliderConstraint_GetLimitsMin((JPH_SliderConstraint*) joint->constraint);
  JPH_SliderConstraint_SetLimits((JPH_SliderConstraint*) joint->constraint, lower_limit, limit);
}
