typedef Shape TriangleShape;

TriangleShape* lovrTriangleShapeCreate(float vertices[9]);

bool lovrWorldQueryTriangle(
  World* world, float vertices[9], int filter, OverlapCallback callback, void* userdata
);

// struct Character;
// struct CharacterVirtual;