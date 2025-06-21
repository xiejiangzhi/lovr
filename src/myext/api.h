

#ifndef LOVR_DISABLE_PHYSICS
struct Shape* luax_newplaneshape(lua_State* L, int index);

float* luax_tovector_with_type(lua_State* L, int index, VectorType type);
#endif