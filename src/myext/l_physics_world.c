
static int l_lovrWorldQueryTriangle(lua_State* L) {
  World* world = luax_checktype(L, 1, World);
  float vertices[9];
  int index = luax_readvec3(L, 2, vertices, NULL);
  index = luax_readvec3(L, index, vertices + 3, NULL);
  index = luax_readvec3(L, index, vertices + 6, NULL);
  uint32_t filter = luax_checktagmask(L, index++, world);

  int ret_args = 0;
  if (lua_isnoneornil(L, index)) {
    OverlapResult hit;
    if (lovrWorldQueryTriangle(world, vertices, filter, overlapFirstCallback, &hit)) {
      ret_args = luax_pushoverlapresult(L, &hit);
    }
  } else {
    luaL_checktype(L, index, LUA_TFUNCTION);
    lua_settop(L, index);
    lovrWorldQueryTriangle(world, vertices, filter, overlapCallback, L);
  }
  return ret_args;
}

static int l_lovrWorldNewPlaneCollider(lua_State* L) {
  World* world = luax_checkworld(L, 1);
  PlaneShape* shape = luax_newplaneshape(L, 2);
  float position[3] = { 0.f, 0.f, 0.f };
  Collider* collider = lovrColliderCreate(world, position, shape);
  lovrRelease(shape, lovrShapeDestroy);
  luax_assert(L, collider);
  luax_pushtype(L, Collider, collider);
  lovrRelease(collider, lovrColliderDestroy);
  return 1;
}

static void format_index(lua_State* L, uint32_t* indices, size_t len, size_t max_i) {
  for (size_t i = 0; i < len; i++) {
    lovrAssert(indices[i] >= 1 && indices[i] <= max_i, "Invalid index %i", indices[i]);
    indices[i] = indices[i] - 1;
  }
}

static void luax_readobjarr_i(lua_State* L, int index, int n, uint32_t* out_arr, const char* name) {
  if (n <= 0) return;
  lovrAssert(luax_len(L, index) >= n, "length of %s table must >= %i", name, n);

  if (index < 0) index = lua_gettop(L) + index + 1;
  for (int i = 0; i < n; i++) {
    lua_rawgeti(L, index, i + 1);
    out_arr[i] = lua_tointeger(L, -1);
  }
  lua_pop(L, n);
}

// vs: { 1,2,3, 4,5,6, ... }
// faces: { 123, 4,5,6, ... }
// NewSoftBodyCollider(vs, faces, edges, volums, bend_type, vertex_compliances{ 1, 2, 3 })
static int l_lovrWorldNewSoftBodyCollider(lua_State* L) {
  World* world = luax_checkworld(L, 1);
  float pos[3], rot[4];
  int index = luax_readvec3(L, 2, pos, NULL);
  index = luax_readquat(L, index, rot, NULL);

  uint32_t vs_val_total = luax_len(L, index);
  uint32_t vs_total = vs_val_total / 3;
  float *vs = lovrMalloc(sizeof(float) * vs_val_total);
  luax_readobjarr(L, index, vs_val_total, vs, "Vertices");
  index++;

   float *vs_mass = lovrMalloc(sizeof(float) * vs_total);
  uint32_t mass_total = luax_len(L, index);
  luax_readobjarr(L, index, vs_total, vs_mass, "VerticesMass");
  // to inv_mass
  for (uint32_t i = 0; i < vs_total; i++) {
    if (vs_mass[i] > 0.f)  {
      vs_mass[i] = 1.f / vs_mass[i];
    } else {
      vs_mass[i] = 0.f;
    }
  }
  index++;

  uint32_t faces_total = luax_len(L, index);
  uint32_t *faces = faces_total > 0 ? lovrMalloc(sizeof(uint32_t) * faces_total) : NULL;
  luax_readobjarr_i(L, index, faces_total, faces, "Faces");
  format_index(L, faces, faces_total, vs_total);
  index++;

  uint32_t edges_total = luax_len(L, index);
  uint32_t *edges = edges_total > 0 ? lovrMalloc(sizeof(uint32_t) * edges_total) : NULL;
  luax_readobjarr_i(L, index, edges_total, edges, "Edges");
  format_index(L, edges, edges_total, vs_total);
  index++;

  uint32_t volumes_total = luax_len(L, index);
  uint32_t *volumes = volumes_total > 0 ? lovrMalloc(sizeof(uint32_t) * volumes_total) : NULL;
  luax_readobjarr_i(L, index, volumes_total, volumes, "Volumes");
  format_index(L, volumes, volumes_total, vs_total);
  index++;

  uint32_t bend_type = lua_tointeger(L, index);
  index++;
  lovrAssert(bend_type >= 0 && bend_type <= 2, "Invalid blend_type %i, 0 <= bend_type <= 2", bend_type);

  float vertex_compliance[3];
  luax_readobjarr(L, index, 3, vertex_compliance, "VertexCompliance");

  Collider* collider = lovrColliderCreateSoftBody(
    world, pos, rot,
    vs, vs_total, vs_mass,
    faces, faces_total / 3,
    edges, edges_total / 2,
    volumes, volumes_total / 4,
    bend_type, vertex_compliance
  );

  lovrFree(vs);
  lovrFree(vs_mass);
  lovrFree(faces);
  lovrFree(edges);
  lovrFree(volumes);

  luax_assert(L, collider);
  luax_pushtype(L, Collider, collider);
  lovrRelease(collider, lovrColliderDestroy);
  return 1;
}
