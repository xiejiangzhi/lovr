
static void luax_checkfield(lua_State* L, const DataField* field, int idx, char* data) {
  int n = typeComponents[field->type];
  switch (lua_type(L, idx)) {
  case LUA_TNUMBER:
    lovrCheck(n == 1, "Cannot set field '%s' by number", field->name);
    luax_checkfieldn(L, idx, &field->type, data);
    break;
  case LUA_TUSERDATA:
  case LUA_TLIGHTUSERDATA:
    luax_checkfieldv(L, idx, &field->type, data);
    break;
  case LUA_TTABLE:
    lovrCheck(luax_len(L, idx) == n, "Table length for key '%s' must be %d", field->name, n);
    luax_checkfieldt(L, idx, &field->type, data + field->offset);
    break;
  default:
    lovrThrow("Invalid data type %s for field '%s'", lua_typename(L, lua_type(L, idx)), field->name);
  }
}

static void luax_checkfields(lua_State* L, const DataField* format, int tidx, char* data) {
  if (tidx < 0) tidx = lua_gettop(L) + tidx + 1;

  lovrCheck(
    luax_len(L, tidx) == format->fieldCount,
    "Expect %s has %i elements, but got %i",
    format->name, format->fieldCount, luax_len(L, tidx)
  );

  for (int i = 0; i < format->fieldCount; i++) {
    lua_rawgeti(L, tidx, i + 1);
    DataField* field = format->fields + i;
    char* fdata = data + field->offset;

    if (field->fieldCount == 0) {
      if (field->length == 0) {
        luax_checkfield(L, field, -1, fdata);
      } else {
        lovrCheck(lua_istable(L, -1), "field %s val must be a table");
        int len = MIN(luax_len(L, -1), field->length);
        for (int j = 0; j < len; j++) {
          lua_rawgeti(L, -1, j + 1);
          luax_checkfield(L, field, -1, fdata + j * field->stride);
          lua_pop(L, 1);
        }
      }
    } else {
      if (field->length == 0) {
        luax_checkfields(L, field, -1, fdata);
      } else {
        lovrCheck(lua_istable(L, -1), "field %s val must be a table");
        int len = MIN(luax_len(L, -1), field->length);
        for (int j = 0; j < len; j++) {
          lua_rawgeti(L, -1, j + 1);
          luax_checkfields(L, field, -1, fdata + j * field->stride);
          lua_pop(L, 1);
        }
      }
    }
    lua_pop(L, 1);
  }
}

static int l_lovrBufferSetDataEx(lua_State* L) {
  Buffer* buffer = luax_checktype(L, 1, Buffer);
  lovrCheck(lua_istable(L, 2), "Must give a table");
  const BufferInfo* info = lovrBufferGetInfo(buffer);
  const DataField* format = info->format;
  lovrCheck(format, "Buffer must be created with format information to copy a table to it");
  lovrCheck(
    luax_len(L, 2) == format->fieldCount,
    "Expect buffer has %i elements, but got %i",
    format->fieldCount, luax_len(L, 2)
  );
  luax_checkfields(L, format, 2, lovrBufferSetData(buffer, 0, ~0u));
  return 0;
}
