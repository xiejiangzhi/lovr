#include "lj_def.h"
#include "lj_tab.h"

static TValue *index2adr(lua_State *L, int idx) {
  if (idx > 0) {
    TValue *o = L->base + (idx - 1);
    return o < L->top ? o : niltv(L);
  }
  return niltv(L);
}

void lua_rawgeti_numarr(lua_State *L, int idx, int tsi, float* out_arr, int out_n) {
  cTValue *v, *t = index2adr(L, idx);
  lj_checkapi(tvistab(t), "stack slot %d is not a table", idx);
  GCtab* gt = tabV(t);

  for (int i = 0; i < out_n; i++) {
    v = inarray(gt, tsi + i) ? arrayslot(gt, tsi + i) : NULL;
    if (v) {
      if (LJ_LIKELY(tvisnumber(v)))
        out_arr[i] = numberVnum(v);
      else
        out_arr[i] = 0;
    } else {
      out_arr[i] = 0;
    }
  }
}

void luax_readobjarr(lua_State* L, int index, int n, float* out_arr, const char* name) {
  lovrAssert(luax_len(L, index) >= n, "length of %s table must >= %i", name, n);
  lua_rawgeti_numarr(L, index, 1, out_arr, n);

  // if (index < 0) index = lua_gettop(L) + index + 1;
  // for (int i = 0; i < n; i++) {
  //   lua_rawgeti(L, index, i + 1);
  //   out[i] = lua_tonumber(L, -1);
  // }
  // lua_pop(L, n);
}

void luax_writeobjarr(lua_State* L, int index, int n, float* in_arr) {
  for (int i = 0; i < n; i++) {
    lua_pushnumber(L, in_arr[i]);
    lua_rawseti(L, index, i + 1);
  }
}
