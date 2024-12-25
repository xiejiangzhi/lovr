
#include "joltc_ext/ext.h"

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

// bool lovrCharacterCreate(
//   World* world, Shape* shape, float position[3], float orientation[4],
// ) {
// }