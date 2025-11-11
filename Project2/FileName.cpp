#include <windows.h>
#include <tchar.h>
#include <iostream>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

TCHAR WinName[] = _T("MainFrame");

// ідентифікатори пунктів меню
#define ID_RESULT_EXIT 40010
#define ID_HELP_AUTHOR 40011
#define ID_HELP_REQ   40012
#define ID_HELP_EXIT  40013
#define ID_EXIT_TOP   40014

// Global cursors / state
HCURSOR g_hArrowCursor = NULL;
HCURSOR g_hWaitCursor = NULL;
bool g_busy = false;

int APIENTRY _tWinMain(
    HINSTANCE This, // Дескриптор поточного застосунку
    HINSTANCE Prev, // У сучасних системах завжди 0
    LPTSTR cmd,     // Командний рядок
    int mode)       // Режим відтворення вікна
{
    HWND hWnd;               // Дескриптор головного вікна програми
    MSG msg;                 // Структура для збереження повідомлень
    WNDCLASS wc;             // Клас вікна
    HMENU hResultMenu, hHelpMenu, hMenu; // Дескриптори пунктів меню
    HACCEL hAccel = NULL;

    // Load cursors to satisfy "arrow and small hourglass" requirement
    g_hArrowCursor = LoadCursor(NULL, IDC_ARROW);
    g_hWaitCursor  = LoadCursor(NULL, IDC_WAIT);

    // Визначити клас вікна
    ZeroMemory(&wc, sizeof(wc));
    wc.hInstance = This;
    wc.lpszClassName = WinName;                    // Ім’я класу вікна
    wc.lpfnWndProc = WndProc;                      // Функція вікна
    wc.style = CS_HREDRAW | CS_VREDRAW;            // Стиль вікна
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);    // піктограма застосунку за умовчанням
    wc.hCursor = g_hArrowCursor;                   // стандартна стрілка (class default)
    wc.lpszMenuName = NULL;                        // Немає меню класу
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Заповнити вікно білим кольором

    if (!RegisterClass(&wc))
        return 0; // Зарегіструвати клас вікна

    // Створити вікно: лівий верхній кут (0,0), розміри 300x500
    hWnd = CreateWindow(
        WinName,                          // Ім’я класу вікна
        _T("Windows app"),                // Заголовок вікна
        WS_OVERLAPPEDWINDOW,              // заголовок і рамка з обрамленням
        0,                                // x лівого верхнього кута
        0,                                // y лівого верхнього кута
        300,                              // ширина в точках
        500,                              // висота в точках
        HWND_DESKTOP,                     // дескриптор батьківського вікна
        NULL,                             // меню підключимо пізніше
        This,                             // дескриптор застосунку
        NULL);                            // додаткової інформації немає

    if (!hWnd)
    {
        MessageBox(NULL, _T("CreateWindow failed"), _T("Error"), MB_ICONERROR);
        return 0;
    }

    // Створити підменю "Результат"
    hResultMenu = CreatePopupMenu();
    AppendMenu(hResultMenu, MF_STRING, ID_RESULT_EXIT, _T("&Вихід з програми\tCtrl+R"));

    // Створити підменю "Довідка"
    hHelpMenu = CreatePopupMenu();
    AppendMenu(hHelpMenu, MF_STRING, ID_HELP_AUTHOR, _T("&Автор програми\tF1"));
    AppendMenu(hHelpMenu, MF_STRING, ID_HELP_REQ, _T("&Вимоги до програми\tCtrl+A"));
    AppendMenu(hHelpMenu, MF_STRING, ID_HELP_EXIT, _T("В&ихід з програми\tCtrl+Q"));

    // Створити головне меню в потрібному порядку:
    // 1) "Результат", 2) "Довідка", 3) "Вихід з програми"
    hMenu = CreateMenu();
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hResultMenu, _T("&Результат"));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, _T("&Довідка"));
    AppendMenu(hMenu, MF_STRING, ID_EXIT_TOP, _T("&Вихід з програми\tCtrl+Q"));

    SetMenu(hWnd, hMenu);   // Підключити меню до вікна

    // Створити таблицю гарячих клавіш (акселераторів)
    ACCEL accel[] = {
        { FCONTROL, (WORD)'R', ID_RESULT_EXIT },          // Ctrl+R -> Результат->Вихід
        { FVIRTKEY, VK_F1,   ID_HELP_AUTHOR },            // F1 -> Автор програми
        { FCONTROL, (WORD)'A', ID_HELP_REQ },             // Ctrl+A -> Вимоги до програми
        { FCONTROL, (WORD)'Q', ID_EXIT_TOP }              // Ctrl+Q -> верхній пункт Вихід
    };
    hAccel = CreateAcceleratorTable(accel, sizeof(accel) / sizeof(ACCEL));

    // Requirement: ShowWindow call must be with "hidden" state
    ShowWindow(hWnd, SW_HIDE);    // kept as required

    // Use AnimateWindow with AW_BLEND to show the window smoothly (satisfies AW_BLEND requirement)
    // AnimateWindow without AW_HIDE shows the window even if it was hidden.
    AnimateWindow(hWnd, 300, AW_BLEND);

    // Ensure the window has focus so accelerators work
    UpdateWindow(hWnd);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);

    // Цикл обробки повідомлень з підтримкою акселераторів
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (hAccel == NULL || TranslateAccelerator(hWnd, hAccel, &msg) == 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (hAccel)
        DestroyAcceleratorTable(hAccel);

    return (int)msg.wParam;
}

// Віконна функція викликається операційною системою
// та отримує повідомлення з черги для даного застосунку
LRESULT CALLBACK WndProc(
    HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SETCURSOR:
        // Provide arrow normally, hourglass when busy
        if (g_busy)
        {
            SetCursor(g_hWaitCursor);
            return TRUE; // handled
        }
        // let DefWindowProc set the normal cursor (class cursor)
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_RESULT_EXIT:
            // show wait cursor, animate hide, then quit
            g_busy = true;
            SetCursor(g_hWaitCursor);
            UpdateWindow(hWnd);
            AnimateWindow(hWnd, 300, AW_BLEND | AW_HIDE);
            PostQuitMessage(0);
            g_busy = false;
            return 0;

        case ID_HELP_AUTHOR:
            // коротке повідомлення про автора
            g_busy = true;
            SetCursor(g_hWaitCursor);
            MessageBox(hWnd, _T("Автор програми: Андрій Бартошек"), _T("Автор"), MB_OK | MB_ICONINFORMATION);
            g_busy = false;
            SetCursor(g_hArrowCursor);
            return 0;

        case ID_HELP_REQ:
            g_busy = true;
            SetCursor(g_hWaitCursor);
            MessageBox(hWnd, _T("Вимоги до програми:\n- Windows\n- Базова конфігурація"), _T("Вимоги"), MB_OK | MB_ICONINFORMATION);
            g_busy = false;
            SetCursor(g_hArrowCursor);
            return 0;

        case ID_HELP_EXIT:
            g_busy = true;
            SetCursor(g_hWaitCursor);
            AnimateWindow(hWnd, 300, AW_BLEND | AW_HIDE);
            PostQuitMessage(0);
            g_busy = false;
            return 0;

        case ID_EXIT_TOP:
            g_busy = true;
            SetCursor(g_hWaitCursor);
            AnimateWindow(hWnd, 300, AW_BLEND | AW_HIDE);
            PostQuitMessage(0);
            g_busy = false;
            return 0;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
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