
// bool lovrShapeQueryOverlapping(Shape* shape, QueryCallback callback, void* userdata) {
//   QueryData data = {
//     .callback = callback, .userdata = userdata, .called = false
//   };
//   dSpaceCollide2(shape->id, (dGeomID)shape->collider->world->space, &data, queryCallback);
//   return data.called;
// }

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
  World* world, float vertices[9], int filter, QueryCallback callback, void* userdata
) {
  Shape* shape = lovrTriangleShapeCreate(vertices);
  float pose[7] = { 0, 0, 0, 0, 0, 0, 1 };
  bool r = lovrWorldOverlapShape(world, shape, pose, filter, callback, userdata);
  lovrShapeDestruct(shape);
  return r;
}



// bool lovrWorldQueryShape(
//   World* world, Shape* shape, float position[3], float orientation[4],
//   const char* tag, QueryCallback callback, void* userdata
// ) {
//   QueryData data = {
//     .callback = callback, .userdata = userdata, .called = false
//   };
//   dSpaceAdd(world->space, shape->id);
//   dGeomSetPosition(shape->id, position[0], position[1], position[2]);
//   dReal q[4] = { orientation[3], orientation[0], orientation[1], orientation[2] };
//   dGeomSetQuaternion(shape->id, q);
//   dSpaceCollide2(shape->id, (dGeomID)world->space, &data, queryCallback);
//   dSpaceRemove(world->space, shape->id);
//   return data.called;
// }