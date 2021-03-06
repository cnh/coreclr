//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
//
/***
*makepath.c - create path name from components
*

*
*Purpose:
*       To provide support for creation of full path names from components
*
*******************************************************************************/
#include "stdafx.h"
#include "winwrap.h"
#include "utilcode.h"
#include "ex.h"

/***
*void _makepath() - build path name from components
*
*Purpose:
*       create a path name from its individual components
*
*Entry:
*       WCHAR *path  - pointer to buffer for constructed path
*       WCHAR *drive - pointer to drive component, may or may not contain
*                     trailing ':'
*       WCHAR *dir   - pointer to subdirectory component, may or may not include
*                     leading and/or trailing '/' or '\' characters
*       WCHAR *fname - pointer to file base name component
*       WCHAR *ext   - pointer to extension component, may or may not contain
*                     a leading '.'.
*
*Exit:
*       path - pointer to constructed path name
*
*Exceptions:
*
*******************************************************************************/

void MakePath (
        __out_ecount (MAX_PATH) register WCHAR *path,
        __in LPCWSTR drive,
        __in LPCWSTR dir,
        __in LPCWSTR fname,
        __in LPCWSTR ext
        )
{
        CONTRACTL
        {
            NOTHROW;
            GC_NOTRIGGER;
            FORBID_FAULT;
        }
        CONTRACTL_END

        register const WCHAR *p;
        register DWORD count = 0;

        /* we assume that the arguments are in the following form (although we
         * do not diagnose invalid arguments or illegal filenames (such as
         * names longer than 8.3 or with illegal characters in them)
         *
         *  drive:
         *      A           ; or
         *      A:
         *  dir:
         *      \top\next\last\     ; or
         *      /top/next/last/     ; or
         *      either of the above forms with either/both the leading
         *      and trailing / or \ removed.  Mixed use of '/' and '\' is
         *      also tolerated
         *  fname:
         *      any valid file name
         *  ext:
         *      any valid extension (none if empty or null )
         */

        /* copy drive */

        if (drive && *drive) {
                *path++ = *drive;
                *path++ = _T(':');
                count += 2;
        }

        /* copy dir */

        if ((p = dir)) {
                while (*p) {
                        *path++ = *p++;
                        count++;

                        if (count == MAX_PATH) {
                            --path;
                            *path = _T('\0');
                            return;
                        }
                }

#ifdef _MBCS
                if (*(p=_mbsdec(dir,p)) != _T('/') && *p != _T('\\')) {
#else  /* _MBCS */
                // suppress warning for the following line; this is safe but would require significant code
                // delta for prefast to understand.
#ifdef _PREFAST_
                #pragma warning( suppress: 26001 ) 
#endif
                if (*(p-1) != _T('/') && *(p-1) != _T('\\')) {
#endif  /* _MBCS */
                        *path++ = _T('\\');
                        count++;

                        if (count == MAX_PATH) {
                            --path;
                            *path = _T('\0');
                            return;
                        }
                }
        }

        /* copy fname */

        if ((p = fname)) {
                while (*p) {
                        *path++ = *p++;
                        count++;

                        if (count == MAX_PATH) {
                            --path;
                            *path = _T('\0');
                            return;
                        }
                }
        }

        /* copy ext, including 0-terminator - check to see if a '.' needs
         * to be inserted.
         */

        if ((p = ext)) {
                if (*p && *p != _T('.')) {
                        *path++ = _T('.');
                        count++;

                        if (count == MAX_PATH) {
                            --path;
                            *path = _T('\0');
                            return;
                        }
                }

                while ((*path++ = *p++)) {
                    count++;

                    if (count == MAX_PATH) {
                        --path;
                        *path = _T('\0');
                        return;
                    }
                }
        }
        else {
                /* better add the 0-terminator */
                *path = _T('\0');
        }
}

#if !defined(FEATURE_CORECLR)
static LPCWSTR g_wszProcessExePath = NULL;

HRESULT GetProcessExePath(LPCWSTR *pwszProcessExePath)
{
    CONTRACTL
    {
        NOTHROW;
        GC_NOTRIGGER;
        CONSISTENCY_CHECK(CheckPointer(pwszProcessExePath));
    }
    CONTRACTL_END;

    HRESULT hr = S_OK;

    if (g_wszProcessExePath == NULL)
    {
        NewArrayHolder<WCHAR> wszProcName = new (nothrow) WCHAR[_MAX_PATH];
        IfNullRet(wszProcName);

        DWORD cchProcName = WszGetModuleFileName(NULL, wszProcName, _MAX_PATH);
        if (cchProcName == 0)
        {
            return HRESULT_FROM_GetLastError();
        }

        if (InterlockedCompareExchangeT(&g_wszProcessExePath, const_cast<LPCWSTR>(wszProcName.GetValue()), NULL) == NULL)
        {
            wszProcName.SuppressRelease();
        }
    }
    _ASSERTE(g_wszProcessExePath != NULL);
    _ASSERTE(SUCCEEDED(hr));

    *pwszProcessExePath = g_wszProcessExePath;
    return hr;
}
#endif

