
static void luax_checkfield(lua_State* L, const DataField* field, int idx, char* data) {
  int n = typeComponents[field->type];
  switch (lua_type(L, idx)) {
  case LUA_TNUMBER:
    lovrCheck(n == 1, "Cannot set field '%s' by number", field->name);
    luax_checkfieldn(L, idx, field, data);
    break;
  case LUA_TUSERDATA:
  case LUA_TLIGHTUSERDATA:
    luax_checkfieldv(L, idx, field, data);
    break;
  case LUA_TTABLE:
    int len = luax_len(L, idx);
    lovrCheck(
      len == n, "Table length for key '%s#%s' must be %d, got %i",
      field->parent ? field->parent->name : "Root", field->name, n, len
    );
    luax_checkfieldt(L, idx, field, data);
    break;
  default:
    lovrThrow("Invalid data type %s for field '%s'", lua_typename(L, lua_type(L, idx)), field->name);
  }
}

static void luax_checkfields(lua_State* L, const DataField* format, int tidx, char* data) {
  if (tidx < 0) tidx = lua_gettop(L) + tidx + 1;

  int ttype = lua_type(L, tidx);
  int tlen = ttype == LUA_TTABLE ? luax_len(L, tidx) : -1;

  if (format->length > 0) {
    lovrCheck(
      lua_istable(L, tidx),
      "Array %s val must be a table", format->name ? format->name : "None"
    );

    int len = MIN(luax_len(L, -1), format->length);
    for (int i = 0; i < len; i++) {
      lua_rawgeti(L, tidx, i + 1);

      if (format->fieldCount == 0) {
        DataField* field = format->fields ? format->fields : format;
        char* fdata = data + field->stride * i;
        luax_checkfields(L, field, -1, fdata);
      } else {
        for (int j = 0; j < format->fieldCount; j++) {
          DataField* field = format->fields + j;
          char* fdata = data + field->offset + field->stride * i;
          lua_rawgeti(L, -1, j + 1);
          luax_checkfields(L, field, -1, fdata);
          lua_pop(L, 1);
        }
      }
      lua_pop(L, 1);
    }
  } else if (format->fieldCount > 0) {
    lovrCheck(
      lua_istable(L, tidx), "Struct %s val must be a table",
      format->name ? format->name : "None"
    );
    lovrCheck(
      luax_len(L, tidx) == format->fieldCount,
      "Expect %s has %i fields, but got %i",
      format->name, format->fieldCount, luax_len(L, tidx)
    );

    for (int i = 0; i < format->fieldCount; i++) {
      lua_rawgeti(L, tidx, i + 1);
      DataField* field = format->fields + i;
      char* fdata = data + field->offset;

      luax_checkfields(L, field, -1, fdata);
      lua_pop(L, 1);
    }
  } else {
    luax_checkfield(L, format, tidx, data);
  }
}

static int l_lovrBufferSetDataEx(lua_State* L) {
  Buffer* buffer = luax_checktype(L, 1, Buffer);
  lovrCheck(lua_istable(L, 2), "Must give a table");
  const BufferInfo* info = lovrBufferGetInfo(buffer);
  const DataField* format = info->format;
  lovrCheck(format, "Buffer must be created with format information to copy a table to it");
  // int len = luax_len(L, 2);
  // lovrCheck(
  //   len == format->fieldCount || len == format->length,
  //   "Expect buffer has %i elements, but got %i",
  //   format->fieldCount, luax_len(L, 2)
  // );
  luax_checkfields(L, format, 2, lovrBufferSetData(buffer, 0, ~0u));
  return 0;
}
