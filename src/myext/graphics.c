
static bool lovrPassViewCullTest(Pass* pass, float* bounds) {
  Canvas* canvas = &pass->canvas;
  Camera* camera = &pass->cameras[(pass->cameraCount - 1) * pass->canvas.views];
  typedef struct { float planes[6][4]; } Frustum;
  Frustum* frusta = tempAlloc(&state.allocator, canvas->views * sizeof(Frustum));

  for (uint32_t v = 0; v < canvas->views; v++) {
    float* m = camera->viewProjection;
    memcpy(frusta[v].planes, (float[6][4]) {
      { (m[3] + m[0]), (m[7] + m[4]), (m[11] + m[8]), (m[15] + m[12]) }, // Left
      { (m[3] - m[0]), (m[7] - m[4]), (m[11] - m[8]), (m[15] - m[12]) }, // Right
      { (m[3] + m[1]), (m[7] + m[5]), (m[11] + m[9]), (m[15] + m[13]) }, // Bottom
      { (m[3] - m[1]), (m[7] - m[5]), (m[11] - m[9]), (m[15] - m[13]) }, // Top
      { m[2], m[6], m[10], m[14] }, // Near
      { (m[3] - m[2]), (m[7] - m[6]), (m[11] - m[10]), (m[15] - m[14]) } // Far
    }, sizeof(Frustum));
  }

  float* center = bounds;
  float* extent = bounds + 3;
  float corners[8][3] = {
    { center[0] - extent[0], center[1] - extent[1], center[2] - extent[2] },
    { center[0] - extent[0], center[1] - extent[1], center[2] + extent[2] },
    { center[0] - extent[0], center[1] + extent[1], center[2] - extent[2] },
    { center[0] - extent[0], center[1] + extent[1], center[2] + extent[2] },
    { center[0] + extent[0], center[1] - extent[1], center[2] - extent[2] },
    { center[0] + extent[0], center[1] - extent[1], center[2] + extent[2] },
    { center[0] + extent[0], center[1] + extent[1], center[2] - extent[2] },
    { center[0] + extent[0], center[1] + extent[1], center[2] + extent[2] }
  };

  for (uint32_t i = 0; i < COUNTOF(corners); i++) {
    mat4_mulPoint(pass->transform, corners[i]);
  }

  uint32_t visible = canvas->views;

  for (uint32_t v = 0; v < canvas->views; v++) {
    for (uint32_t p = 0; p < 6; p++) {
      bool inside = false;

      for (uint32_t c = 0; c < COUNTOF(corners); c++) {
        if (vec3_dot(corners[c], frusta[v].planes[p]) + frusta[v].planes[p][3] > 0.f) {
          inside = true;
          break;
        }
      }

      if (!inside) {
        visible--;
        break;
      }
    }
  }
  return visible;
}
