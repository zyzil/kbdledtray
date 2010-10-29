/*
Copyright (c) 2010, Kevin Mullins
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Includes
#include <winsdkver.h>
#if !defined(_WIN32_WINNT)
	#define _WIN32_WINNT 0x0501
#endif
#include <sdkddkver.h>
#if !defined(WIN32_LEAN_AND_MEAN)
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "KbdLedTrayHook.h"

// Vars
HINSTANCE m_hInstance = NULL;
HHOOK m_hKeyboardHook = NULL;
HOOKCALLBACK m_hookCallback = NULL;

// Forward decs
LRESULT CALLBACK KeyboardProc(int, WPARAM, LPARAM);

// Entry point
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		m_hInstance = hinstDLL;

	return TRUE;
}

// Hook proc
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	m_hookCallback(nCode, wParam, lParam);
	return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam);
}

// Exported methods
#if defined(__cplusplus)
extern "C" {
#endif

DWORD __declspec(dllexport) InstallHook(HOOKCALLBACK callback)
{
	if (!callback)
		return ERROR_INVALID_PARAMETER;

	m_hookCallback = callback;

	m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, m_hInstance, 0);
	if (!m_hKeyboardHook)
		return GetLastError();

	return 0;
}

void __declspec(dllexport) RemoveHook()
{
	UnhookWindowsHookEx(m_hKeyboardHook);
}

#if defined (__cplusplus)
}
#endif