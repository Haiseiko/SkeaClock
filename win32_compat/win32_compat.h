#pragma once

// Minimal win32 compatibility shim for initial Android port.
// Provides a small subset of Win32 types and functions used by SkeaClock.
// This is intentionally minimal: functions are stubs or simple cross-platform
// implementations to allow compilation and iterative porting.

#include <string>
#include <cstdint>
#include <chrono>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* HANDLE;
typedef void* HINSTANCE;
typedef void* HICON;
typedef void* HWND;
typedef void* HDC;

typedef uint32_t UINT;
typedef int32_t  BOOL;
typedef uint64_t ULONGLONG;
typedef uint32_t DWORD;
typedef intptr_t LPARAM;
typedef uintptr_t WPARAM;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

struct POINT { int x; int y; };
struct RECT { int left; int top; int right; int bottom; };

struct SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
};

// Timer callback type used by SetTimer in the original app.
typedef void (CALLBACK *TIMERPROC)(HWND, UINT, UINT, DWORD);

// Basic functions implemented as stubs or simple equivalents.
void GetLocalTime(SYSTEMTIME* out);

HANDLE CreateMutexW(void* security, BOOL initialOwner, const wchar_t* name);
void CloseHandle(HANDLE h);
DWORD GetLastError();

// Simple module/file helpers
DWORD GetModuleFileNameW(HINSTANCE, wchar_t* buffer, DWORD size);

// Message box -> logs to stdout (no UI on Android stage)
int MessageBoxW(HWND, const wchar_t* text, const wchar_t* caption, unsigned int type);

// INI helpers
int GetPrivateProfileIntW(const wchar_t* section, const wchar_t* key, int defaultValue, const wchar_t* filename);
int GetPrivateProfileStringW(const wchar_t* section, const wchar_t* key, const wchar_t* defaultValue, wchar_t* out, int outSize, const wchar_t* filename);
bool WritePrivateProfileStringW(const wchar_t* section, const wchar_t* key, const wchar_t* value, const wchar_t* filename);

// Minimal cursor/window functions (stubs)
void SetCursor(void* cursor);
void GetCursorPos(POINT* p);
BOOL GetClientRect(HWND, RECT*);

// Window creation / message loop stubs
ATOM RegisterClassW(const void*);
HWND CreateWindowExW(unsigned long exStyle, const wchar_t* className, const wchar_t* windowName,
    unsigned long style, int x, int y, int width, int height, HWND parent, void* menu, HINSTANCE hInstance, void* param);

// Timers: simple non-repeating implementation using std::thread in cpp file.
UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);
void KillTimer(HWND hWnd, UINT_PTR uIDEvent);

#ifdef __cplusplus
}
#endif
