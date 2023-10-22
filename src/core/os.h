#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#pragma once

typedef struct os_window_config {
  uint32_t width;
  uint32_t height;
  bool fullscreen;
  bool resizable;
  const char* title;
  struct {
    void* data;
    uint32_t width;
    uint32_t height;
  } icon;
} os_window_config;

typedef enum {
  MOUSE_LEFT,
  MOUSE_RIGHT
} os_mouse_button;

typedef enum {
  MOUSE_MODE_NORMAL,
  MOUSE_MODE_GRABBED
} os_mouse_mode;

typedef enum {
  KEY_A,
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G,
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q,
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z,
  KEY_0,
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
  KEY_SPACE,
  KEY_ENTER,
  KEY_TAB,
  KEY_ESCAPE,
  KEY_BACKSPACE,
  KEY_UP,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_HOME,
  KEY_END,
  KEY_PAGE_UP,
  KEY_PAGE_DOWN,
  KEY_INSERT,
  KEY_DELETE,
  KEY_F1,
  KEY_F2,
  KEY_F3,
  KEY_F4,
  KEY_F5,
  KEY_F6,
  KEY_F7,
  KEY_F8,
  KEY_F9,
  KEY_F10,
  KEY_F11,
  KEY_F12,
  KEY_BACKTICK,
  KEY_MINUS,
  KEY_EQUALS,
  KEY_LEFT_BRACKET,
  KEY_RIGHT_BRACKET,
  KEY_BACKSLASH,
  KEY_SEMICOLON,
  KEY_APOSTROPHE,
  KEY_COMMA,
  KEY_PERIOD,
  KEY_SLASH,
  KEY_LEFT_CONTROL,
  KEY_LEFT_SHIFT,
  KEY_LEFT_ALT,
  KEY_LEFT_OS,
  KEY_RIGHT_CONTROL,
  KEY_RIGHT_SHIFT,
  KEY_RIGHT_ALT,
  KEY_RIGHT_OS,
  KEY_CAPS_LOCK,
  KEY_SCROLL_LOCK,
  KEY_NUM_LOCK,
  KEY_COUNT
} os_key;

typedef enum {
  BUTTON_PRESSED,
  BUTTON_RELEASED
} os_button_action;

typedef enum {
  OS_PERMISSION_AUDIO_CAPTURE
} os_permission;

typedef void fn_quit(void);
typedef void fn_focus(bool focused);
typedef void fn_resize(uint32_t width, uint32_t height);
typedef void fn_key(os_button_action action, os_key key, uint32_t scancode, bool repeat);
typedef void fn_text(uint32_t codepoint);
typedef void fn_mouse_button(int button, bool pressed);
typedef void fn_mouse_move(double x, double y);
typedef void fn_mousewheel_move(double deltaX, double deltaY);
typedef void fn_permission(os_permission permission, bool granted);

bool os_init(void);
void os_destroy(void);
const char* os_get_name(void);
uint32_t os_get_core_count(void);
void os_open_console(void);
double os_get_time(void);
void os_sleep(double seconds);
void os_request_permission(os_permission permission);

void* os_vm_init(size_t size);
bool os_vm_free(void* p, size_t size);
bool os_vm_commit(void* p, size_t size);
bool os_vm_release(void* p, size_t size);

void os_thread_attach(void);
void os_thread_detach(void);

void os_poll_events(void);
void os_on_quit(fn_quit* callback);
void os_on_focus(fn_focus* callback);
void os_on_resize(fn_resize* callback);
void os_on_key(fn_key* callback);
void os_on_text(fn_text* callback);
void os_on_mouse_button(fn_mouse_button* callback);
void os_on_mouse_move(fn_mouse_move* callback);
void os_on_mousewheel_move(fn_mousewheel_move* callback);
void os_on_permission(fn_permission* callback);

bool os_window_open(const os_window_config* config);
bool os_window_is_open(void);
void os_window_get_size(uint32_t* width, uint32_t* height);
float os_window_get_pixel_density(void);
void os_window_message_box(const char* message);

size_t os_get_home_directory(char* buffer, size_t size);
size_t os_get_data_directory(char* buffer, size_t size);
size_t os_get_working_directory(char* buffer, size_t size);
size_t os_get_executable_path(char* buffer, size_t size);
size_t os_get_bundle_path(char* buffer, size_t size, const char** root);

void os_get_mouse_position(double* x, double* y);
void os_set_mouse_mode(os_mouse_mode mode);
bool os_is_mouse_down(os_mouse_button button);
bool os_is_key_down(os_key key);

uintptr_t os_get_win32_window(void);
uintptr_t os_get_win32_instance(void);

uintptr_t os_get_ca_metal_layer(void);

uintptr_t os_get_xcb_connection(void);
uintptr_t os_get_xcb_window(void);
