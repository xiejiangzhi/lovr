typedef Shape TriangleShape;
typedef Shape PlaneShape;

uint32_t lovrWorldGetActiveColliderCount(World* world);

TriangleShape* lovrTriangleShapeCreate(float vertices[9]);
PlaneShape* lovrPlaneShapeCreate(float normal[3], float distance);

bool lovrConvexShapeGetIndicesCount(ConvexShape* shape, uint32_t* icount, uint32_t* maxFaceVcount);
bool lovrConvexShapeGetVertices(ConvexShape* shape, float* vertices, uint32_t maxFaceVcount);

bool lovrWorldQueryTriangle(
  World* world, float vertices[9], int filter, OverlapCallback callback, void* userdata
);

Collider* lovrColliderCreateSoftBody(
  World* world,
  float position[3], float rot[4],
  const float* vertices, size_t vs_total, // vertices = vs_total * 3
  const float* vertices_mass, // vs_total
  const uint32_t* faces, size_t faces_total, // faces: vs index list, 3 points per face

  const uint32_t* edges, size_t edges_total, // edges: vs index list, 2 points per edge
  const uint32_t* volumes, size_t volumes_total, // edges: vs index list, 4 points per volums

  uint32_t bend_type,
  const float vertex_compliance[3] // Compliance, ShearCompliance, BendCompliance
);

size_t lovrColliderGetSoftBodyVerticesCount(Collider* collider);
size_t lovrColliderGetSoftBodyFacesCount(Collider* collider);

void lovrColliderGetSoftBodyVertices(Collider* collider, float* outVertices);
void lovrColliderGetSoftBodyFaces(Collider* collider, uint32_t* outFaces);

// struct Character;
// struct CharacterVirtual;