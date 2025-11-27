
// from https://wayland-book.com/seat/example.html

// used stable xdg-shell

// Wayland does not have stable "decoration" support, xdg-decoration not supported
// zxdg_toplevel_decoration_v1 is unstable
// https://wayland-book.com/xdg-shell-in-depth/interactive.html

#include <linux/input.h>

static uint8_t local_k_map[0xff+1]={0}; //+1 to have 0xff index
static uint8_t get_ASCII_key(uint8_t key){return local_k_map[key];}

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
	xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {xdg_wm_base_ping,};

static void seatCapabilities(struct wl_seat *seat, uint32_t caps, struct app_os_window *os_window);
static void seatCapabilitiesCb(void *data, struct wl_seat *seat, uint32_t caps)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	seatCapabilities(seat, caps, os_window);
}

void registryGlobal(struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version, struct app_os_window *os_window)
{
	if (strcmp(interface, "wl_compositor") == 0) {
		os_window->compositor = (struct wl_compositor *) wl_registry_bind(registry, name, &wl_compositor_interface, 3);
	}
	else if (strcmp(interface, "xdg_wm_base") == 0)
	{
		os_window->shell = (struct xdg_wm_base *) wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(os_window->shell, &xdg_wm_base_listener, NULL);
	}
	else if (strcmp(interface, "wl_seat") == 0)
	{
    os_window->seat = (struct wl_seat *) wl_registry_bind(registry, name, &wl_seat_interface, 1);
		static const struct wl_seat_listener seat_listener ={ seatCapabilitiesCb, };
		wl_seat_add_listener(os_window->seat, &seat_listener, os_window);
	}
}

static void setSize(int width, int height, struct app_os_window *os_window)
{
	if (width <= 0 || height <= 0){
    os_window->is_minimized = true;
  }else
  {
    os_window->is_minimized = false;
    if ((os_window->app_data.iResolution[0] != width) || (os_window->app_data.iResolution[1] != height)) {
        os_window->is_minimized = false;
        os_window->app_data.iResolution[0] = width;
        os_window->app_data.iResolution[1] = height;
        if((os_window->app_data.iResolution[0]<=1)||(os_window->app_data.iResolution[1]<=1)){
            os_window->is_minimized = true;
        }
    }
  }
}

static void xdg_surface_handle_configure(void *data, struct xdg_surface *surface, uint32_t serial)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	xdg_surface_ack_configure(surface, serial);
	os_window->configured = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {xdg_surface_handle_configure,};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *toplevel, int32_t width, int32_t height, struct wl_array *states)
{
	struct app_os_window *os_window = (struct app_os_window *) data;

	setSize(width, height, os_window);
}

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	os_window->app_data.quit = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {xdg_toplevel_handle_configure, xdg_toplevel_handle_close,};

static void registryGlobalCb(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	registryGlobal(registry, name, interface, version, os_window);
}

static void pointerEnterCb(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy){}

static void pointerLeaveCb(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface){}

static void pointerMotion(struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy, struct app_os_window *os_window)
{
  os_window->app_data.iMouse[0]=wl_fixed_to_int(sx);
  os_window->app_data.iMouse[1]=os_window->app_data.iResolution[1] - wl_fixed_to_int(sy);
}

static void pointerMotionCb(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	pointerMotion(pointer, time, sx, sy, os_window);
}

static void pointerButton(struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state, struct app_os_window *os_window)
{
	switch (button)
	{
	case BTN_LEFT:
    os_window->app_data.iMouse_click[0] = state;
    if(state){
      os_window->app_data.iMouse_lclick[0] = os_window->app_data.iMouse[0];
      os_window->app_data.iMouse_lclick[1] = os_window->app_data.iMouse[1];
    }else{
      os_window->app_data.iMouse_lclick[0] = -os_window->app_data.iMouse_lclick[0];
      os_window->app_data.iMouse_lclick[1] = -os_window->app_data.iMouse_lclick[1];
    }
		break;
	case BTN_MIDDLE:
		break;
	case BTN_RIGHT:
		os_window->app_data.iMouse_click[1] = state;
    if(state){
      os_window->app_data.iMouse_rclick[0] = os_window->app_data.iMouse[0];
      os_window->app_data.iMouse_rclick[1] = os_window->app_data.iMouse[1];
    }else{
      os_window->app_data.iMouse_rclick[0] = -os_window->app_data.iMouse_rclick[0];
      os_window->app_data.iMouse_rclick[1] = -os_window->app_data.iMouse_rclick[1];
    }
		break;
	}
}

static void pointerButtonCb(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	pointerButton(pointer, serial, time, button, state, os_window);
}

static void pointerAxis(struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value, struct app_os_window *os_window)
{
	float d = wl_fixed_to_double(value);
	switch (axis)
	{
	case REL_X:
		//printf("mouse wheel %f\n",d);
		break;
	}
}

static void pointerAxisCb(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
  pointerAxis(pointer, time, axis, value, os_window);
}

static void keyboardKeymapCb(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size){}

static void keyboardEnterCb(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys){}

static void keyboardLeaveCb(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface){}

static void keyboardKey(struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state, struct app_os_window *os_window)
{
  uint8_t keyA=get_ASCII_key((uint8_t)key);
  if(state) {
      os_window->pause_refresh=true;
      keyboard_need_update=true;
      keyboard_draw=true;
      update_key_map(keyA,2,!keyboard_map[keyA][2]);
      update_key_map(keyA,1,true);
      update_key_map(keyA,0,true);
  }else {
      os_window->pause_refresh=true;
      keyboard_need_update=true;
      keyboard_draw=true;
      update_key_map(keyA,0,false);
  }
  
}

static void keyboardKeyCb(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	keyboardKey(keyboard, serial, time, key, state, os_window);
}

static void keyboardModifiersCb(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group){}

static void seatCapabilities(struct wl_seat *seat, uint32_t caps, struct app_os_window *os_window)
{
	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !os_window->pointer)
	{
		os_window->pointer = wl_seat_get_pointer(seat);
		static const struct wl_pointer_listener pointer_listener ={ pointerEnterCb, pointerLeaveCb, pointerMotionCb, pointerButtonCb, pointerAxisCb, };
		wl_pointer_add_listener(os_window->pointer, &pointer_listener, os_window);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && os_window->pointer)
	{
		wl_pointer_destroy(os_window->pointer);
		os_window->pointer = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !os_window->keyboard)
	{
		os_window->keyboard = wl_seat_get_keyboard(seat);
		static const struct wl_keyboard_listener keyboard_listener = { keyboardKeymapCb, keyboardEnterCb, keyboardLeaveCb, keyboardKeyCb, keyboardModifiersCb, };
		wl_keyboard_add_listener(os_window->keyboard, &keyboard_listener, os_window);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && os_window->keyboard)
	{
		wl_keyboard_destroy(os_window->keyboard);
		os_window->keyboard = NULL;
	}
}



static void registryGlobalRemoveCb(void *data, struct wl_registry *registry, uint32_t name){}

void initWaylandConnection(struct app_os_window *os_window)
{
	os_window->display = wl_display_connect(NULL);
	if (!os_window->display)
	{
		printf("Could not connect to Wayland display!\n");
		fflush(stdout);
		exit(1);
	}

	os_window->registry = wl_display_get_registry(os_window->display);
	if (!os_window->registry)
	{
		printf("Could not get Wayland registry!\n");
		fflush(stdout);
		exit(1);
	}

	static const struct wl_registry_listener registry_listener = { registryGlobalCb, registryGlobalRemoveCb };
	wl_registry_add_listener(os_window->registry, &registry_listener, os_window);
	wl_display_dispatch(os_window->display);
	wl_display_roundtrip(os_window->display);
	if (!os_window->compositor || !os_window->shell || !os_window->seat)
	{
		printf("Could not bind Wayland protocols!\n");
		fflush(stdout);
		exit(1);
	}
}

static void gen_key_map();
static void setupWindow(struct app_os_window *os_window)
{
	os_window->surface = wl_compositor_create_surface(os_window->compositor);
	os_window->xdg_surface = xdg_wm_base_get_xdg_surface(os_window->shell, os_window->surface);

	xdg_surface_add_listener(os_window->xdg_surface, &xdg_surface_listener, os_window);
	os_window->xdg_toplevel = xdg_surface_get_toplevel(os_window->xdg_surface);
	xdg_toplevel_add_listener(os_window->xdg_toplevel, &xdg_toplevel_listener, os_window);

	xdg_toplevel_set_title(os_window->xdg_toplevel, os_window->name);
	wl_surface_commit(os_window->surface);
  gen_key_map();
}

//shadertoy keys
static const uint8_t Key_Backspace = 8, Key_Tab = 9, Key_Enter = 13, Key_Shift = 16, Key_Space = 32,
Key_Ctrl = 17, Key_Alt = 18, Key_Pause = 19, Key_Caps = 20, Key_Escape = 27, Key_PageUp = 33, Key_PageDown = 34, Key_End = 35,
Key_Home = 36, Key_LeftArrow = 37, Key_UpArrow = 38, Key_RightArrow = 39, Key_DownArrow = 40, Key_Insert = 45, 
Key_Delete = 46, Key_0 = 48, Key_1 = 49, Key_2 = 50, Key_3 = 51, Key_4 = 52,
Key_5 = 53, Key_6 = 54, Key_7 = 55, Key_8 = 56, Key_9 = 57, Key_A = 65, Key_B = 66, 
Key_C = 67, Key_D = 68, Key_E = 69, Key_F = 70, Key_G = 71, Key_H = 72,
Key_I = 73, Key_J = 74, Key_K = 75, Key_L = 76, Key_M = 77, Key_N = 78, Key_O = 79, Key_P = 80, Key_Q = 81, 
Key_R = 82, Key_S = 83, Key_T = 84, Key_U = 85,
Key_V = 86, Key_W = 87, Key_X = 88, Key_Y = 89, Key_Z = 90, Key_LeftWindow = 91, Key_RightWindows = 92, 
Key_Select = 93, Key_Numpad0 = 96, Key_Numpad1 = 97, Key_Numpad2 = 98, Key_Numpad3 = 99,
Key_Numpad4 = 100, Key_Numpad5 = 101, Key_Numpad6 = 102, Key_Numpad7 = 103, Key_Numpad8 = 104, Key_Numpad9 = 105, 
Key_NumpadMultiply = 106, Key_NumpadAdd = 107, Key_NumpadSubtract = 109, Key_NumpadPeriod = 110, Key_NumpadDivide = 111, 
Key_F1 = 112, Key_F2 = 113, Key_F3 = 114, Key_F4 = 115, Key_F5 = 116, Key_F6 = 117, Key_F7 = 118, Key_F8 = 119, Key_F9 = 120, 
Key_F10 = 121, Key_F11 = 122, Key_F12 = 123, Key_NumLock = 144, Key_ScrollLock = 145,
Key_SemiColon = 186, Key_Equal = 187, Key_Comma = 188, Key_Dash = 189, Key_Period = 190, 
Key_ForwardSlash = 191, Key_GraveAccent = 192, Key_OpenBracket = 219, Key_BackSlash = 220, Key_CloseBraket = 221, Key_SingleQuote = 222;


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

static void gen_key_map(){
  
  uint32_t keycode=0;
  keycode=KEY_A;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_A;
  keycode=KEY_B;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_B;
  keycode=KEY_C;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_C;
  keycode=KEY_D;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_D;
  keycode=KEY_E;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_E;
  keycode=KEY_F;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F;
  keycode=KEY_G;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_G;
  keycode=KEY_H;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_H;
  keycode=KEY_I;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_I;
  keycode=KEY_J;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_J;
  keycode=KEY_K;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_K;
  keycode=KEY_L;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_L;
  keycode=KEY_M;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_M;
  keycode=KEY_N;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_N;
  keycode=KEY_O;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_O;
  keycode=KEY_P;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_P;
  keycode=KEY_Q;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Q;
  keycode=KEY_R;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_R;
  keycode=KEY_S;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_S;
  keycode=KEY_T;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_T;
  keycode=KEY_U;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_U;
  keycode=KEY_V;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_V;
  keycode=KEY_W;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_W;
  keycode=KEY_X;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_X;
  keycode=KEY_Y;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Y;
  keycode=KEY_Z;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Z;
  keycode=KEY_SPACE;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Space;
  
  keycode=KEY_0;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_0;
  keycode=KEY_1;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_1;
  keycode=KEY_2;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_2;
  keycode=KEY_3;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_3;
  keycode=KEY_4;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_4;
  keycode=KEY_5;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_5;
  keycode=KEY_6;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_6;
  keycode=KEY_7;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_7;
  keycode=KEY_8;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_8;
  keycode=KEY_9;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_9;
  
  keycode=KEY_ESC;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Escape;
  keycode=KEY_LEFTCTRL;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Ctrl;
  keycode=KEY_LEFTSHIFT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Shift;
  keycode=KEY_LEFTALT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Alt;
  //keycode=KEY_Super_L;
  //local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_LeftWindow;
  keycode=KEY_RIGHTCTRL;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Ctrl;
  keycode=KEY_RIGHTSHIFT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Shift;
  keycode=KEY_RIGHTALT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Alt;
  //keycode=KEY_Super_R;
  //local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_RightWindows;
  
  keycode=KEY_BACKSPACE;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Backspace;
  keycode=KEY_TAB;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Tab;
  keycode=KEY_ENTER;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Enter;
  keycode=KEY_PAUSE;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Pause;
  keycode=KEY_CAPSLOCK;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Caps;
  keycode=KEY_PAGEUP;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_PageUp;
  keycode=KEY_PAGEDOWN;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_PageDown;
  keycode=KEY_END;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_End;
  keycode=KEY_HOME;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Home;
  
  keycode=KEY_LEFT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_LeftArrow;
  keycode=KEY_UP;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_UpArrow;
  keycode=KEY_RIGHT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_RightArrow;
  keycode=KEY_DOWN;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_DownArrow;
  
  keycode=KEY_INSERT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Insert;
  keycode=KEY_DELETE;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Delete;
  keycode=KEY_SELECT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Select;
  
  keycode=KEY_KP0;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad0;
  keycode=KEY_KP1;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad1;
  keycode=KEY_KP2;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad2;
  keycode=KEY_KP3;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad3;
  keycode=KEY_KP4;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad4;
  keycode=KEY_KP5;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad5;
  keycode=KEY_KP6;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad6;
  keycode=KEY_KP7;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad7;
  keycode=KEY_KP8;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad8;
  keycode=KEY_KP9;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Numpad9;
  
  keycode=KEY_KPENTER;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Enter;
  keycode=KEY_KPASTERISK;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_NumpadMultiply;
  keycode=KEY_KPPLUS;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_NumpadAdd;
  keycode=KEY_KPMINUS;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_NumpadSubtract;
  keycode=KEY_KPDOT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_NumpadPeriod;
  keycode=KEY_KPSLASH;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_NumpadDivide;
  keycode=KEY_F1;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F1;
  keycode=KEY_F2;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F2;
  keycode=KEY_F3;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F3;
  keycode=KEY_F4;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F4;
  keycode=KEY_F5;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F5;
  keycode=KEY_F6;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F6;
  keycode=KEY_F7;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F7;
  keycode=KEY_F8;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F8;
  keycode=KEY_F9;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F9;
  keycode=KEY_F10;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F10;
  keycode=KEY_F11;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F11;
  keycode=KEY_F12;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_F12;
  
  keycode=KEY_NUMLOCK;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_NumLock;
  keycode=KEY_SCROLLLOCK;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_ScrollLock;
  
  keycode=KEY_SEMICOLON;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_SemiColon;
  keycode=KEY_EQUAL;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Equal;
  keycode=KEY_COMMA;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Comma;
  keycode=KEY_MINUS;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Dash;
  keycode=KEY_DOT;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_Period;
  keycode=KEY_SLASH;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_ForwardSlash;
  keycode=KEY_GRAVE;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_GraveAccent;
  keycode=KEY_LEFTBRACE;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_OpenBracket;
  keycode=KEY_BACKSLASH;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_BackSlash;
  keycode=KEY_RIGHTBRACE;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_CloseBraket;
  keycode=KEY_APOSTROPHE;
  local_k_map[(uint8_t)max(min(keycode,0xff),0)]=Key_SingleQuote;
  
}













