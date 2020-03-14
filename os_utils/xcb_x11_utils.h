
static void print_modifiers(uint32_t mask)
{
	const char **mod, *mods[] = {
		"Shift", "Lock", "Ctrl", "Alt",
		"Mod2", "Mod3", "Mod4", "Mod5",
		"Button1", "Button2", "Button3", "Button4", "Button5"
	};
	printf("Modifier mask: ");
	for (mod = mods; mask; mask >>= 1, mod++)
		if (mask & 1)
			printf(*mod);
	putchar('\n');
}

static void app_handle_xcb_event(struct app_os_window *os_window, const xcb_generic_event_t *event) {
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
		switch (key->detail) {
		case 0x9:  // Escape
			os_window->app_data.quit = true;
			break;
		case 0x71:  // left arrow key
			break;
		case 0x72:  // right arrow key
			break;
		case 0x41:  // space bar
			os_window->app_data.pause = !os_window->app_data.pause;
			break;
		case 0xa: //1
			os_window->app_data.drawdebug = !os_window->app_data.drawdebug;
			break;
        case 0xb: //2
			os_window->fps_lock = !os_window->fps_lock;
			break;
		}
	} break;
	case XCB_MOTION_NOTIFY: {
		const xcb_motion_notify_event_t *ev = (const xcb_motion_notify_event_t *)event;
		//printf ("Mouse moved in window %ld, at coordinates (%d,%d)\n",ev->event, ev->event_x, ev->event_y);
        os_window->app_data.iMouse[0] = ev->event_x;
        os_window->app_data.iMouse[1] = os_window->app_data.iResolution[1] - ev->event_y;
	} break;
	case XCB_BUTTON_PRESS: {
		const xcb_button_press_event_t *ev = (const xcb_button_press_event_t *)event;
		//print_modifiers(ev->state);
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
            os_window->app_data.iMouse_rclick[1] = -os_window->app_data.iMouse_rclick[0];
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
            if((os_window->app_data.iResolution[0]==0)||(os_window->app_data.iResolution[1]==0)){
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
}
