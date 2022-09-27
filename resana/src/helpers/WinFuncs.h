#pragma once

#include "core/Log.h"

#include <Windows.h>
#include <strsafe.h>

namespace RESANA {

static void PrintErrorMsg(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, nullptr);

    // Display the error message and exit the process
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + strlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    RS_CORE_WARN((LPCTSTR)lpDisplayBuf);
    // MessageBox(nullptr, (LPCTSTR) lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

static void PrintWin32Error(const TCHAR* msg)
{
    DWORD eNum;
    TCHAR sysMsg[256];
    TCHAR* p;

    eNum = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, eNum,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        sysMsg, 256, nullptr);

    // Trim the end of the line and terminate it with a null
    p = sysMsg;
    while ((*p > 31) || (*p == 9)) {
        ++p;
    }
    do {
        *p-- = 0;
    } while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

    // Display the message
    RS_CORE_WARN("  WARNING: {0} failed with error {1} ({2})", msg, eNum, sysMsg);
}

static uint64_t FileTimeToInt64(const FILETIME& ft)
{
    return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}

}