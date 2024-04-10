#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#pragma once

#define MAX_CONTACTS 10
#define MAX_TAGS 32
#define NO_TAG ~0u

typedef struct World World;
typedef struct Collider Collider;
typedef struct Shape Shape;
typedef struct Joint Joint;

typedef Shape SphereShape;
typedef Shape BoxShape;
typedef Shape CapsuleShape;
typedef Shape CylinderShape;
typedef Shape ConvexShape;
typedef Shape MeshShape;
typedef Shape TerrainShape;
typedef Shape CompoundShape;

typedef Joint BallJoint;
typedef Joint DistanceJoint;
typedef Joint HingeJoint;
typedef Joint SliderJoint;

typedef void (*CollisionResolver)(World* world, void* userdata);
typedef bool (*RaycastCallback)(Collider* collider, float position[3], float normal[3], uint32_t child, void* userdata);
typedef bool (*QueryCallback)(Collider* collider, uint32_t child, void* userdata);

bool lovrPhysicsInit(void);
void lovrPhysicsDestroy(void);

// World

typedef struct {
  float x, y, z;
  float nx, ny, nz;
  float depth;
} Contact;

typedef struct {
  uint32_t maxColliders;
  uint32_t maxColliderPairs;
  uint32_t maxContacts;
  bool allowSleep;
  const char* tags[MAX_TAGS];
  uint32_t tagCount;
} WorldInfo;

World* lovrWorldCreate(WorldInfo* info);
void lovrWorldDestroy(void* ref);
void lovrWorldDestroyData(World* world);
uint32_t lovrWorldGetColliderCount(World* world);
uint32_t lovrWorldGetJointCount(World* world);
Collider* lovrWorldGetColliders(World* world, Collider* collider);
Joint* lovrWorldGetJoints(World* world, Joint* joint);
void lovrWorldUpdate(World* world, float dt, CollisionResolver resolver, void* userdata);
void lovrWorldComputeOverlaps(World* world);
int lovrWorldGetNextOverlap(World* world, Shape** a, Shape** b);
int lovrWorldCollide(World* world, Shape* a, Shape* b, float friction, float restitution);
void lovrWorldGetContacts(World* world, Shape* a, Shape* b, Contact contacts[MAX_CONTACTS], uint32_t* count);
void lovrWorldRaycast(World* world, float start[3], float end[3], const char* tag, RaycastCallback callback, void* userdata);
bool lovrWorldQueryBox(World* world, float position[3], float size[3], const char* tag, QueryCallback callback, void* userdata);
bool lovrWorldQuerySphere(World* world, float position[3], float radius, const char* tag, QueryCallback callback, void* userdata);
void lovrWorldGetGravity(World* world, float gravity[3]);
void lovrWorldSetGravity(World* world, float gravity[3]);
const char* lovrWorldGetTagName(World* world, uint32_t tag);
void lovrWorldDisableCollisionBetween(World* world, const char* tag1, const char* tag2);
void lovrWorldEnableCollisionBetween(World* world, const char* tag1, const char* tag2);
bool lovrWorldIsCollisionEnabledBetween(World* world, const char* tag1, const char* tag);

// Deprecated
int lovrWorldGetStepCount(World* world);
void lovrWorldSetStepCount(World* world, int iterations);
float lovrWorldGetResponseTime(World* world);
void lovrWorldSetResponseTime(World* world, float responseTime);
float lovrWorldGetTightness(World* world);
void lovrWorldSetTightness(World* world, float tightness);
void lovrWorldGetLinearDamping(World* world, float* damping, float* threshold);
void lovrWorldSetLinearDamping(World* world, float damping, float threshold);
void lovrWorldGetAngularDamping(World* world, float* damping, float* threshold);
void lovrWorldSetAngularDamping(World* world, float damping, float threshold);
bool lovrWorldIsSleepingAllowed(World* world);
void lovrWorldSetSleepingAllowed(World* world, bool allowed);

// Collider

Collider* lovrColliderCreate(World* world, Shape* shape, float position[3]);
void lovrColliderDestroy(void* ref);
void lovrColliderDestroyData(Collider* collider);
bool lovrColliderIsDestroyed(Collider* collider);
bool lovrColliderIsEnabled(Collider* collider);
void lovrColliderSetEnabled(Collider* collider, bool enable);
void lovrColliderInitInertia(Collider* collider, Shape* shape);
World* lovrColliderGetWorld(Collider* collider);
Joint* lovrColliderGetJoints(Collider* collider, Joint* joint);
Shape* lovrColliderGetShape(Collider* collider, uint32_t child);
void lovrColliderSetShape(Collider* collider, Shape* shape);
void lovrColliderGetShapeOffset(Collider* collider, float position[3], float orientation[4]);
void lovrColliderSetShapeOffset(Collider* collider, float position[3], float orientation[4]);
const char* lovrColliderGetTag(Collider* collider);
bool lovrColliderSetTag(Collider* collider, const char* tag);
float lovrColliderGetFriction(Collider* collider);
void lovrColliderSetFriction(Collider* collider, float friction);
float lovrColliderGetRestitution(Collider* collider);
void lovrColliderSetRestitution(Collider* collider, float restitution);
bool lovrColliderIsKinematic(Collider* collider);
void lovrColliderSetKinematic(Collider* collider, bool kinematic);
bool lovrColliderIsSensor(Collider* collider);
void lovrColliderSetSensor(Collider* collider, bool sensor);
bool lovrColliderIsContinuous(Collider* collider);
void lovrColliderSetContinuous(Collider* collider, bool continuous);
float lovrColliderGetGravityScale(Collider* collider);
void lovrColliderSetGravityScale(Collider* collider, float scale);
bool lovrColliderIsSleepingAllowed(Collider* collider);
void lovrColliderSetSleepingAllowed(Collider* collider, bool allowed);
bool lovrColliderIsAwake(Collider* collider);
void lovrColliderSetAwake(Collider* collider, bool awake);
float lovrColliderGetMass(Collider* collider);
void lovrColliderSetMass(Collider* collider, float mass);
void lovrColliderGetMassData(Collider* collider, float centerOfMass[3], float* mass, float inertia[6]);
void lovrColliderSetMassData(Collider* collider, float centerOfMass[3], float mass, float inertia[6]);
void lovrColliderGetPosition(Collider* collider, float position[3]);
void lovrColliderSetPosition(Collider* collider, float position[3]);
void lovrColliderGetOrientation(Collider* collider, float orientation[4]);
void lovrColliderSetOrientation(Collider* collider, float orientation[4]);
void lovrColliderGetLinearVelocity(Collider* collider, float velocity[3]);
void lovrColliderSetLinearVelocity(Collider* collider, float velocity[3]);
void lovrColliderGetAngularVelocity(Collider* collider, float velocity[3]);
void lovrColliderSetAngularVelocity(Collider* collider, float velocity[3]);
void lovrColliderGetLinearDamping(Collider* collider, float* damping, float* threshold);
void lovrColliderSetLinearDamping(Collider* collider, float damping, float threshold);
void lovrColliderGetAngularDamping(Collider* collider, float* damping, float* threshold);
void lovrColliderSetAngularDamping(Collider* collider, float damping, float threshold);
void lovrColliderApplyForce(Collider* collider, float force[3]);
void lovrColliderApplyForceAtPosition(Collider* collider, float force[3], float position[3]);
void lovrColliderApplyTorque(Collider* collider, float torque[3]);
void lovrColliderApplyLinearImpulse(Collider* collider, float impulse[3]);
void lovrColliderApplyLinearImpulseAtPosition(Collider* collider, float impulse[3], float position[3]);
void lovrColliderApplyAngularImpulse(Collider* collider, float impulse[3]);
void lovrColliderGetLocalCenter(Collider* collider, float center[3]);
void lovrColliderGetLocalPoint(Collider* collider, float world[3], float local[3]);
void lovrColliderGetWorldPoint(Collider* collider, float local[3], float world[3]);
void lovrColliderGetLocalVector(Collider* collider, float world[3], float local[3]);
void lovrColliderGetWorldVector(Collider* collider, float local[3], float world[3]);
void lovrColliderGetLinearVelocityFromLocalPoint(Collider* collider, float point[3], float velocity[3]);
void lovrColliderGetLinearVelocityFromWorldPoint(Collider* collider, float point[3], float velocity[3]);
void lovrColliderGetAABB(Collider* collider, float aabb[6]);

// Shapes

typedef enum {
  SHAPE_SPHERE,
  SHAPE_BOX,
  SHAPE_CAPSULE,
  SHAPE_CYLINDER,
  SHAPE_CONVEX,
  SHAPE_MESH,
  SHAPE_TERRAIN,
  SHAPE_COMPOUND
} ShapeType;

void lovrShapeDestroy(void* ref);
void lovrShapeDestroyData(Shape* shape);
ShapeType lovrShapeGetType(Shape* shape);
void lovrShapeGetMass(Shape* shape, float density, float centerOfMass[3], float* mass, float inertia[6]);
void lovrShapeGetAABB(Shape* shape, float position[3], float orientation[4], float aabb[6]);

SphereShape* lovrSphereShapeCreate(float radius);
float lovrSphereShapeGetRadius(SphereShape* sphere);

BoxShape* lovrBoxShapeCreate(float dimensions[3]);
void lovrBoxShapeGetDimensions(BoxShape* box, float dimensions[3]);

CapsuleShape* lovrCapsuleShapeCreate(float radius, float length);
float lovrCapsuleShapeGetRadius(CapsuleShape* capsule);
float lovrCapsuleShapeGetLength(CapsuleShape* capsule);

CylinderShape* lovrCylinderShapeCreate(float radius, float length);
float lovrCylinderShapeGetRadius(CylinderShape* cylinder);
float lovrCylinderShapeGetLength(CylinderShape* cylinder);

ConvexShape* lovrConvexShapeCreate(float points[], uint32_t count);

MeshShape* lovrMeshShapeCreate(int vertexCount, float vertices[], int indexCount, uint32_t indices[]);

TerrainShape* lovrTerrainShapeCreate(float* vertices, uint32_t n, float scaleXZ, float scaleY);

CompoundShape* lovrCompoundShapeCreate(Shape** shapes, float* positions, float* orientations, uint32_t count, bool freeze);
bool lovrCompoundShapeIsFrozen(CompoundShape* shape);
void lovrCompoundShapeAddChild(CompoundShape* shape, Shape* child, float position[3], float orientation[4]);
void lovrCompoundShapeReplaceChild(CompoundShape* shape, uint32_t index, Shape* child, float position[3], float orientation[4]);
void lovrCompoundShapeRemoveChild(CompoundShape* shape, uint32_t index);
Shape* lovrCompoundShapeGetChild(CompoundShape* shape, uint32_t index);
uint32_t lovrCompoundShapeGetChildCount(CompoundShape* shape);
void lovrCompoundShapeGetChildOffset(CompoundShape* shape, uint32_t index, float position[3], float orientation[4]);
void lovrCompoundShapeSetChildOffset(CompoundShape* shape, uint32_t index, float position[3], float orientation[4]);

// These tokens need to exist for Lua bindings
#define lovrSphereShapeDestroy lovrShapeDestroy
#define lovrBoxShapeDestroy lovrShapeDestroy
#define lovrCapsuleShapeDestroy lovrShapeDestroy
#define lovrCylinderShapeDestroy lovrShapeDestroy
#define lovrConvexShapeDestroy lovrShapeDestroy
#define lovrMeshShapeDestroy lovrShapeDestroy
#define lovrTerrainShapeDestroy lovrShapeDestroy
#define lovrCompoundShapeDestroy lovrShapeDestroy

// Joints

typedef enum {
  JOINT_BALL,
  JOINT_DISTANCE,
  JOINT_HINGE,
  JOINT_SLIDER
} JointType;

void lovrJointDestroy(void* ref);
void lovrJointDestroyData(Joint* joint);
bool lovrJointIsDestroyed(Joint* joint);
JointType lovrJointGetType(Joint* joint);
Collider* lovrJointGetColliderA(Joint* joint);
Collider* lovrJointGetColliderB(Joint* joint);
Joint* lovrJointGetNext(Joint* joint, Collider* collider);
bool lovrJointIsEnabled(Joint* joint);
void lovrJointSetEnabled(Joint* joint, bool enable);

BallJoint* lovrBallJointCreate(Collider* a, Collider* b, float anchor[3]);
void lovrBallJointGetAnchors(BallJoint* joint, float anchor1[3], float anchor2[3]);
void lovrBallJointSetAnchor(BallJoint* joint, float anchor[3]);
float lovrBallJointGetResponseTime(Joint* joint);
void lovrBallJointSetResponseTime(Joint* joint, float responseTime);
float lovrBallJointGetTightness(Joint* joint);
void lovrBallJointSetTightness(Joint* joint, float tightness);

DistanceJoint* lovrDistanceJointCreate(Collider* a, Collider* b, float anchor1[3], float anchor2[3]);
void lovrDistanceJointGetAnchors(DistanceJoint* joint, float anchor1[3], float anchor2[3]);
void lovrDistanceJointSetAnchors(DistanceJoint* joint, float anchor1[3], float anchor2[3]);
float lovrDistanceJointGetDistance(DistanceJoint* joint);
void lovrDistanceJointSetDistance(DistanceJoint* joint, float distance);
float lovrDistanceJointGetResponseTime(Joint* joint);
void lovrDistanceJointSetResponseTime(Joint* joint, float responseTime);
float lovrDistanceJointGetTightness(Joint* joint);
void lovrDistanceJointSetTightness(Joint* joint, float tightness);

HingeJoint* lovrHingeJointCreate(Collider* a, Collider* b, float anchor[3], float axis[3]);
void lovrHingeJointGetAnchors(HingeJoint* joint, float anchor1[3], float anchor2[3]);
void lovrHingeJointSetAnchor(HingeJoint* joint, float anchor[3]);
void lovrHingeJointGetAxis(HingeJoint* joint, float axis[3]);
void lovrHingeJointSetAxis(HingeJoint* joint, float axis[3]);
float lovrHingeJointGetAngle(HingeJoint* joint);
float lovrHingeJointGetLowerLimit(HingeJoint* joint);
void lovrHingeJointSetLowerLimit(HingeJoint* joint, float limit);
float lovrHingeJointGetUpperLimit(HingeJoint* joint);
void lovrHingeJointSetUpperLimit(HingeJoint* joint, float limit);

SliderJoint* lovrSliderJointCreate(Collider* a, Collider* b, float axis[3]);
void lovrSliderJointGetAxis(SliderJoint* joint, float axis[3]);
void lovrSliderJointSetAxis(SliderJoint* joint, float axis[3]);
float lovrSliderJointGetPosition(SliderJoint* joint);
float lovrSliderJointGetLowerLimit(SliderJoint* joint);
void lovrSliderJointSetLowerLimit(SliderJoint* joint, float limit);
float lovrSliderJointGetUpperLimit(SliderJoint* joint);
void lovrSliderJointSetUpperLimit(SliderJoint* joint, float limit);

// These tokens need to exist for Lua bindings
#define lovrBallJointDestroy lovrJointDestroy
#define lovrDistanceJointDestroy lovrJointDestroy
#define lovrHingeJointDestroy lovrJointDestroy
#define lovrSliderJointDestroy lovrJointDestroy

#include "myext/physics.h"
