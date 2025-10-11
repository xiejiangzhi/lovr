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

// struct Character;
// struct CharacterVirtual;