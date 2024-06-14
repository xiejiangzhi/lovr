
int luax_readvec2(lua_State* L, int index, vec2 v, const char* expected) {
  switch (lua_type(L, index)) {
    case LUA_TNIL:
    case LUA_TNONE:
      v[0] = v[1] = 0.f;
      return index + 1;
    case LUA_TNUMBER:
      v[0] = luax_tofloat(L, index++);
      v[1] = luax_optfloat(L, index++, v[0]);
      return index;
    case LUA_TTABLE:
      luax_readobjarr(L, index, 2, v, "vec2");
      return index + 1;
    default:
      vec2_init(v, luax_checkvector(L, index, V_VEC2, expected ? expected : "vec2, table or number"));
      return index + 1;
  }
}

int luax_readvec3(lua_State* L, int index, vec3 v, const char* expected) {
  switch (lua_type(L, index)) {
    case LUA_TNIL:
    case LUA_TNONE:
      v[0] = v[1] = v[2] = 0.f;
      return index + 1;
    case LUA_TNUMBER:
      v[0] = luax_tofloat(L, index++);
      v[1] = luax_optfloat(L, index++, v[0]);
      v[2] = luax_optfloat(L, index++, v[0]);
      return index;
    case LUA_TTABLE:
      luax_readobjarr(L, index, 3, v, "vec3");
      return index + 1;
    default:
      vec3_init(v, luax_checkvector(L, index, V_VEC3, expected ? expected : "vec3, table or number"));
      return index + 1;
  }
}

int l_lovrVec3Set(lua_State* L) {
  vec3 v = luax_checkvector(L, 1, V_VEC3, NULL);
  if (lua_isnoneornil(L, 2) || lua_type(L, 2) == LUA_TNUMBER) {
    float x = luax_optfloat(L, 2, 0.f);
    vec3_set(v, x, luax_optfloat(L, 3, x), luax_optfloat(L, 4, x));
  } else if (lua_istable(L, 2)) {
    float p[3];
    luax_readobjarr(L, 2, 3, p, "vec3");
    vec3_set(v, p[0], p[1], p[2]);
  } else {
    VectorType t;
    float* p = luax_tovector(L, 2, &t);
    if (p && t == V_VEC3) {
      vec3_init(v, p);
    } else if (p && t == V_MAT4) {
      vec3_set(v, p[12], p[13], p[14]);
    } else if (p && t == V_QUAT) {
      quat_getDirection(p, v);
    } else{
      luax_typeerror(L, 2, "vec3, quat, mat4, or number");
    }
  }
  lua_settop(L, 1);
  return 1;
}

int luax_readvec4(lua_State* L, int index, vec4 v, const char* expected) {
  switch (lua_type(L, index)) {
    case LUA_TNIL:
    case LUA_TNONE:
      v[0] = v[1] = v[2] = v[3] = 0.f;
      return index + 1;
    case LUA_TNUMBER:
      v[0] = luax_tofloat(L, index++);
      v[1] = luax_optfloat(L, index++, v[0]);
      v[2] = luax_optfloat(L, index++, v[0]);
      v[3] = luax_optfloat(L, index++, v[0]);
      return index;
    case LUA_TTABLE:
      luax_readobjarr(L, index, 4, v, "vec4");
      return index + 1;
    default:
      vec4_init(v, luax_checkvector(L, index, V_VEC4, expected ? expected : "vec4, table or number"));
      return index + 1;
  }
}

int luax_readscale(lua_State* L, int index, vec3 v, int components, const char* expected) {
  int tlen;
  switch (lua_type(L, index)) {
    case LUA_TNIL:
    case LUA_TNONE:
      v[0] = v[1] = v[2] = 1.f;
      return index + components;
    case LUA_TNUMBER:
      if (components == 1) {
        v[0] = v[1] = v[2] = luax_tofloat(L, index++);
      } else if (components == -2) { // -2 is special and means "2 components: xy and z"
        v[0] = v[1] = luax_tofloat(L, index++);
        v[2] = luax_optfloat(L, index++, 1.f);
      } else {
        v[0] = v[1] = v[2] = 1.f;
        for (int i = 0; i < components; i++) {
          v[i] = luax_optfloat(L, index++, v[0]);
        }
      }
      return index;
    case LUA_TTABLE:
      tlen = luax_len(L, index);
      if (tlen >= 3) {
        luax_readobjarr(L, index, 3, v, "scale");
      } else if (tlen == 2) {
        luax_readobjarr(L, index, 2, v, "scale");
        v[2] = 1.f;
      } else {
        return luax_typeerror(L, index, "table length must >= 2");
      }
      return index + 1;
    default: {
      VectorType type;
      float* u = luax_tovector(L, index++, &type);
      if (type == V_VEC2) {
        v[0] = u[0];
        v[1] = u[1];
        v[2] = 1.f;
      } else if (type == V_VEC3) {
        vec3_init(v, u);
      } else {
        return luax_typeerror(L, index, "vec2, vec3, table or number");
      }
      return index;
    }
  }
}

int luax_readquat(lua_State* L, int index, quat q, const char* expected) {
  float angle, ax, ay, az;
  switch (lua_type(L, index)) {
    case LUA_TNIL:
    case LUA_TNONE:
      quat_identity(q);
      return ++index;
    case LUA_TNUMBER:
      angle = luax_optfloat(L, index++, 0.f);
      ax = luax_optfloat(L, index++, 0.f);
      ay = luax_optfloat(L, index++, 1.f);
      az = luax_optfloat(L, index++, 0.f);
      quat_fromAngleAxis(q, angle, ax, ay, az);
      return index;
    case LUA_TTABLE:
      luax_readobjarr(L, index, 4, q, "quat");
      return index + 1;
    default:
      quat_init(q, luax_checkvector(L, index++, V_QUAT, expected ? expected : "quat, table or number"));
      return index;
  }
}

int luax_readmat4(lua_State* L, int index, mat4 m, int scaleComponents) {
  switch (lua_type(L, index)) {
    case LUA_TNIL:
    case LUA_TNONE:
      mat4_identity(m);
      return index + 1;

    case LUA_TLIGHTUSERDATA:
    case LUA_TUSERDATA:
    default: {
      VectorType type;
      float* p = luax_tovector(L, index, &type);
      if (type == V_MAT4) {
        mat4_init(m, p);
        return index + 1;
      }
    } // Fall through

    case LUA_TTABLE:
      if (lua_istable(L, index) && luax_len(L, index) >= 16) {
        luax_readobjarr(L, index, 16, m, "mat4");
        return index + 1;
      }
      // Fall through

    case LUA_TNUMBER: {
      float S[3];
      float R[4];
      mat4_identity(m);
      index = luax_readvec3(L, index, m + 12, "mat4, vec3, table or number");
      index = luax_readscale(L, index, S, scaleComponents, NULL);
      index = luax_readquat(L, index, R, NULL);
      mat4_rotateQuat(m, R);
      mat4_scale(m, S[0], S[1], S[2]);
      return index;
    }
  }
}

static int l_lovrMat4SetPosition(lua_State* L) {
  mat4 m = luax_checkvector(L, 1, V_MAT4, NULL);
  float position[3];
  luax_readvec3(L, 2, position, "nil, number, vec3, or mat4");
  mat4_setPosition(m, position);
  lua_settop(L, 1);
  return 1;
}
