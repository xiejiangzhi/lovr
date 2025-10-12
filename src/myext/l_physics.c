
#include "data/blob.h"

static int l_lovrPhysicsNewPlaneShape(lua_State* L) {
  PlaneShape* plane = luax_newplaneshape(L, 1);
  luax_pushtype(L, PlaneShape, plane);
  lovrRelease(plane, lovrPlaneShapeDestroy);
  return 1;
}


static int l_lovrPhysicsGetSoftBodyMeshData(lua_State* L) {
  Collider* collider = luax_checktype(L, 1, Collider);
  size_t vs_total = lovrColliderGetSoftBodyVerticesCount(collider);
  size_t fs_total = lovrColliderGetSoftBodyFacesCount(collider);

  float* vertices = lovrMalloc(sizeof(float) * vs_total * 3);
  float* faces = lovrMalloc(sizeof(float) * fs_total * 3);

  lovrColliderGetSoftBodyVertices(collider, vertices);
  Blob* vs_blob = lovrBlobCreate(vertices, sizeof(float) * vs_total * 3, "Vertices");
  luax_pushtype(L, Blob, vs_blob);
  lovrRelease(vs_blob, lovrBlobDestroy);

  lovrColliderGetSoftBodyFaces(collider, faces);
  Blob* fs_blob = lovrBlobCreate(faces, sizeof(float) * fs_total * 3, "Faces");
  luax_pushtype(L, Blob, fs_blob);
  lovrRelease(fs_blob, lovrBlobDestroy);

  lua_pushnumber(L, vs_total);
  lua_pushnumber(L, fs_total);
  return 4;
}