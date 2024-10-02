
static struct {
  const char* name;
  int components;
} lovrVectorComp[] = {
  [V_VEC2] = { "vec2", 2 },
  [V_VEC3] = { "vec3", 3 },
  [V_VEC4] = { "vec4", 4 },
  [V_QUAT] = { "quat", 4 },
  [V_MAT4] = { "mat4", 16 }
};

float* luax_tovector(lua_State* L, int index, VectorType* type) {
  if (lua_istable(L, index)) {
    lua_rawgeti(L, index, 1);
    bool is_num = lua_isnumber(L, -1);
    lua_pop(L, 1);
    if (!is_num) {
      type = V_NONE;
      return NULL;
    }

    VectorType vtype;
    switch(luax_len(L, index)) {
    case 2: vtype = V_VEC2; break;
    case 3: vtype = V_VEC3; break;
    case 4: vtype = V_VEC4; break;
    case 5: vtype = V_QUAT; break; // bad way to detect quat
    case 16: vtype = V_MAT4; break;
    default:
      if (type) *type = V_NONE;
      return NULL;
    }
    float* p;
    lovrPoolAllocate(pool, vtype, &p);
    luax_readobjarr(L, index, lovrVectorComp[vtype].components, p, lovrVectorComp[vtype].name);
    if (type) *type = vtype;
    return p;
  } else {
    void* p = lua_touserdata(L, index);

    if (p) {
      if (lua_type(L, index) == LUA_TLIGHTUSERDATA) {
        Vector v = { .pointer = p };
        if (v.handle.type > V_NONE && v.handle.type < MAX_VECTOR_TYPES) {
          if (type) *type = v.handle.type;
          float* pointer = lovrPoolResolve(pool, v);
          luax_assert(L, pointer);
          return pointer;
        }
      } else {
        VectorType* t = p;
        if (*t > V_NONE && *t < MAX_VECTOR_TYPES) {
          if (type) *type = *t;
          return (float*) (t + 1);
        }
      }
    }

    if (type) *type = V_NONE;
    return NULL;
  }
}

float* luax_checkvector(lua_State* L, int index, VectorType type, const char* expected) {
  VectorType t;
  float* p;
  if (lua_istable(L, index)) {
    lovrPoolAllocate(pool, type, &p);
    if (p) {
      luax_readobjarr(L, index, lovrVectorComp[type].components, p, lovrVectorComp[type].name);
    }
    return p;
  } else {
    p = luax_tovector(L, index, &t);
  }

  if (!p || t != type) {
    luax_typeerror(L, index, expected ? expected : lovrVectorComp[type].name);
  }
  return p;
}
