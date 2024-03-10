
bool lovrShapeQueryOverlapping(Shape* shape, QueryCallback callback, void* userdata) {
  QueryData data = {
    .callback = callback, .userdata = userdata, .called = false, .shouldStop = false,
    .tag = shape->collider ? shape->collider->tag : NO_TAG
  };
  dSpaceCollide2(shape->id, (dGeomID)shape->collider->world->space, &data, queryCallback);
  return data.called;
}

bool lovrWorldQueryTriangle(World* world, float vertices[9], const char* tag, QueryCallback callback, void* userdata) {
  QueryData data = {
    .callback = callback, .userdata = userdata, .called = false, .shouldStop = false, .tag = findTag(world, tag)
  };

  dTriIndex indices[3] = { 0, 1, 2 };

  dTriMeshDataID dataID = dGeomTriMeshDataCreate();
  dGeomTriMeshDataBuildSingle(dataID, vertices, 3 * sizeof(float), 3, indices, 3, 3 * sizeof(dTriIndex));
  dGeomTriMeshDataPreprocess2(dataID, (1U << dTRIDATAPREPROCESS_BUILD_FACE_ANGLES), NULL);
  dGeomID mesh = dCreateTriMesh(world->space, dataID, 0, 0, 0);
  dSpaceCollide2(mesh, (dGeomID) world->space, &data, queryCallback);
  dGeomTriMeshDataDestroy(dataID);
  dGeomDestroy(mesh);
  return data.called;
}

bool lovrWorldQueryShape(
  World* world, Shape* shape, float position[3], float orientation[4],
  const char* tag, QueryCallback callback, void* userdata
) {
  QueryData data = {
    .callback = callback, .userdata = userdata, .called = false, .shouldStop = false, .tag = findTag(world, tag)
  };
  dSpaceAdd(world->space, shape->id);
  dGeomSetPosition(shape->id, position[0], position[1], position[2]);
  dReal q[4] = { orientation[3], orientation[0], orientation[1], orientation[2] };
  dGeomSetQuaternion(shape->id, q);
  dSpaceCollide2(shape->id, (dGeomID)world->space, &data, queryCallback);
  dSpaceRemove(world->space, shape->id);
  return data.called;
}