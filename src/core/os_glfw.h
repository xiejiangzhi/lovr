#ifndef LOVR_USE_GLFW

void os_poll_events(void) {
  //
}

bool os_window_open(const os_window_config* config) {
  return false;
}

bool os_window_is_open(void) {
  return false;
}

void os_window_get_size(uint32_t* width, uint32_t* height) {
  *width = *height = 0;
}

float os_window_get_pixel_density(void) {
  return 0.f;
}

void os_on_quit(fn_quit* callback) {
  //
}

void os_on_focus(fn_focus* callback) {
  //
}

void os_on_resize(fn_resize* callback) {
  //
}

void os_on_key(fn_key* callback) {
  //
}

void os_on_text(fn_text* callback) {
  //
}

void os_on_mouse_button(fn_mouse_button* callback) {
  //
}

void os_on_mouse_move(fn_mouse_move* callback) {
  //
}

void os_on_mousewheel_move(fn_wheel_move* callback)
{
  //
}

void os_get_mouse_position(double* x, double* y) {
  *x = *y = 0.;
}

void os_set_mouse_mode(os_mouse_mode mode) {
  //
}

bool os_is_mouse_down(os_mouse_button button) {
  return false;
}

bool os_is_key_down(os_key key) {
  return false;
}

uintptr_t os_get_win32_window(void) {
  return 0;
}

uintptr_t os_get_win32_instance(void) {
  return 0;
}

uintptr_t os_get_ca_metal_layer(void) {
  return 0;
}

uintptr_t os_get_xcb_connection(void) {
  return 0;
}

uintptr_t os_get_xcb_window(void) {
  return 0;
}

#else

#include <stdio.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <QuartzCore/CAMetalLayer.h>
#endif

#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#include <X11/Xlib-xcb.h>
#endif

#include <GLFW/glfw3native.h>

static struct {
  GLFWwindow* window;
  fn_quit* onQuitRequest;
  fn_focus* onWindowFocus;
  fn_resize* onWindowResize;
  fn_key* onKeyboardEvent;
  fn_text* onTextEvent;
  fn_mouse_button* onMouseButton;
  fn_mouse_move* onMouseMove;
  fn_mousewheel_move* onMouseWheelMove;
  uint32_t width;
  uint32_t height;
} glfwState;

static void onError(int code, const char* description) {
  printf("GLFW error %d: %s\n", code, description);
}

static void onWindowClose(GLFWwindow* window) {
  if (glfwState.onQuitRequest) {
    glfwState.onQuitRequest();
  }
}

static void onWindowFocus(GLFWwindow* window, int focused) {
  if (glfwState.onWindowFocus) {
    glfwState.onWindowFocus(focused);
  }
}

static void onWindowResize(GLFWwindow* window, int width, int height) {
  if (glfwState.onWindowResize && width > 0 && height > 0) {
    glfwState.width = width;
    glfwState.height = height;
    glfwState.onWindowResize(width, height);
  }
}

static void onKeyboardEvent(GLFWwindow* window, int k, int scancode, int a, int mods) {
  if (glfwState.onKeyboardEvent) {
    os_key key;
    switch (k) {
      case GLFW_KEY_A: key = KEY_A; break;
      case GLFW_KEY_B: key = KEY_B; break;
      case GLFW_KEY_C: key = KEY_C; break;
      case GLFW_KEY_D: key = KEY_D; break;
      case GLFW_KEY_E: key = KEY_E; break;
      case GLFW_KEY_F: key = KEY_F; break;
      case GLFW_KEY_G: key = KEY_G; break;
      case GLFW_KEY_H: key = KEY_H; break;
      case GLFW_KEY_I: key = KEY_I; break;
      case GLFW_KEY_J: key = KEY_J; break;
      case GLFW_KEY_K: key = KEY_K; break;
      case GLFW_KEY_L: key = KEY_L; break;
      case GLFW_KEY_M: key = KEY_M; break;
      case GLFW_KEY_N: key = KEY_N; break;
      case GLFW_KEY_O: key = KEY_O; break;
      case GLFW_KEY_P: key = KEY_P; break;
      case GLFW_KEY_Q: key = KEY_Q; break;
      case GLFW_KEY_R: key = KEY_R; break;
      case GLFW_KEY_S: key = KEY_S; break;
      case GLFW_KEY_T: key = KEY_T; break;
      case GLFW_KEY_U: key = KEY_U; break;
      case GLFW_KEY_V: key = KEY_V; break;
      case GLFW_KEY_W: key = KEY_W; break;
      case GLFW_KEY_X: key = KEY_X; break;
      case GLFW_KEY_Y: key = KEY_Y; break;
      case GLFW_KEY_Z: key = KEY_Z; break;
      case GLFW_KEY_0: key = KEY_0; break;
      case GLFW_KEY_1: key = KEY_1; break;
      case GLFW_KEY_2: key = KEY_2; break;
      case GLFW_KEY_3: key = KEY_3; break;
      case GLFW_KEY_4: key = KEY_4; break;
      case GLFW_KEY_5: key = KEY_5; break;
      case GLFW_KEY_6: key = KEY_6; break;
      case GLFW_KEY_7: key = KEY_7; break;
      case GLFW_KEY_8: key = KEY_8; break;
      case GLFW_KEY_9: key = KEY_9; break;

      case GLFW_KEY_SPACE: key = KEY_SPACE; break;
      case GLFW_KEY_ENTER: key = KEY_ENTER; break;
      case GLFW_KEY_TAB: key = KEY_TAB; break;
      case GLFW_KEY_ESCAPE: key = KEY_ESCAPE; break;
      case GLFW_KEY_BACKSPACE: key = KEY_BACKSPACE; break;
      case GLFW_KEY_UP: key = KEY_UP; break;
      case GLFW_KEY_DOWN: key = KEY_DOWN; break;
      case GLFW_KEY_LEFT: key = KEY_LEFT; break;
      case GLFW_KEY_RIGHT: key = KEY_RIGHT; break;
      case GLFW_KEY_HOME: key = KEY_HOME; break;
      case GLFW_KEY_END: key = KEY_END; break;
      case GLFW_KEY_PAGE_UP: key = KEY_PAGE_UP; break;
      case GLFW_KEY_PAGE_DOWN: key = KEY_PAGE_DOWN; break;
      case GLFW_KEY_INSERT: key = KEY_INSERT; break;
      case GLFW_KEY_DELETE: key = KEY_DELETE; break;
      case GLFW_KEY_F1: key = KEY_F1; break;
      case GLFW_KEY_F2: key = KEY_F2; break;
      case GLFW_KEY_F3: key = KEY_F3; break;
      case GLFW_KEY_F4: key = KEY_F4; break;
      case GLFW_KEY_F5: key = KEY_F5; break;
      case GLFW_KEY_F6: key = KEY_F6; break;
      case GLFW_KEY_F7: key = KEY_F7; break;
      case GLFW_KEY_F8: key = KEY_F8; break;
      case GLFW_KEY_F9: key = KEY_F9; break;
      case GLFW_KEY_F10: key = KEY_F10; break;
      case GLFW_KEY_F11: key = KEY_F11; break;
      case GLFW_KEY_F12: key = KEY_F12; break;

      case GLFW_KEY_GRAVE_ACCENT: key = KEY_BACKTICK; break;
      case GLFW_KEY_MINUS: key = KEY_MINUS; break;
      case GLFW_KEY_EQUAL: key = KEY_EQUALS; break;
      case GLFW_KEY_LEFT_BRACKET: key = KEY_LEFT_BRACKET; break;
      case GLFW_KEY_RIGHT_BRACKET: key = KEY_RIGHT_BRACKET; break;
      case GLFW_KEY_BACKSLASH: key = KEY_BACKSLASH; break;
      case GLFW_KEY_SEMICOLON: key = KEY_SEMICOLON; break;
      case GLFW_KEY_APOSTROPHE: key = KEY_APOSTROPHE; break;
      case GLFW_KEY_COMMA: key = KEY_COMMA; break;
      case GLFW_KEY_PERIOD: key = KEY_PERIOD; break;
      case GLFW_KEY_SLASH: key = KEY_SLASH; break;

      case GLFW_KEY_LEFT_CONTROL: key = KEY_LEFT_CONTROL; break;
      case GLFW_KEY_LEFT_SHIFT: key = KEY_LEFT_SHIFT; break;
      case GLFW_KEY_LEFT_ALT: key = KEY_LEFT_ALT; break;
      case GLFW_KEY_LEFT_SUPER: key = KEY_LEFT_OS; break;
      case GLFW_KEY_RIGHT_CONTROL: key = KEY_RIGHT_CONTROL; break;
      case GLFW_KEY_RIGHT_SHIFT: key = KEY_RIGHT_SHIFT; break;
      case GLFW_KEY_RIGHT_ALT: key = KEY_RIGHT_ALT; break;
      case GLFW_KEY_RIGHT_SUPER: key = KEY_RIGHT_OS; break;

      case GLFW_KEY_CAPS_LOCK: key = KEY_CAPS_LOCK; break;
      case GLFW_KEY_SCROLL_LOCK: key = KEY_SCROLL_LOCK; break;
      case GLFW_KEY_NUM_LOCK: key = KEY_NUM_LOCK; break;

      default: return;
    }
    os_button_action action = (a == GLFW_RELEASE) ? BUTTON_RELEASED : BUTTON_PRESSED;
    bool repeat = (a == GLFW_REPEAT);
    glfwState.onKeyboardEvent(action, key, scancode, repeat);
  }
}

static void onTextEvent(GLFWwindow* window, unsigned int codepoint) {
  if (glfwState.onTextEvent) {
    glfwState.onTextEvent(codepoint);
  }
}

static void onMouseButton(GLFWwindow* window, int button, int action, int mods) {
  if (glfwState.onMouseButton) {
    glfwState.onMouseButton(button, action == GLFW_PRESS);
  }
}

static void onMouseMove(GLFWwindow* window, double x, double y) {
  if (glfwState.onMouseMove) {
    glfwState.onMouseMove(x, y);
  }
}

static void onMouseWheelMove(GLFWwindow* window, double deltaX, double deltaY) {
  if (glfwState.onMouseWheelMove) {
    deltaX = (deltaX == 0.0) ? 0.0 : -deltaX;
    glfwState.onMouseWheelMove(deltaX, deltaY);
  }
}

static int convertMouseButton(os_mouse_button button) {
  switch (button) {
    case MOUSE_LEFT: return GLFW_MOUSE_BUTTON_LEFT;
    case MOUSE_RIGHT: return GLFW_MOUSE_BUTTON_RIGHT;
    default: return (int) button;
  }
}

static int convertKey(os_key key) {
  switch (key) {
    case KEY_W: return GLFW_KEY_W;
    case KEY_A: return GLFW_KEY_A;
    case KEY_S: return GLFW_KEY_S;
    case KEY_D: return GLFW_KEY_D;
    case KEY_Q: return GLFW_KEY_Q;
    case KEY_E: return GLFW_KEY_E;
    case KEY_UP: return GLFW_KEY_UP;
    case KEY_DOWN: return GLFW_KEY_DOWN;
    case KEY_LEFT: return GLFW_KEY_LEFT;
    case KEY_RIGHT: return GLFW_KEY_RIGHT;
    case KEY_LEFT_SHIFT: return GLFW_KEY_LEFT_SHIFT;
    case KEY_RIGHT_SHIFT: return GLFW_KEY_RIGHT_SHIFT;
    case KEY_ESCAPE: return GLFW_KEY_ESCAPE;
    case KEY_F5: return GLFW_KEY_F5;
    default: return GLFW_KEY_UNKNOWN;
  }
}

void os_poll_events(void) {
  if (glfwState.window) {
    glfwPollEvents();
  }
}

bool os_window_open(const os_window_config* config) {
  if (glfwState.window) {
    return true;
  }

  glfwSetErrorCallback(onError);
#ifdef __APPLE__
  glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
  if (!glfwInit()) {
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, config->resizable);

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  uint32_t width = config->width ? config->width : (uint32_t) mode->width;
  uint32_t height = config->height ? config->height : (uint32_t) mode->height;

  if (config->fullscreen) {
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
  }

  glfwState.window = glfwCreateWindow(width, height, config->title, config->fullscreen ? monitor : NULL, NULL);

  if (!glfwState.window) {
    return false;
  }

  if (config->icon.data) {
    glfwSetWindowIcon(glfwState.window, 1, &(GLFWimage) {
      .pixels = config->icon.data,
      .width = config->icon.width,
      .height = config->icon.height
    });
  }

  glfwSetWindowCloseCallback(glfwState.window, onWindowClose);
  glfwSetWindowFocusCallback(glfwState.window, onWindowFocus);
  glfwSetWindowSizeCallback(glfwState.window, onWindowResize);
  glfwSetKeyCallback(glfwState.window, onKeyboardEvent);
  glfwSetCharCallback(glfwState.window, onTextEvent);
  glfwSetMouseButtonCallback(glfwState.window, onMouseButton);
  glfwSetCursorPosCallback(glfwState.window, onMouseMove);
  glfwSetScrollCallback(glfwState.window, onMouseWheelMove);
  glfwState.width = width;
  glfwState.height = height;
  return true;
}

bool os_window_is_open(void) {
  return glfwState.window;
}

void os_window_get_size(uint32_t* width, uint32_t* height) {
  *width = glfwState.width;
  *height = glfwState.height;
}

float os_window_get_pixel_density(void) {
  if (!glfwState.window) {
    return 0.f;
  }

  int w, h, fw, fh;
  glfwGetWindowSize(glfwState.window, &w, &h);
  glfwGetFramebufferSize(glfwState.window, &fw, &fh);
  return (w == 0 || fw == 0) ? 1.f : (float) fw / w;
}

void os_on_quit(fn_quit* callback) {
  glfwState.onQuitRequest = callback;
}

void os_on_focus(fn_focus* callback) {
  glfwState.onWindowFocus = callback;
}

void os_on_resize(fn_resize* callback) {
  glfwState.onWindowResize = callback;
}

void os_on_key(fn_key* callback) {
  glfwState.onKeyboardEvent = callback;
}

void os_on_text(fn_text* callback) {
  glfwState.onTextEvent = callback;
}

void os_on_mouse_button(fn_mouse_button* callback) {
  glfwState.onMouseButton = callback;
}

void os_on_mouse_move(fn_mouse_move* callback) {
  glfwState.onMouseMove = callback;
}

void os_on_mousewheel_move(fn_mousewheel_move* callback) {
  glfwState.onMouseWheelMove = callback;
}

void os_get_mouse_position(double* x, double* y) {
  if (glfwState.window) {
    glfwGetCursorPos(glfwState.window, x, y);
  } else {
    *x = *y = 0.;
  }
}

void os_set_mouse_mode(os_mouse_mode mode) {
  if (glfwState.window) {
    int m = (mode == MOUSE_MODE_GRABBED) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
    glfwSetInputMode(glfwState.window, GLFW_CURSOR, m);
  }
}

bool os_is_mouse_down(os_mouse_button button) {
  return glfwState.window ? glfwGetMouseButton(glfwState.window, convertMouseButton(button)) == GLFW_PRESS : false;
}

bool os_is_key_down(os_key key) {
  return glfwState.window ? glfwGetKey(glfwState.window, convertKey(key)) == GLFW_PRESS : false;
}

#if defined(_WIN32)
uintptr_t os_get_win32_window(void) {
  return (uintptr_t) glfwGetWin32Window(glfwState.window);
}

uintptr_t os_get_win32_instance(void) {
  return (uintptr_t) GetModuleHandle(NULL);
}
#elif defined(__APPLE__)
uintptr_t os_get_ca_metal_layer(void) {
  id window = glfwGetCocoaWindow(glfwState.window);
  id view = msg(id, window, "contentView");
  id layer = msg(id, cls(CAMetalLayer), "layer");
  CGFloat scale = msg(CGFloat, window, "backingScaleFactor");
  msg1(void, layer, "setContentsScale:", CGFloat, scale);
  msg1(void, view, "setLayer:", id, layer);
  msg1(void, view, "setWantsLayer:", BOOL, YES);
  return (uintptr_t) layer;
}
#elif defined(__linux__) && !defined(__ANDROID__)
uintptr_t os_get_xcb_connection(void) {
  return (uintptr_t) XGetXCBConnection(glfwGetX11Display());
}

uintptr_t os_get_xcb_window(void) {
  return (uintptr_t) glfwGetX11Window(glfwState.window);
}
#endif

#ifdef _WIN32
#define OS_DLL_EXPORT __declspec(dllexport)
#else
#define OS_DLL_EXPORT __attribute__((visibility("default")))
#endif

OS_DLL_EXPORT GLFWwindow* os_get_glfw_window(void) {
  return glfwState.window;
}

#endif
