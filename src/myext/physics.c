#include <joltc_ext/ext.h>

uint32_t lovrWorldGetActiveColliderCount(World* world) {
  return JPH_PhysicsSystem_GetNumActiveBodies(world->system, JPH_BodyType_Rigid);
}

TriangleShape* lovrTriangleShapeCreate(float vertices[9]) {
  TriangleShape* shape = lovrCalloc(sizeof(TriangleShape));
  shape->ref = 1;
  shape->type = SHAPE_TRIANGLE;
  quat_identity(shape->rotation);

  shape->handle = (JPH_Shape*) JPH_TriangleShape_Create(
    vec3_toJolt(vertices),
    vec3_toJolt((vertices + 3)),
    vec3_toJolt((vertices + 6)),
    0
  );
  return shape;
}

bool lovrWorldQueryTriangle(
  World* world, float vertices[9], int filter, OverlapCallback* callback, void* userdata
) {
  Shape* shape = lovrTriangleShapeCreate(vertices);
  float pose[7] = { 0, 0, 0, 0, 0, 0, 1 };
  bool r = lovrWorldOverlapShape(world, shape, pose, 0.f, filter, callback, userdata);
  lovrShapeDestruct(shape);
  return r;
}

PlaneShape* lovrPlaneShapeCreate(float normal[3], float half_size) {
  PlaneShape* shape = lovrCalloc(sizeof(PlaneShape));
  shape->ref = 1;
  shape->type = SHAPE_PLANE;
  JPH_Plane plane = { .normal = { normal[0], normal[1], normal[2] }, .distance = 0 };
  shape->handle = (JPH_Shape*) JPH_PlaneShape_Create(&plane, NULL, half_size);
  JPH_Shape_SetUserData(shape->handle, (uint64_t) (uintptr_t) shape);
  quat_identity(shape->rotation);
  return shape;
}

bool lovrConvexShapeGetIndicesCount(ConvexShape* shape, uint32_t* icount, uint32_t* maxFaceVcount) {
  const JPH_ConvexHullShape* hull = (const JPH_ConvexHullShape*) JPH_DecoratedShape_GetInnerShape((const JPH_DecoratedShape*) shape->handle);
  *icount = 0;
  *maxFaceVcount = 0;
  for (uint32_t i = 0; i < JPH_ConvexHullShape_GetNumFaces(hull); i++) {
    uint32_t n = JPH_ConvexHullShape_GetNumVerticesInFace(hull, i);
    *maxFaceVcount = MAX(*maxFaceVcount, n);
    *icount = *icount + (n - 2) * 3;
  }
  return true;
}

static void setVertice(float* vertice, float pos[3], float normal[3]) {
  vertice[0] = pos[0];
  vertice[1] = pos[1];
  vertice[2] = pos[2];
  vertice[3] = normal[0];
  vertice[4] = normal[1];
  vertice[5] = normal[2];
  vertice[6] = 0.f;
  vertice[7] = 0.f;
}

static void calcTriangleNormal(float p1[3], float p2[3], float p3[3], float outNormal[3]) {
  float tmp[3];
  vec3_init(outNormal, p2);
  vec3_sub(outNormal, p1);
  vec3_init(tmp, p3);
  vec3_sub(tmp, p1);
  vec3_cross(outNormal, tmp);
  vec3_normalize(outNormal);
}

bool lovrConvexShapeGetVertices(ConvexShape* shape, float* vertices, uint32_t maxFaceVcount) {
  const JPH_ConvexHullShape* hull = (const JPH_ConvexHullShape*) JPH_DecoratedShape_GetInnerShape((const JPH_DecoratedShape*) shape->handle);
  uint32_t* fvs = lovrMalloc(maxFaceVcount * sizeof(uint32_t));
  uint32_t idx = 0;
  JPH_Vec3 v;
  for (uint32_t i = 0; i < JPH_ConvexHullShape_GetNumFaces(hull); i++) {
    int n = JPH_ConvexHullShape_GetFaceVertices(hull, i, maxFaceVcount, fvs);
    float p0[3], p1[3], p2[3], normal[3];
    JPH_ConvexHullShape_GetPoint(hull, fvs[0], &v);
    vec3_fromJolt(p0, &v);
    for (int i = 2; i < n; i++) {
      JPH_ConvexHullShape_GetPoint(hull, fvs[i - 1], &v);
      vec3_fromJolt(p1, &v);
      JPH_ConvexHullShape_GetPoint(hull, fvs[i], &v);
      vec3_fromJolt(p2, &v);
      calcTriangleNormal(p0, p1, p2, normal);
      setVertice(vertices + idx, p0, normal);
      setVertice(vertices + idx + 8, p1, normal);
      setVertice(vertices + idx + 16, p2, normal);
      idx += 24;
    }
  }
  lovrFree(fvs);
  return true;
}

Collider* lovrColliderCreateSoftBody(
  World* world, float position[3], float rot[4],
  const float* vertices, size_t vs_total, // vertices = vs_total * 3
  const float* vertices_inv_mass,
  const float* faces, size_t faces_total, // faces: vs index list, 3 points per face

  const float* edges, size_t edges_total, // edges: vs index list, 2 points per edge
  const float* volumes, size_t volumes_total, // edges: vs index list, 4 points per volums

  uint32_t bend_type,
  const float vertex_compliance[3] // Compliance, ShearCompliance, BendCompliance
) {
  uint32_t count = JPH_PhysicsSystem_GetNumBodies(world->system);
  uint32_t limit = JPH_PhysicsSystem_GetMaxBodies(world->system);
  lovrCheck(count < limit, "Too many colliders!");

  Collider* collider = lovrCalloc(sizeof(Collider));
  collider->ref = 1;
  collider->world = world;
  collider->tag = 0xff;
  collider->enabled = true;
  collider->automaticMass = true;

  JPH_ObjectLayer objectLayer = world->tagCount + 1; // Untagged/shapeless layer

  JPH_SoftBodySharedSettings* shared_settings = JPH_SoftBodySharedSettings_CreateByVertices(
    vertices, vs_total, vertices_inv_mass,
    faces, faces_total,
    edges, edges_total,
    volumes, volumes_total,
    bend_type, vertex_compliance
  );

  JPH_RVec3* p = vec3_toJolt(position);
  JPH_Quat* q = quat_toJolt(rot);
  JPH_SoftBodyCreationSettings* settings = JPH_SoftBodyCreationSettings_CreateBySharedSettings(
    shared_settings, p, q, objectLayer
  );

  collider->body = JPH_BodyInterface_CreateSoftBody(world->bodyInterfaceLocked, settings);
  collider->id = JPH_Body_GetID(collider->body);
  JPH_Body_SetUserData(collider->body, (uint64_t) (uintptr_t) collider);
  // JPH_SoftBodySharedSettings_Destroy(shared_settings);
  JPH_SoftBodyCreationSettings_Destroy(settings);

  JPH_BodyInterface_AddBody(world->bodyInterfaceLocked, collider->id, JPH_Activation_Activate);

  vec3_init(collider->lastPosition, position);
  quat_init(collider->lastOrientation, rot);

  // lovrColliderSetLinearDamping(collider, world->defaultLinearDamping);
  // lovrColliderSetAngularDamping(collider, world->defaultAngularDamping);
  // lovrColliderSetSleepingAllowed(collider, world->defaultIsSleepingAllowed);

  if (world->colliders) {
    collider->next = world->colliders;
    collider->next->prev = collider;
  }

  world->colliders = collider;

  lovrRetain(collider);
  return collider;
}

size_t lovrColliderGetSoftBodyVerticesCount(Collider* collider) {
  return JPH_SoftBody_GetNumVertices(collider->body);
}

void lovrColliderGetSoftBodyVertices(Collider* collider, float* outVertices) {
  JPH_SoftBody_GetVertices(collider->body, outVertices);
}

size_t lovrColliderGetSoftBodyFacesCount(Collider* collider) {
  return JPH_SoftBody_GetNumFaces(collider->body);
}

void lovrColliderGetSoftBodyFaces(Collider* collider, float* outFaces) {
  JPH_SoftBody_GetFaces(collider->body, outFaces);
}