#include <xcb/xcb_keysyms.h>

static uint8_t local_k_map[0xff+1]={0}; //+1 to have 0xff index
static uint8_t get_ASCII_key(uint8_t key){return local_k_map[key];}
static void get_modifiers(uint32_t mask, bool *c_lock, bool *n_lock, bool *s_lock);
static uint8_t check_num(bool nmod,uint8_t key);

static void app_handle_xcb_event(struct app_os_window *os_window, const xcb_generic_event_t *event, bool (*save_map)[3], bool (*state_map)[2]) {
    uint8_t event_code = event->response_type & 0x7f;
    
    switch (event_code) {
    case XCB_EXPOSE:
        break;
    case XCB_CLIENT_MESSAGE:
        if ((*(xcb_client_message_event_t *)event).data.data32[0] == (*os_window->atom_wm_delete_window).atom) {
            os_window->app_data.quit = true;
        }
        break;
    case XCB_KEY_RELEASE: {
        const xcb_key_release_event_t *key = (const xcb_key_release_event_t *)event;
        uint8_t keyA=get_ASCII_key((uint8_t)key->detail);
        if(keyA>0){
            keyboard_need_update=true;
            keyboard_draw=true;
            bool a,b,c;
            get_modifiers(key->state,&a,&b,&c);
            keyA=check_num(b,keyA);
            if((!state_map[keyA][0])&&(!state_map[keyA][1])){
                save_map[keyA][0]=keyboard_map[keyA][0];
                save_map[keyA][1]=keyboard_map[keyA][1];
                save_map[keyA][2]=keyboard_map[keyA][2];
            }
            state_map[keyA][0]=true;
            update_key_map(keyA,0,false);
        }
    } break;
    case XCB_KEY_PRESS: {
        const xcb_key_press_event_t *key = (const xcb_key_press_event_t *)event;
        uint8_t keyA=get_ASCII_key((uint8_t)key->detail);
        if(keyA>0){
            bool a,b,c;
            get_modifiers(key->state,&a,&b,&c);
            keyA=check_num(b,keyA);
            if((!state_map[keyA][0])&&(!state_map[keyA][1])){
                save_map[keyA][0]=keyboard_map[keyA][0];
                save_map[keyA][1]=keyboard_map[keyA][1];
                save_map[keyA][2]=keyboard_map[keyA][2];
            }
            state_map[keyA][1]=true;
            
            if(!keyboard_map[keyA][0]){
                keyboard_need_update=true;
                keyboard_draw=true;
                update_key_map(keyA,2,!keyboard_map[keyA][2]);
                update_key_map(keyA,1,true);
                update_key_map(keyA,0,true);
            }
        }
    } break;
    case XCB_MOTION_NOTIFY: {
        const xcb_motion_notify_event_t *ev = (const xcb_motion_notify_event_t *)event;
        os_window->app_data.iMouse[0] = ev->event_x;
        os_window->app_data.iMouse[1] = os_window->app_data.iResolution[1] - ev->event_y;
    } break;
    case XCB_BUTTON_PRESS: {
        const xcb_button_press_event_t *ev = (const xcb_button_press_event_t *)event;
        switch (ev->detail) {
        case 1:
            os_window->app_data.iMouse_click[0] = true;
            os_window->app_data.iMouse_lclick[0] = ev->event_x;
            os_window->app_data.iMouse_lclick[1] = os_window->app_data.iResolution[1] - ev->event_y;
            break;
        case 3:
            os_window->app_data.iMouse_click[1] = true;
            os_window->app_data.iMouse_rclick[0] = ev->event_x;
            os_window->app_data.iMouse_rclick[1] = os_window->app_data.iResolution[1] - ev->event_y;
            break;
        case 4:
            /*printf ("Wheel Button up in window %ld, at coordinates (%d,%d)\n",
            ev->event, ev->event_x, ev->event_y);*/
            break;
        case 5:
            /*printf ("Wheel Button down in window %ld, at coordinates (%d,%d)\n",
            ev->event, ev->event_x, ev->event_y);*/
            break;
        //default:
            /*printf ("Button %d pressed in window %ld, at coordinates (%d,%d)\n",
            ev->detail, ev->event, ev->event_x, ev->event_y);*/
        }
    } break;
    case XCB_BUTTON_RELEASE: {
        const xcb_button_release_event_t *ev = (const xcb_button_release_event_t *)event;
        switch (ev->detail) {
        case 1:
            os_window->app_data.iMouse_click[0] = false;
            os_window->app_data.iMouse_lclick[0] = -os_window->app_data.iMouse_lclick[0];
            os_window->app_data.iMouse_lclick[1] = -os_window->app_data.iMouse_lclick[1];
            break;
        case 3:
            os_window->app_data.iMouse_click[1] = false;
            os_window->app_data.iMouse_rclick[0] = -os_window->app_data.iMouse_rclick[0];
            os_window->app_data.iMouse_rclick[1] = -os_window->app_data.iMouse_rclick[1];
            break;
        }
        /*print_modifiers(ev->state);

        printf ("Button %d released in window %ld, at coordinates (%d,%d)\n",
        ev->detail, ev->event, ev->event_x, ev->event_y);*/
    } break;
    
    case XCB_CONFIGURE_NOTIFY: {
        const xcb_configure_notify_event_t *cfg = (const xcb_configure_notify_event_t *)event;
        if ((os_window->app_data.iResolution[0] != cfg->width) || (os_window->app_data.iResolution[1] != cfg->height)) {
            os_window->is_minimized = false;
            os_window->app_data.iResolution[0] = cfg->width;
            os_window->app_data.iResolution[1] = cfg->height;
            if((os_window->app_data.iResolution[0]<=1)||(os_window->app_data.iResolution[1]<=1)){
                os_window->is_minimized = true;
            }else{
                os_window->resize_event=true;
            }
        }
    } break;
    default:
        break;
    }
}

static void app_init_connection(struct app_os_window *os_window) {
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    const char *display_envar = getenv("DISPLAY");
    if (display_envar == NULL || display_envar[0] == '\0') {
        printf("Environment variable DISPLAY requires a valid value.\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    os_window->connection = xcb_connect(NULL, &scr);
    if (xcb_connection_has_error(os_window->connection) > 0) {
        printf("Cannot find a compatible Vulkan installable client driver (ICD).\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    setup = xcb_get_setup(os_window->connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0) xcb_screen_next(&iter);

    os_window->screen = iter.data;
}

static void gen_key_map(xcb_connection_t* connection);
static void app_create_xcb_window(struct app_os_window *os_window) {
    uint32_t value_mask, value_list[32];
    
    os_window->xcb_window = xcb_generate_id(os_window->connection);
    
    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = os_window->screen->black_pixel;
    
    value_list[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_create_window(os_window->connection, XCB_COPY_FROM_PARENT, os_window->xcb_window, os_window->screen->root, 0, 0, os_window->app_data.iResolution[0], os_window->app_data.iResolution[1],
        0, XCB_WINDOW_CLASS_INPUT_OUTPUT, os_window->screen->root_visual, value_mask, value_list);

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(os_window->connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(os_window->connection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(os_window->connection, 0, 16, "WM_DELETE_WINDOW");
    os_window->atom_wm_delete_window = xcb_intern_atom_reply(os_window->connection, cookie2, 0);
    
    xcb_change_property (os_window->connection, XCB_PROP_MODE_REPLACE, os_window->xcb_window,
                       XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                       strlen (os_window->name), os_window->name);
    
    xcb_change_property (os_window->connection, XCB_PROP_MODE_REPLACE, os_window->xcb_window,
                       XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
                       strlen(os_window->name), os_window->name);
    
    xcb_change_property(os_window->connection, XCB_PROP_MODE_REPLACE, os_window->xcb_window, (*reply).atom, 4, 32, 1,
        &(*os_window->atom_wm_delete_window).atom);
    free(reply);

    xcb_map_window(os_window->connection, os_window->xcb_window);

    const uint32_t coords[] = { 100, 100 };
    xcb_configure_window(os_window->connection, os_window->xcb_window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
    xcb_flush(os_window->connection);
    gen_key_map(os_window->connection);
}

static void get_modifiers(uint32_t mask, bool *c_lock, bool *n_lock, bool *s_lock)
{
    const char **mod, *mods[] = {
        "Shift", "Lock", "Ctrl", "Alt",
        "Mod2", "Mod3", "Mod4", "Mod5",
        "Button1", "Button2", "Button3", "Button4", "Button5"
    };
    //printf("Modifier mask: ");
    int cntr=0;
    *c_lock=false;*n_lock=false;*s_lock=false;
    for (mod = mods; mask; mask >>= 1, mod++){
        //if (mask & 1)printf("%s ",*mod);
        if ((mask & 1)&&(cntr==1))*c_lock=true;
        if ((mask & 1)&&(cntr==4))*n_lock=true;
        cntr++;
    }
    //putchar('\n');
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

// this is not ASCII keys, this used to detect keys when NumLock pressed/not
#define Key_npx0 239
#define Key_npx1 240
#define Key_npx2 241
#define Key_npx3 242
#define Key_npx4 243
#define Key_npx5 244
#define Key_npx6 245
#define Key_npx7 246
#define Key_npx8 247
#define Key_npx9 248
#define Key_npx10 249

static uint8_t check_num(bool nmod,uint8_t key){
    if(nmod){
        switch(key){
            case Key_npx0:return Key_Numpad0;
            case Key_npx1:return Key_Numpad1;
            case Key_npx2:return Key_Numpad2;
            case Key_npx3:return Key_Numpad3;
            case Key_npx4:return Key_Numpad4;
            case Key_npx5:return Key_Numpad5;
            case Key_npx6:return Key_Numpad6;
            case Key_npx7:return Key_Numpad7;
            case Key_npx8:return Key_Numpad8;
            case Key_npx9:return Key_Numpad9;
            case Key_npx10:return Key_NumpadPeriod;
        }
    }else{
        switch(key){
            case Key_npx0:return Key_Insert;
            case Key_npx1:return Key_End;
            case Key_npx2:return Key_DownArrow;
            case Key_npx3:return Key_PageDown;
            case Key_npx4:return Key_LeftArrow;
            case Key_npx5:return 0;
            case Key_npx6:return Key_RightArrow;
            case Key_npx7:return Key_Home;
            case Key_npx8:return Key_UpArrow;
            case Key_npx9:return Key_PageUp;
            case Key_npx10:return Key_Delete;
        }
    }
    return key;
}

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// keyboard mapping can be wrong on some keys(not tested) when used some unique keyboards with swapped keys order
// example where first(main) key is <@> and only second is <2>, I expect that keyboard has first(main) key is <2>
static void gen_key_map(xcb_connection_t* connection){
    xcb_key_symbols_t* keySymbols = xcb_key_symbols_alloc(connection);
    xcb_keycode_t* keycode=NULL;
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_A);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_A;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_B);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_B;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_C);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_C;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_D);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_D;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_E);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_E;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_G);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_G;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_H);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_H;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_I);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_I;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_J);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_J;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_K);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_K;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_L);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_L;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_M);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_M;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_N);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_N;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_O);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_O;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_P);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_P;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Q);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Q;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_R);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_R;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_S);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_S;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_T);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_T;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_U);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_U;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_V);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_V;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_W);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_W;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_X);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_X;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Y);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Y;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Z);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Z;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_space);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Space;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_0);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_0;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_1);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_1;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_2);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_2;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_3);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_3;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_4);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_4;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_5);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_5;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_6);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_6;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_7);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_7;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_8);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_8;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_9);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_9;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Escape);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Escape;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Control_L);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Ctrl;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Shift_L);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Shift;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Alt_L);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Alt;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Super_L);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_LeftWindow;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Control_R);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Ctrl;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Shift_R);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Shift;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Alt_R);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Alt;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Super_R);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_RightWindows;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_BackSpace);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Backspace;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Tab);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Tab;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Return);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Enter;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Pause);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Pause;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Caps_Lock);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Caps;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Page_Up);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_PageUp;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Page_Down);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_PageDown;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_End);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_End;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Home);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Home;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Left);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_LeftArrow;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Up);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_UpArrow;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Right);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_RightArrow;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Down);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_DownArrow;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Insert);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Insert;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Delete);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Delete;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Select);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Select;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_0);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad0;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_1);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad1;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_2);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad2;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_3);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad3;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_4);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad4;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_5);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad5;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_6);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad6;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_7);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad7;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_8);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad8;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_9);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Numpad9;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Space);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Space;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Tab);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Tab;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Home);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx7;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Left);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx4;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Up);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx8;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Right);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx6;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Down);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx2;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Page_Up);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx9;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Page_Down);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx3;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_End);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx1;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Insert);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx0;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Delete);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_npx10;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Enter);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Enter;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Multiply);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_NumpadMultiply;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Add);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_NumpadAdd;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Subtract);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_NumpadSubtract;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Separator);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_NumpadPeriod;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_KP_Divide);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_NumpadDivide;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F1);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F1;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F2);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F2;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F3);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F3;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F4);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F4;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F5);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F5;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F6);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F6;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F7);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F7;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F8);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F8;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F9);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F9;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F10);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F10;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F11);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F11;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_F12);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_F12;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Num_Lock);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_NumLock;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_Scroll_Lock);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_ScrollLock;free(keycode);
    
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_semicolon);
    if(keycode)if(keycode[1]>0)local_k_map[(uint8_t)max(min(keycode[1],0xff),0)]=Key_SemiColon;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_equal);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Equal;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_comma);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Comma;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_minus);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Dash;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_period);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_Period;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_slash);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_ForwardSlash;
    if(keycode)if(keycode[1]>0)local_k_map[(uint8_t)max(min(keycode[1],0xff),0)]=Key_ForwardSlash;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_grave);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_GraveAccent;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_bracketleft);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_OpenBracket;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_backslash);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_BackSlash;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_bracketright);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_CloseBraket;free(keycode);
    keycode=xcb_key_symbols_get_keycode(keySymbols, XK_apostrophe);
    if(keycode)local_k_map[(uint8_t)max(min(keycode[0],0xff),0)]=Key_SingleQuote;free(keycode);
    
    xcb_key_symbols_free( keySymbols );
}

