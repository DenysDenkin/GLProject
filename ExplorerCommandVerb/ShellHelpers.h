#pragma once

#define STRICT_TYPED_ITEMIDS   
#define UNICODE 1

#include <windows.h>
#include <windowsx.h>          
#include <shlobj.h>             
#include <shlwapi.h>            
#include <propkey.h>
#include <propvarutil.h>
#include <strsafe.h>
#include <objbase.h>

#pragma comment(lib, "shlwapi.lib")    
#pragma comment(lib, "comctl32.lib")    
#pragma comment(lib, "propsys.lib")     

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

__inline HRESULT ResultFromKnownLastError() { const DWORD err = GetLastError(); return err == ERROR_SUCCESS ? E_FAIL : HRESULT_FROM_WIN32(err); }


__inline HRESULT ResultFromWin32Bool(BOOL b)
{
    return b ? S_OK : ResultFromKnownLastError();
}

#if (NTDDI_VERSION >= NTDDI_VISTA)

__inline HRESULT ShellExecuteItem(HWND hwnd, PCWSTR pszVerb, IShellItem *psi)
{
    PIDLIST_ABSOLUTE pidl;
    HRESULT hr = SHGetIDListFromObject(psi, &pidl);
    if (SUCCEEDED(hr))
    {
        SHELLEXECUTEINFO ei = { sizeof(ei) };
        ei.fMask = SEE_MASK_INVOKEIDLIST;
        ei.hwnd = hwnd;
        ei.nShow = SW_NORMAL;
        ei.lpIDList = pidl;
        ei.lpVerb = pszVerb;

        hr = ResultFromWin32Bool(ShellExecuteEx(&ei));

        CoTaskMemFree(pidl);
    }
    return hr;
}

__inline HRESULT GetItemFromView(IFolderView2 *pfv, int iItem, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = S_OK;

    if (iItem == -1)
    {
        hr = pfv->GetSelectedItem(-1, &iItem); // Returns S_FALSE if none selected
    }

    if (S_OK == hr)
    {
        hr = pfv->GetItem(iItem, riid, ppv);
    }
    else
    {
        hr = E_FAIL;
    }
    return hr;
}

__inline void SetDialogIcon(HWND hdlg, SHSTOCKICONID siid)
{
    SHSTOCKICONINFO sii = {sizeof(sii)};
    if (SUCCEEDED(SHGetStockIconInfo(siid, SHGFI_ICON | SHGFI_SMALLICON, &sii)))
    {
        SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM) sii.hIcon);
    }
    if (SUCCEEDED(SHGetStockIconInfo(siid, SHGFI_ICON | SHGFI_LARGEICON, &sii)))
    {
        SendMessage(hdlg, WM_SETICON, ICON_BIG, (LPARAM) sii.hIcon);
    }
}
#endif

__inline void ClearDialogIcon(HWND hdlg)
{
    DestroyIcon((HICON)SendMessage(hdlg, WM_GETICON, ICON_SMALL, 0));
    DestroyIcon((HICON)SendMessage(hdlg, WM_GETICON, ICON_BIG, 0));
}

__inline HRESULT SetItemImageImageInStaticControl(HWND hwndStatic, IShellItem *psi)
{
    HBITMAP hbmp = NULL;
    HRESULT hr = S_OK;
    if (psi)
    {
        IShellItemImageFactory *psiif;
        hr = psi->QueryInterface(IID_PPV_ARGS(&psiif));
        if (SUCCEEDED(hr))
        {
            RECT rc;
            GetWindowRect(hwndStatic, &rc);
            const UINT dxdy = min(rc.right - rc.left, rc.bottom - rc.top);    // make it square
            const SIZE size = { dxdy, dxdy };

            hr = psiif->GetImage(size, SIIGBF_RESIZETOFIT, &hbmp);
            psiif->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        HGDIOBJ hgdiOld = (HGDIOBJ) SendMessage(hwndStatic, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) hbmp);
        if (hgdiOld)
        {
            DeleteObject(hgdiOld);
        }
    }

    return hr;
}


__inline HRESULT SHILCloneFull(PCUIDLIST_ABSOLUTE pidl, PIDLIST_ABSOLUTE *ppidl)
{
    *ppidl = ILCloneFull(pidl);
    return *ppidl ? S_OK : E_OUTOFMEMORY;
}

__inline HRESULT SHILClone(PCUIDLIST_RELATIVE pidl, PIDLIST_RELATIVE *ppidl)
{
    *ppidl = ILClone(pidl);
    return *ppidl ? S_OK : E_OUTOFMEMORY;
}

__inline HRESULT SHILCombine(PCIDLIST_ABSOLUTE pidl1, PCUIDLIST_RELATIVE pidl2, PIDLIST_ABSOLUTE *ppidl)
{
    *ppidl = ILCombine(pidl1, pidl2);
    return *ppidl ? S_OK : E_OUTOFMEMORY;
}

__inline HRESULT GetItemAt(IShellItemArray *psia, DWORD i, REFIID riid, void **ppv)
{
    *ppv = NULL;
    IShellItem *psi = NULL;  
    HRESULT hr = psia ? psia->GetItemAt(i, &psi) : E_NOINTERFACE;
    if (SUCCEEDED(hr))
    {
        hr = psi->QueryInterface(riid, ppv);
        psi->Release();
    }
    return hr;
}

#define MAP_ENTRY(x) {L#x, x}

__inline HRESULT ShellAttributesToString(SFGAOF sfgaof, PWSTR *ppsz)
{
    *ppsz = NULL;

    static const struct { PCWSTR pszName; SFGAOF sfgaof; } c_rgItemAttributes[] =
    {
        MAP_ENTRY(SFGAO_STREAM),
        MAP_ENTRY(SFGAO_FOLDER),
        MAP_ENTRY(SFGAO_FILESYSTEM),
        MAP_ENTRY(SFGAO_FILESYSANCESTOR),
        MAP_ENTRY(SFGAO_STORAGE),
        MAP_ENTRY(SFGAO_STORAGEANCESTOR),
        MAP_ENTRY(SFGAO_LINK),
        MAP_ENTRY(SFGAO_CANCOPY),
        MAP_ENTRY(SFGAO_CANMOVE),
        MAP_ENTRY(SFGAO_CANLINK),
        MAP_ENTRY(SFGAO_CANRENAME),
        MAP_ENTRY(SFGAO_CANDELETE),
        MAP_ENTRY(SFGAO_HASPROPSHEET),
        MAP_ENTRY(SFGAO_DROPTARGET),
        MAP_ENTRY(SFGAO_ENCRYPTED),
        MAP_ENTRY(SFGAO_ISSLOW),
        MAP_ENTRY(SFGAO_GHOSTED),
        MAP_ENTRY(SFGAO_SHARE),
        MAP_ENTRY(SFGAO_READONLY),
        MAP_ENTRY(SFGAO_HIDDEN),
        MAP_ENTRY(SFGAO_REMOVABLE),
        MAP_ENTRY(SFGAO_COMPRESSED),
        MAP_ENTRY(SFGAO_BROWSABLE),
        MAP_ENTRY(SFGAO_NONENUMERATED),
        MAP_ENTRY(SFGAO_NEWCONTENT),
    };

    WCHAR sz[512] = {};
    PWSTR psz = sz;
    size_t cch = ARRAYSIZE(sz);

    StringCchPrintfEx(psz, cch, &psz, &cch, 0, L"0x%08X", sfgaof);

    for (int i = 0; i < ARRAYSIZE(c_rgItemAttributes); i++)
    {
        if (c_rgItemAttributes[i].sfgaof & sfgaof)
        {
            StringCchPrintfEx(psz, cch, &psz, &cch, 0, L", %s", c_rgItemAttributes[i].pszName);
        }
    }
    return SHStrDup(sz, ppsz);
}

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


template <class T> HRESULT SetInterface(T **ppT, IUnknown *punk)
{
    SafeRelease(ppT);
    return punk ? punk->QueryInterface(ppT) : E_NOINTERFACE;
}

__inline void DisableComExceptionHandling()
{
    IGlobalOptions *pGlobalOptions;
    HRESULT hr =  CoCreateInstance(CLSID_GlobalOptions, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pGlobalOptions));
    if (SUCCEEDED(hr))
    {
#if (NTDDI_VERSION >= NTDDI_WIN7)
        hr = pGlobalOptions->Set(COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE_ANY);
#else
        hr = pGlobalOptions->Set(COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE);
#endif
        pGlobalOptions->Release();
    }
}

__inline void GetWindowRectInClient(HWND hwnd, RECT *prc)
{
    GetWindowRect(hwnd, prc);
    MapWindowPoints(GetDesktopWindow(), GetParent(hwnd), (POINT*)prc, 2);
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
__inline HINSTANCE GetModuleHINSTANCE() { return (HINSTANCE)&__ImageBase; }
