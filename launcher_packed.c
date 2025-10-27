#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

/* Resource ID for embedded exe */
#define IDR_MAIN_EXE 101

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    
    char tempPath[MAX_PATH];
    char exePath[MAX_PATH];
    
    /* Get temp directory */
    if (GetTempPathA(MAX_PATH, tempPath) == 0) {
        MessageBoxA(NULL, "Failed to get temp directory", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    /* Create unique temp exe path using process ID and tick count */
    DWORD pid = GetCurrentProcessId();
    DWORD tick = GetTickCount();
    wsprintfA(exePath, "%sLVGL_Windows_Demo_%u_%u.exe", tempPath, pid, tick);
    
    /* Load embedded exe from resources */
    HRSRC hRes = FindResourceA(hInstance, MAKEINTRESOURCEA(IDR_MAIN_EXE), RT_RCDATA);
    if (!hRes) {
        MessageBoxA(NULL, "Failed to find embedded executable", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    HGLOBAL hResData = LoadResource(hInstance, hRes);
    if (!hResData) {
        MessageBoxA(NULL, "Failed to load embedded executable", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    void* pData = LockResource(hResData);
    DWORD dwSize = SizeofResource(hInstance, hRes);
    
    /* Write embedded exe to temp file */
    HANDLE hFile = CreateFileA(exePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Failed to create temporary file", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    DWORD written;
    WriteFile(hFile, pData, dwSize, &written, NULL);
    CloseHandle(hFile);
    
    /* Launch the extracted exe */
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    ZeroMemory(&pi, sizeof(pi));
    
    if (!CreateProcessA(exePath, NULL, NULL, NULL, FALSE, DETACHED_PROCESS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        MessageBoxA(NULL, "Failed to launch application", "Error", MB_OK | MB_ICONERROR);
        DeleteFileA(exePath);
        return 1;
    }
    
    /* Wait for the process to finish */
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    /* Cleanup */
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    DeleteFileA(exePath);
    
    return 0;
}
