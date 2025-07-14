#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commdlg.h>

// Control-IDs
#define IDC_EDIT_FILE     101
#define IDC_BUTTON_CHOOSE 102
#define IDC_EDIT_SHARE    103
#define IDC_EDIT_IP       104
#define IDC_BUTTON_SEND   105

static const wchar_t CLASS_NAME[] = L"NetzwerkSenderClass";

HWND hEditFile, hEditShare, hEditIP;

// Helfer: Datei-Dialog aufrufen
bool ChooseFile(HWND hwndOwner, wchar_t* outPath, DWORD maxLen) {
    OPENFILENAMEW ofn = { sizeof(ofn) };
    ofn.hwndOwner    = hwndOwner;
    ofn.lpstrFile    = outPath;
    ofn.nMaxFile     = maxLen;
    ofn.lpstrFilter  = L"Alle Dateien\0*.*\0";
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    return GetOpenFileNameW(&ofn) == TRUE;
}

// Fenster-Prozedur
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg) {
    case WM_CREATE:
        // Datei-Label + Edit + Button
        CreateWindowW(L"STATIC", L"Datei:", 
            WS_CHILD|WS_VISIBLE, 10, 10, 50,20, hwnd, NULL, NULL, NULL);
        hEditFile = CreateWindowW(L"EDIT", L"", 
            WS_CHILD|WS_VISIBLE|WS_BORDER|ES_READONLY, 70,10,300,20,
            hwnd, (HMENU)IDC_EDIT_FILE, NULL, NULL);
        CreateWindowW(L"BUTTON", L"...", 
            WS_CHILD|WS_VISIBLE, 380,10,30,20,
            hwnd, (HMENU)IDC_BUTTON_CHOOSE, NULL, NULL);

        // Freigabe-Ordner
        CreateWindowW(L"STATIC", L"Freigabe:", 
            WS_CHILD|WS_VISIBLE, 10,40,60,20, hwnd, NULL, NULL, NULL);
        hEditShare = CreateWindowW(L"EDIT", L"Freigabe", 
            WS_CHILD|WS_VISIBLE|WS_BORDER, 80,40,330,20,
            hwnd, (HMENU)IDC_EDIT_SHARE, NULL, NULL);

        // IPv4-Eingabe
        CreateWindowW(L"STATIC", L"IPv4:", 
            WS_CHILD|WS_VISIBLE, 10,70,50,20, hwnd, NULL, NULL, NULL);
        hEditIP = CreateWindowW(L"EDIT", L"192.168.1.50", 
            WS_CHILD|WS_VISIBLE|WS_BORDER, 70,70,150,20,
            hwnd, (HMENU)IDC_EDIT_IP, NULL, NULL);

        // Senden-Button
        CreateWindowW(L"BUTTON", L"Senden", 
            WS_CHILD|WS_VISIBLE, 10,110,100,30,
            hwnd, (HMENU)IDC_BUTTON_SEND, NULL, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(w) == IDC_BUTTON_CHOOSE) {
            wchar_t path[MAX_PATH] = L"";
            if (ChooseFile(hwnd, path, _countof(path))) {
                SetWindowTextW(hEditFile, path);
            }
        }
        else if (LOWORD(w) == IDC_BUTTON_SEND) {
            wchar_t src[MAX_PATH], share[64], ip[32];
            GetWindowTextW(hEditFile, src, MAX_PATH);
            GetWindowTextW(hEditShare, share, 64);
            GetWindowTextW(hEditIP, ip, 32);

            if (!*src || !*share || !*ip) {
                MessageBoxW(hwnd, L"Bitte alle Felder ausfüllen!", L"Fehler", MB_ICONEXCLAMATION);
                break;
            }

            // Dateiname extrahieren
            wchar_t* fname = wcsrchr(src, L'\\');
            fname = fname ? fname+1 : src;

            // Ziel-UNC zusammenbauen
            wchar_t dest[MAX_PATH];
            wsprintfW(dest, L"\\\\%s\\%s\\%s", ip, share, fname);

            // Kopieren
            if (CopyFileW(src, dest, FALSE)) {
                MessageBoxW(hwnd, L"✅ Übertragung erfolgreich!", L"Fertig", MB_OK);
            } else {
                wchar_t err[64];
                wsprintfW(err, L"❌ Fehler: %d", GetLastError());
                MessageBoxW(hwnd, err, L"Fehler", MB_ICONERROR);
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcW(hwnd, msg, w, l);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int nCmdShow) {
    // Klasse registrieren
    WNDCLASSW wc = { };
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(240,240,240));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    // Fenster erstellen
    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Netzwerk Datei-Sender",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 440, 200,
        NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, nCmdShow);

    // Message-Loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

