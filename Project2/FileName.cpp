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

    // Визначити клас вікна
    ZeroMemory(&wc, sizeof(wc));
    wc.hInstance = This;
    wc.lpszClassName = WinName;                    // Ім’я класу вікна
    wc.lpfnWndProc = WndProc;                      // Функція вікна
    wc.style = CS_HREDRAW | CS_VREDRAW;            // Стиль вікна
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);    // піктограма застосунку за умовчанням
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // стандартна стрілка
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

    // --- Змінено тут: показати вікно (не ховати) та застосувати анімацію AW_BLEND ---
    ShowWindow(hWnd, SW_SHOW);                 // раніше було SW_HIDE — саме це заважало вікну відкриватися
    UpdateWindow(hWnd);
    // плавне проявлення (AW_BLEND) — потребує user32 (за замовчуванням лінкується)
    AnimateWindow(hWnd, 300, AW_BLEND);

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
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_RESULT_EXIT:
            // показати пісочний годинник, а потім анімовано сховати вікно й вийти
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            UpdateWindow(hWnd);
            AnimateWindow(hWnd, 300, AW_BLEND | AW_HIDE);
            PostQuitMessage(0);
            break;

        case ID_HELP_AUTHOR:
            // коротке повідомлення про автора
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            MessageBox(hWnd, _T("Автор програми: Ваше ім'я"), _T("Автор"), MB_OK | MB_ICONINFORMATION);
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;

        case ID_HELP_REQ:
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            MessageBox(hWnd, _T("Вимоги до програми:\n- Windows\n- Базова конфігурація"), _T("Вимоги"), MB_OK | MB_ICONINFORMATION);
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;

        case ID_HELP_EXIT:
            // вихід з програми з допомогою меню Довідка
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            AnimateWindow(hWnd, 300, AW_BLEND | AW_HIDE);
            PostQuitMessage(0);
            break;

        case ID_EXIT_TOP:
            // верхній пункт "Вихід з програми"
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            AnimateWindow(hWnd, 300, AW_BLEND | AW_HIDE);
            PostQuitMessage(0);
            break;

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