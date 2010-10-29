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
#include <stdio.h>
#include <stdlib.h>
#include <ShellAPI.h>

// Constants
const UINT WM_USER_NOTIFYICON = WM_USER + 1;
const UINT IDM_EXIT = 1001;
const UINT IDT_KBDTIMER = 1;

// Vars
HINSTANCE m_hInstance = NULL;
HWND m_hWnd = NULL;
HANDLE m_hMutex = NULL;
HMENU m_hMenu = NULL;

// Forward decs
HMENU CreateTaskbarMenu();
void ErrorExit(DWORD);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HICON DrawCustomIcon(bool, bool, bool);
void UpdateTrayIcon(bool);

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Mutex
	m_hMutex = CreateMutex(NULL, TRUE, L"KbdLedTrayProcessInstance");
	if (!m_hMutex)
		ErrorExit(GetLastError());

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, L"Already running.", L"KbdLedTray", MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	// Register class
	WNDCLASSEX wcex = {0};
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_VREDRAW | CS_HREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance;
	wcex.lpszClassName	= L"KbdLedTrayWnd";
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hCursor		= LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	RegisterClassEx(&wcex);

	// Create window
	m_hInstance = hInstance;
	m_hWnd = CreateWindow(wcex.lpszClassName, L"KbdLedTrayWnd", WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!m_hWnd) 
		ErrorExit(GetLastError());

	// Setup menu
	m_hMenu = CreateTaskbarMenu();
	if (!m_hMenu)
		ErrorExit(GetLastError());

	// Start timer for monitoring keys
	SetTimer(m_hWnd, IDT_KBDTIMER, 250, NULL);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Cleanup
	KillTimer(m_hWnd, IDT_KBDTIMER);
	UpdateTrayIcon(true);
	DestroyMenu(m_hMenu);
	ReleaseMutex(m_hMutex);

	return (int)msg.wParam;
}

HMENU CreateTaskbarMenu()
{
	HMENU hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_ENABLED | MF_STRING, IDM_EXIT, L"E&xit");
	return hMenu;
}

void ErrorExit(DWORD dwError)
{
	wchar_t szError[256];
	swprintf_s(szError, L"Exiting due to error: %d.", dwError);
	MessageBox(NULL, szError, L"KbdLedTray", MB_OK | MB_ICONERROR);
	exit(1);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_COMMAND:
		if (HIWORD(wParam) == 0 && LOWORD(wParam) == IDM_EXIT)
			PostQuitMessage(0);

		break;
	
	case WM_TIMER:
		if (wParam == IDT_KBDTIMER)
			UpdateTrayIcon(false);

		break;

	case WM_USER_NOTIFYICON:
		if (lParam == WM_RBUTTONUP)
		{
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hWnd);
			TrackPopupMenu(m_hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0, 0);
		}

		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

HICON DrawCustomIcon(bool capsOn, bool numOn, bool scrollOn)
{
	// Setup
	HDC hAndDC = CreateCompatibleDC(NULL);
	HDC hXorDC = CreateCompatibleDC(NULL);

	HBITMAP hAndBitmap = CreateCompatibleBitmap(hAndDC, 16, 16);
	HBITMAP hXorBitmap = CreateBitmap(16, 16, 1, 32, NULL);

	HBITMAP hOldAndBitmap = (HBITMAP)SelectObject(hAndDC, hAndBitmap);
	HBITMAP hOldXorBitmap = (HBITMAP)SelectObject(hXorDC, hXorBitmap);

	// AND mask
	PatBlt(hAndDC, 0, 0, 16, 16, BLACKNESS);	

	// XOR mask
	RECT rcBack = {0, 0, 16, 16};
	RECT rcCaps = {2, 8, 4, 14};
	RECT rcNum = {7, 8, 9, 14};
	RECT rcScroll = {12, 8, 14, 14};

	HBRUSH hBackBrush = CreateSolidBrush(RGB(64, 64, 64));
	HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH hGreenBrush = CreateSolidBrush(RGB(0, 255, 0));

	FillRect(hXorDC, &rcBack, hBackBrush);
	FillRect(hXorDC, &rcCaps, capsOn ? hGreenBrush : hBlackBrush);
	FillRect(hXorDC, &rcNum, numOn ? hGreenBrush : hBlackBrush);
	FillRect(hXorDC, &rcScroll, scrollOn ? hGreenBrush : hBlackBrush);

	int nOldMode = SetBkMode(hXorDC, TRANSPARENT);
	COLORREF crOld = SetTextColor(hXorDC, RGB(255, 255, 255));
	int nFontHeight = -MulDiv(5, GetDeviceCaps(hXorDC, LOGPIXELSY), 72);
	HFONT hFont = CreateFont(nFontHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, L"Small Fonts");
	HFONT hOldFont = (HFONT)SelectObject(hXorDC, hFont);
	TextOut(hXorDC, 0, -1, L"A", 1);
	TextOut(hXorDC, 6, -1, L"1", 1);
	HFONT hWingDingsFont = CreateFont(nFontHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, L"Wingdings");
	SelectObject(hXorDC, hWingDingsFont);
	TextOut(hXorDC, 11, -1, L"ô", 1);

	// Cleanup
	SelectObject(hXorDC, hOldFont);
	SetTextColor(hXorDC, crOld);
	SetBkMode(hXorDC, nOldMode);

	DeleteObject(hFont);
	DeleteObject(hWingDingsFont);
	DeleteObject(hBackBrush);
	DeleteObject(hBlackBrush);
	DeleteObject(hGreenBrush);

	SelectObject(hAndDC, hOldAndBitmap);
	SelectObject(hXorDC, hOldXorBitmap);

	DeleteDC(hAndDC);
	DeleteDC(hXorDC);

	// Create icon
	ICONINFO icon = {0};
	icon.fIcon = TRUE;
	icon.hbmMask = hAndBitmap;
	icon.hbmColor = hXorBitmap;
	HICON hIcon = CreateIconIndirect(&icon);

	// Final cleanup
	DeleteObject(hAndBitmap);
	DeleteObject(hXorBitmap);

	return hIcon;
}

void UpdateTrayIcon(bool remove)
{
	static bool iconRegistered = false;
	
	static bool capsOnPrevious = false;
	static bool numOnPrevious = false;
	static bool scrollOnPrevious = false;

	bool capsOn = (GetKeyState(VK_CAPITAL) & 1) == 1;
	bool numOn = (GetKeyState(VK_NUMLOCK) & 1) == 1;
	bool scrollOn = (GetKeyState(VK_SCROLL) & 1) == 1;

	if (remove || !iconRegistered ||
		capsOn != capsOnPrevious || numOn != numOnPrevious || scrollOn != scrollOnPrevious)
	{
		capsOnPrevious = capsOn;
		numOnPrevious = numOn;
		scrollOnPrevious = scrollOn;

		NOTIFYICONDATA icon = {0};
		icon.cbSize = sizeof(NOTIFYICONDATA);
		icon.hWnd = m_hWnd;
		icon.uCallbackMessage = WM_USER_NOTIFYICON;
		icon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		icon.hIcon = DrawCustomIcon(capsOn, numOn, scrollOn);	
		InvalidateRect(m_hWnd, NULL, TRUE);

		swprintf_s(icon.szTip, L"Caps Lock: %s\nNum Lock: %s\nScroll Lock: %s", 
			(capsOn ? L"ON" : L"OFF"), (numOn ? L"ON" : L"OFF"), (scrollOn ? L"ON" : L"OFF"));

		DWORD message = remove 
			? NIM_DELETE 
			: iconRegistered
				? NIM_MODIFY : 0;		

		Shell_NotifyIcon(message, &icon);
	
		if (!iconRegistered)
			iconRegistered = true;
	}
}