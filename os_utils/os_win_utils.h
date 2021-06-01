
void process_msg(MSG *msg, bool *done){
    static bool mouse_click[2]={false, false}; //click once control
    if (msg->message == WM_QUIT)
    {
        *done = true;
    }
    else {
        TranslateMessage(msg);
        DispatchMessage(msg);
    }
    if (msg->message == WM_KEYDOWN) {
        os_window.pause_refresh=true;
        if((msg->wParam>0)&&(msg->wParam<0xff)&&(((((msg->lParam)>>30))-(((msg->lParam)>>31)<<1))<1)){
            keyboard_need_update=true;
            keyboard_draw=true;
            update_key_map(msg->wParam,2,!keyboard_map[msg->wParam][2]);
            update_key_map(msg->wParam,1,true);
            update_key_map(msg->wParam,0,true);
        }
    }
    if (msg->message == WM_KEYUP) {
        os_window.pause_refresh=true;
        if((msg->wParam>0)&&(msg->wParam<0xff)){
            keyboard_need_update=true;
            keyboard_draw=true;
            update_key_map(msg->wParam,0,false);
        }
    }
    
    if (msg->message == WM_MOUSEMOVE) {
        os_window.pause_refresh=true;
        os_window.app_data.iMouse[0] = (int)LOWORD(msg->lParam);
        os_window.app_data.iMouse[1] = os_window.app_data.iResolution[1] - (int)HIWORD(msg->lParam);
        switch (msg->wParam)
        {
        case WM_LBUTTONDOWN:os_window.app_data.iMouse_click[0] = true; break;
        case WM_MBUTTONDOWN:break;
        case WM_RBUTTONDOWN:os_window.app_data.iMouse_click[1] = true; break;
        }
        switch (msg->wParam)
        {
        case WM_LBUTTONUP:os_window.app_data.iMouse_click[0] = false; break;
        case WM_MBUTTONUP:break;
        case WM_RBUTTONUP:os_window.app_data.iMouse_click[1] = false; break;
        }
    }
    else {

        switch (msg->message)
        {
            case WM_LBUTTONDOWN:os_window.app_data.iMouse_click[0] = true; os_window.pause_refresh=true; break;
            case WM_MBUTTONDOWN:break;
            case WM_RBUTTONDOWN:os_window.app_data.iMouse_click[1] = true; os_window.pause_refresh=true; break;
        }
        switch (msg->message)
        {
            case WM_LBUTTONUP:os_window.app_data.iMouse_click[0] = false; os_window.pause_refresh=true; break;
            case WM_MBUTTONUP:break;
            case WM_RBUTTONUP:os_window.app_data.iMouse_click[1] = false; os_window.pause_refresh=true; break;
        }
    }
    if(!os_window.app_data.iMouse_click[0]&&(!mouse_click[0])){
        
    }else{
        if(os_window.app_data.iMouse_click[0]&&(!mouse_click[0])){
            mouse_click[0]=true;
            os_window.app_data.iMouse_lclick[0]=os_window.app_data.iMouse[0];
            os_window.app_data.iMouse_lclick[1]=os_window.app_data.iMouse[1];
        }else{
            if(!os_window.app_data.iMouse_click[0]&&(mouse_click[0])){
                mouse_click[0]=false;
                os_window.app_data.iMouse_lclick[0]=-abs(os_window.app_data.iMouse_lclick[0]);
                os_window.app_data.iMouse_lclick[1]=-abs(os_window.app_data.iMouse_lclick[1]);
            }
        }
    }
    if(!os_window.app_data.iMouse_click[1]&&(!mouse_click[1])){
        
    }else{
        if(os_window.app_data.iMouse_click[1]&&(!mouse_click[1])){
            mouse_click[1]=true;
            os_window.app_data.iMouse_rclick[0]=os_window.app_data.iMouse[0];
            os_window.app_data.iMouse_rclick[1]=os_window.app_data.iMouse[1];
        }else{
            if(!os_window.app_data.iMouse_click[1]&&(mouse_click[1])){
                mouse_click[1]=false;
                os_window.app_data.iMouse_rclick[0]=-abs(os_window.app_data.iMouse_rclick[0]);
                os_window.app_data.iMouse_rclick[1]=-abs(os_window.app_data.iMouse_rclick[1]);
            }
        }
    }
    
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    chWnd = hWnd;
    switch (uMsg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        check_hotkeys(&os_window);
        if((!os_window.is_minimized)&&(!os_window.resize_event)&&((!os_window.app_data.pause)||((os_window.app_data.pause)&&(os_window.pause_refresh)))){
            if(!os_window.app_data.quit){
                os_window.app_data.quit=!render_loop_draw(&phy_dev, &dev, &swapchain, &os_window);
                update_keypress();
            }
            else PostQuitMessage(0);
        }else{
            if((!os_window.is_minimized)&&os_window.resize_event){
                on_window_resize(&phy_dev, &dev, &essentials, &swapchain, &render_data, &os_window); //execute draw or resize per frame, not together
            }
        }
        
        if(os_window.is_minimized||(((os_window.app_data.pause)&&(!os_window.pause_refresh)))){ //I do not delete everything on minimize, only stop rendering
            sleep_ms(16);
        }
        break;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lParam)->ptMinTrackSize = os_window.minsize;
        return 0;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            int tsize[2]={0};
            tsize[0] = lParam & 0xffff;
            tsize[1] = (lParam & 0xffff0000) >> 16;
            os_window.is_minimized = false;
            if((tsize[0]!=os_window.app_data.iResolution[0])||(tsize[1]!=os_window.app_data.iResolution[1])){
                os_window.app_data.iResolution[0]=tsize[0];
                os_window.app_data.iResolution[1]=tsize[1];
                if((os_window.app_data.iResolution[0]==0)||(os_window.app_data.iResolution[1]==0)){
                    os_window.is_minimized = true;
                }else{
                    os_window.resize_event=true;
                }
            }
        }
        else{
            if (wParam == SIZE_MINIMIZED){
                os_window.is_minimized = true;
            }
        }
        break;
    default:
        break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

static void app_create_window(struct app_os_window *os_window) {
    WNDCLASSEX win_class;

    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = WndProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = os_window->connection;
    win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = NULL;
    win_class.lpszClassName = os_window->name;
    win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    if (!RegisterClassEx(&win_class)) {
        printf("Unexpected error trying to start the application!\n");
        fflush(stdout);
        exit(1);
    }
    RECT wr = { 0, 0, os_window->app_data.iResolution[0], os_window->app_data.iResolution[1] };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    os_window->window = CreateWindowEx(0,
        os_window->name,            // class name
        os_window->name,            // app name
        WS_OVERLAPPEDWINDOW |  // window style
        WS_VISIBLE | WS_SYSMENU,
        100, 100,            // x/y coords
        wr.right - wr.left,  // width
        wr.bottom - wr.top,  // height
        NULL,                // handle to parent
        NULL,                // handle to menu
        os_window->connection,    // hInstance
        NULL);               // no extra parameters
    if (!os_window->window) {
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }
    os_window->minsize.x = GetSystemMetrics(SM_CXMINTRACK);
    os_window->minsize.y = GetSystemMetrics(SM_CYMINTRACK) + 1;
}

#include <io.h>
static void SetStdOutToNewConsole()
{
    static bool once=false;
    if(once)return;
    once=true;
    AllocConsole();
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    int fileDescriptor = _open_osfhandle((intptr_t)consoleHandle, _A_SYSTEM);
    FILE *fp = _fdopen( fileDescriptor, "w" );
    *stdout = *fp;
    setvbuf( stdout, NULL, _IONBF, 0 );
    SetConsoleTitle((LPCSTR)"Debug Output");
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if ( GetConsoleScreenBufferInfo(consoleHandle, &csbi) )
    {
    COORD bufferSize;
    bufferSize.X = csbi.dwSize.X;
    bufferSize.Y = 9999;
    SetConsoleScreenBufferSize(consoleHandle, bufferSize);
    }
}
