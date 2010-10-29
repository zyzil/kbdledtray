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

#if !defined(KBDLEDTRAYHOOK_H_INCLUDED)
#define KBDLEDTRAYHOOK_H_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER > 1020)
	#pragma once
#endif

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

// Typedefs
typedef void (CALLBACK *HOOKCALLBACK)(int, WPARAM, LPARAM);
typedef DWORD (*INSTALLHOOK)(HOOKCALLBACK);
typedef void (*REMOVEHOOK)();

// Exported methods
#if defined(__cplusplus)
extern "C" {
#endif

DWORD __declspec(dllexport) InstallHook(HOOKCALLBACK callback);
void __declspec(dllexport) RemoveHook();

#if defined (__cplusplus)
}
#endif

#endif // KBDLEDTRAYHOOK_H_INCLUDED