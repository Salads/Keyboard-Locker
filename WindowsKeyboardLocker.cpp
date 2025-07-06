// WindowsKeyboardLocker.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsKeyboardLocker.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

bool isLocked = false;
HWND hMainWindow = NULL;
HWND hLockButton = NULL;
HHOOK hKeyboardHook = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam);
bool ToggleKeyboardHook(bool enable);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSKEYBOARDLOCKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSKEYBOARDLOCKER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Just in case.
    if (hKeyboardHook)
    {
        UnhookWindowsHookEx(hKeyboardHook);
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSKEYBOARDLOCKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSKEYBOARDLOCKER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   int mainWindowWidth = 250;
   int mainWindowHeight = mainWindowWidth;
   int lockButtonWidth = 100;
   int lockButtonHeight = lockButtonWidth;

   int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = GetSystemMetrics(SM_CYSCREEN);
   int startX = (screenWidth / 2) - (mainWindowWidth/2);
   int startY = (screenHeight / 2) - (mainWindowHeight / 2);

   hMainWindow = CreateWindowW(szWindowClass, 
       szTitle, 
       WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
       startX, startY,
       mainWindowWidth, mainWindowHeight,
       nullptr,
       nullptr,
       hInstance,
       nullptr);

   if (!hMainWindow)
   {
      return FALSE;
   }

   RECT mainWindowRect = {};
   GetClientRect(hMainWindow, &mainWindowRect);
   int clientWidth = mainWindowRect.right - mainWindowRect.left;
   int clientHeight = mainWindowRect.bottom - mainWindowRect.top;

   int lockButtonPosX = ((clientWidth - lockButtonWidth) / 2);
   int lockButtonPosY = ((clientHeight - lockButtonHeight) / 2);

   hLockButton = CreateWindow(
       L"BUTTON",
       L"UNLOCKED",
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
       lockButtonPosX, lockButtonPosY,
       lockButtonWidth, lockButtonHeight,
       hMainWindow,
       (HMENU)IDM_LOCKBUTTON,
       (HINSTANCE)GetWindowLongPtr(hMainWindow, GWLP_HINSTANCE),
       NULL);

   ShowWindow(hMainWindow, nCmdShow);
   UpdateWindow(hMainWindow);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_LOCKBUTTON:
            {
                bool newLocked = !isLocked;
                bool result = ToggleKeyboardHook(!isLocked);
                if (result)
                {
                    if (newLocked)
                    {
                        SetWindowText(hLockButton, L"Locked");
                    }
                    else
                    {
                        SetWindowText(hLockButton, L"Unlocked");
                    }
                }
            } break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) // wParam & lParam have keyboard event
    {
        // WPARAM windowMessage = wParam;
        // KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        // We don't actually care which keys, we want to block all keys.
        // So, just return "1" (non-zero) to "eat" the input.
        return 1;

    }
    else if (nCode < 0)
    {
        // Docs say to call this if nCode < 0. 
        // First arg is ignored.
        // Ref: https://learn.microsoft.com/en-us/windows/win32/winmsg/lowlevelkeyboardproc
        return CallNextHookEx(0, nCode, wParam, lParam);
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}

bool ToggleKeyboardHook(bool enable)
{
    if (enable) 
    {
        hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
        if (hKeyboardHook == NULL)
        {
            MessageBox(0, L"Failed to set Keyboard Hook!", L"Error", MB_OK);
            return false;
        }

        isLocked = true;
    }
    else
    {
        BOOL result = UnhookWindowsHookEx(hKeyboardHook);

        if (!result)
        {
            MessageBox(0, L"Failed to unhook Keyboard Hook!", L"Error", MB_OK);
            return false;
        }

        hKeyboardHook = NULL;
        isLocked = false;
    }
    
    return true;
}
