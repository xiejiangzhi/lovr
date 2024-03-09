#include "physics/physics.h"

bool lovrShapeQueryOverlapping(Shape* shape, QueryCallback callback, void* userdata);

bool lovrWorldQueryTriangle(World* world, float position[9], const char* tag, QueryCallback callback, void* userdata);
bool lovrWorldQueryShape(World* world, Shape* shape, float position[3], float orientation[4], const char* tag, QueryCallback callback, void* userdata);
