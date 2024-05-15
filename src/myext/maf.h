
MAF void mat4_setPosition(mat4 m, vec3 position) {
  m[12] = position[0];
  m[13] = position[1];
  m[14] = position[2];
}
