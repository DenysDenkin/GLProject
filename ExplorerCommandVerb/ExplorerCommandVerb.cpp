#include "Dll.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <functional>
#include "Threadpool.h"

static WCHAR const c_szVerbDisplayName[] = L"Avid the best";
static WCHAR const c_szVerbName[] = L"TestVerb.ExplorerCommandVerb";
struct FileInfo {
	PWSTR Name;
	UINT64 Size;
	DWORD Bytesum;
	SYSTEMTIME DateCr;
	PWSTR Path;
};

class CExplorerCommandVerb : public IExplorerCommand,
                             public IInitializeCommand,
                             public IObjectWithSite
{
public:
    CExplorerCommandVerb() : _cRef(1), _punkSite(NULL), _hwnd(NULL), _pstmShellItemArray(NULL)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CExplorerCommandVerb, IExplorerCommand),      
            QITABENT(CExplorerCommandVerb, IInitializeCommand),    
            QITABENT(CExplorerCommandVerb, IObjectWithSite),      
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }


    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IExplorerCommand
    IFACEMETHODIMP GetTitle(IShellItemArray * , LPWSTR *ppszName)
    {
        return SHStrDup(c_szVerbDisplayName, ppszName);
    }

    IFACEMETHODIMP GetIcon(IShellItemArray * , LPWSTR *ppszIcon)
    {
        *ppszIcon = NULL;
        return E_NOTIMPL;
    }

    IFACEMETHODIMP GetToolTip(IShellItemArray * , LPWSTR *ppszInfotip)
    {
        *ppszInfotip = NULL;
        return E_NOTIMPL;
    }

    IFACEMETHODIMP GetCanonicalName(GUID* pguidCommandName)
    {
        *pguidCommandName = __uuidof(this);
        return S_OK;
    }


    IFACEMETHODIMP GetState(IShellItemArray * , BOOL fOkToBeSlow, EXPCMDSTATE *pCmdState)
    {
        HRESULT hr;
        if (fOkToBeSlow)
        {
            *pCmdState = ECS_ENABLED;
            hr = S_OK;
        }
        else
        {
            *pCmdState = ECS_DISABLED;
            hr = S_OK;
        }
        return hr;
    }

    IFACEMETHODIMP Invoke(IShellItemArray *psiItemArray, IBindCtx *pbc);

	void Analyze(FileInfo &fileinfo);

    IFACEMETHODIMP GetFlags(EXPCMDFLAGS *pFlags)
    {
        *pFlags = ECF_DEFAULT;
        return S_OK;
    }

    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand **ppEnum)
    {
        *ppEnum = NULL;
        return E_NOTIMPL;
    }

    // IInitializeCommand
    IFACEMETHODIMP Initialize(PCWSTR , IPropertyBag * )
    {
        return S_OK;
    }

	std::ofstream hi;

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown *punkSite)
    {
        SetInterface(&_punkSite, punkSite);
        return S_OK;
    }

    IFACEMETHODIMP GetSite(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        return _punkSite ? _punkSite->QueryInterface(riid, ppv) : E_FAIL;
    }

private:
    ~CExplorerCommandVerb()
    {
        SafeRelease(&_punkSite);
        SafeRelease(&_pstmShellItemArray);
        DllRelease();
    }

    DWORD _ThreadProc();

    static DWORD __stdcall s_ThreadProc(void *pv)
    {
        CExplorerCommandVerb *pecv = (CExplorerCommandVerb *)pv;
        const DWORD ret = pecv->_ThreadProc();
        pecv->Release();
        return ret;
    }

    long _cRef;
    IUnknown *_punkSite;
    HWND _hwnd;
    IStream *_pstmShellItemArray;
};

DWORD CExplorerCommandVerb::_ThreadProc()
{
    IShellItemArray *psia;
    HRESULT hr = CoGetInterfaceAndReleaseStream(_pstmShellItemArray, IID_PPV_ARGS(&psia));
    _pstmShellItemArray = NULL;
    if (SUCCEEDED(hr))
    {
        DWORD count;
		FILETIME tempft;
		PWSTR path;

        psia->GetCount(&count);

		std::vector<IShellItem2*> items(count, nullptr);
		std::vector<FileInfo> fileinfo(count);
		std::vector<PWSTR> names(count);
		hr = GetItemAt(psia, 0, IID_PPV_ARGS(&items[0]));
		for (int i = 1; i < count && SUCCEEDED(hr); i++)
			hr = GetItemAt(psia, i, IID_PPV_ARGS(&items[i]));

        if (SUCCEEDED(hr))
        {
			for (int i = 0; i < count; i++) {
				hr = items[i]->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &fileinfo[i].Name);
				if (!SUCCEEDED(hr))
					break;
				hr = items[i]->GetUInt64(PKEY_Size, &fileinfo[i].Size);
				if (!SUCCEEDED(hr))
					break;
				hr = items[i]->GetFileTime(PKEY_DateCreated, &tempft);
				if (!SUCCEEDED(hr))
					break;
				::FileTimeToSystemTime(&tempft, &fileinfo[i].DateCr);

				hr = items[i]->GetString(PKEY_ParsingPath, &fileinfo[i].Path);
				if (!SUCCEEDED(hr))
					break;

			}
			Threadpool fts;
			for (int i = 0; i < count; i++) {
				fts.AddTask(std::bind(&CExplorerCommandVerb::Analyze, this, std::ref(fileinfo[i])));
			}
			fts.WaitForAll();
	/*		for (int i = 0; i < count; i++) {
				hr = items[i]->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &fileinfo[i].Name);
				if (!SUCCEEDED(hr))
					break;
				hr = items[i]->GetUInt64(PKEY_Size, &fileinfo[i].Size);
				if (!SUCCEEDED(hr))
					break;
				hr = items[i]->GetFileTime(PKEY_DateCreated, &tempft);
				if (!SUCCEEDED(hr))
					break;
				::FileTimeToSystemTime(&tempft, &fileinfo[i].DateCr);

				hr = items[i]->GetString(PKEY_ParsingPath, &path);
				if (!SUCCEEDED(hr))
					break;

				std::ifstream curfile(path, std::ios::binary|std::ios::ate);
				std::ifstream::pos_type pos = curfile.tellg();
				curfile.seekg(0, std::ios::beg);
				for (int j = 0; j < pos; j++) {
					curfile.read((char*)&curbyte, sizeof(char));
					fileinfo[i].Bytesum += curbyte;
				}
				curfile.close();

			}*/
            if (SUCCEEDED(hr))
            {
				std::sort(fileinfo.begin(), fileinfo.end(), [](const FileInfo& a, const FileInfo& b)->bool {
					return std::wstring(a.Name) < std::wstring(b.Name);
				});
				std::wstring DisplayString (std::to_wstring(count) + L" item(s), items are: \n");
				for (int i = 0; i < count; i++) {

					DisplayString += fileinfo[i].Name + std::wstring(L"  ");
					DisplayString += std::to_wstring(fileinfo[i].Size) + std::wstring(L" bytes  ");
					DisplayString += std::to_wstring(fileinfo[i].DateCr.wYear) + std::wstring(L"-");
					DisplayString += (fileinfo[i].DateCr.wMonth < 10 ? std::wstring(L"0") : std::wstring(L"")) + std::to_wstring(fileinfo[i].DateCr.wMonth) + std::wstring(L"-");
					DisplayString += (fileinfo[i].DateCr.wDay < 10 ? std::wstring(L"0") : std::wstring(L"")) + std::to_wstring(fileinfo[i].DateCr.wDay) + std::wstring(L"    ");
					DisplayString += std::to_wstring(fileinfo[i].Bytesum);

					DisplayString += L"\n";

				}


                MessageBox(_hwnd, DisplayString.c_str(), L"Avid the best", MB_OK);

				//for (int i = 0; i < count; i++){
				//	CoTaskMemFree(names[i]);
				//	}
            }

			for (int i = 0; i < count && items[i] != nullptr; i++) {
				items[i]->Release();
			}
        }
        psia->Release();
    }

    return 0;
}

void CExplorerCommandVerb::Analyze(FileInfo &fileinfo) {
	unsigned char curbyte;
		std::ifstream curfile(fileinfo.Path, std::ios::binary | std::ios::ate);
		if (curfile.is_open())
			hi << fileinfo.Path << "\n";
		hi.flush();
		std::ifstream::pos_type pos = curfile.tellg();
		curfile.seekg(0, std::ios::beg);
		for (int j = 0; j < pos; j++) {
			curfile.read((char*)&curbyte, sizeof(char));
			fileinfo.Bytesum += curbyte;
		}
		curfile.close();
}


IFACEMETHODIMP CExplorerCommandVerb::Invoke(IShellItemArray *psia, IBindCtx *)
{
    IUnknown_GetWindow(_punkSite, &_hwnd);

    HRESULT hr = CoMarshalInterThreadInterfaceInStream(__uuidof(psia), psia, &_pstmShellItemArray);
    if (SUCCEEDED(hr))
    {
        AddRef();
        if (!SHCreateThread(s_ThreadProc, this, CTF_COINIT_STA | CTF_PROCESS_REF, NULL))
        {
            Release();
        }
    }
    return S_OK;
}

static WCHAR const c_szProgID[] = L"*";

HRESULT CExplorerCommandVerb_RegisterUnRegister(bool fRegister)
{
    CRegisterExtension re(__uuidof(CExplorerCommandVerb));

    HRESULT hr;
    if (fRegister)
    {
        hr = re.RegisterInProcServer(c_szVerbDisplayName, L"Apartment");
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterExplorerCommandVerb(c_szProgID, c_szVerbName, c_szVerbDisplayName);
            if (SUCCEEDED(hr))
            {
                hr = re.RegisterVerbAttribute(c_szProgID, c_szVerbName, L"NeverDefault");
            }
        }
    }
    else
    {
        hr = re.UnRegisterVerb(c_szProgID, c_szVerbName);
        hr = re.UnRegisterObject();
    }
    return hr;
}

HRESULT CExplorerCommandVerb_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    CExplorerCommandVerb *pVerb = new (std::nothrow) CExplorerCommandVerb();
    HRESULT hr = pVerb ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        pVerb->QueryInterface(riid, ppv);
        pVerb->Release();
    }
    return hr;
}
