#pragma once

#include <Windows.h>
#include <strsafe.h>

namespace RESANA {

    void PrintErrorMsg(LPTSTR lpszFunction) {
        // Retrieve the system error message for the last-error code
        LPVOID lpMsgBuf;
        LPVOID lpDisplayBuf;
        DWORD dw = GetLastError();

        FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, nullptr);

        // Display the error message and exit the process
        lpDisplayBuf = (LPVOID) LocalAlloc(LMEM_ZEROINIT,
                                           (lstrlen((LPCTSTR) lpMsgBuf) +
                                            strlen((LPCTSTR) lpszFunction) + 40) *
                                           sizeof(TCHAR));
        StringCchPrintf((LPTSTR) lpDisplayBuf,
                        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                        TEXT("%s failed with error %d: %s"),
                        lpszFunction, dw, lpMsgBuf);
        RS_CORE_WARN((LPCTSTR) lpDisplayBuf);
        // MessageBox(nullptr, (LPCTSTR) lpDisplayBuf, TEXT("Error"), MB_OK);

        LocalFree(lpMsgBuf);
        LocalFree(lpDisplayBuf);
    }

}