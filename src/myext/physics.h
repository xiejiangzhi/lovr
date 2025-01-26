typedef Shape TriangleShape;
typedef Shape PlaneShape;

TriangleShape* lovrTriangleShapeCreate(float vertices[9]);
PlaneShape* lovrPlaneShapeCreate(float normal[3], float distance);

bool lovrWorldQueryTriangle(
  World* world, float vertices[9], int filter, OverlapCallback callback, void* userdata
);

// struct Character;
// struct CharacterVirtual;