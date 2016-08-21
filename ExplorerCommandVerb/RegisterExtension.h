#pragma once

#include <windows.h>



class CRegisterExtension
{
public:
    CRegisterExtension(REFCLSID clsid = CLSID_NULL, HKEY hkeyRoot = HKEY_CURRENT_USER);
    ~CRegisterExtension();
    void SetHandlerCLSID(REFCLSID clsid);
    void SetInstallScope(HKEY hkeyRoot);
    HRESULT SetModule(PCWSTR pszModule);
    HRESULT SetModule(HINSTANCE hinst);

    HRESULT RegisterInProcServer(PCWSTR pszFriendlyName, PCWSTR pszThreadingModel) const;
    HRESULT RegisterInProcServerAttribute(PCWSTR pszAttribute, DWORD dwValue) const;

    // register the COM local server for the current running module
    // this is for self registering applications
    HRESULT RegisterAppAsLocalServer(PCWSTR pszFriendlyName, PCWSTR pszCmdLine = NULL) const;
    HRESULT RegisterElevatableLocalServer(PCWSTR pszFriendlyName, UINT idLocalizeString, UINT idIconRef) const;
    HRESULT RegisterElevatableInProcServer(PCWSTR pszFriendlyName, UINT idLocalizeString, UINT idIconRef) const;

    // remove a COM object registration
    HRESULT UnRegisterObject() const;


    HRESULT RegisterAppDropTarget() const;



    HRESULT RegisterCreateProcessVerb(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszCmdLine, PCWSTR pszVerbDisplayName) const;
    HRESULT RegisterDropTargetVerb(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszVerbDisplayName) const;
    HRESULT RegisterExecuteCommandVerb(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszVerbDisplayName) const;
    HRESULT RegisterExplorerCommandVerb(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszVerbDisplayName) const;
    HRESULT RegisterExplorerCommandStateHandler(PCWSTR pszProgID, PCWSTR pszVerb) const;
    HRESULT RegisterVerbAttribute(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszValueName) const;
    HRESULT RegisterVerbAttribute(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszValueName, PCWSTR pszValue) const;
    HRESULT RegisterVerbAttribute(PCWSTR pszProgID, PCWSTR pszVerb, PCWSTR pszValueName, DWORD dwValue) const;
    HRESULT RegisterVerbDefaultAndOrder(PCWSTR pszProgID, PCWSTR pszVerbOrderFirstIsDefault) const;

    HRESULT RegisterPlayerVerbs(PCWSTR const rgpszAssociation[], UINT countAssociation,
                                PCWSTR pszVerb, PCWSTR pszTitle) const;

    HRESULT UnRegisterVerb(PCWSTR pszProgID, PCWSTR pszVerb) const;
    HRESULT UnRegisterVerbs(PCWSTR const rgpszAssociation[], UINT countAssociation, PCWSTR pszVerb) const;

    HRESULT RegisterContextMenuHandler(PCWSTR pszProgID, PCWSTR pszDescription) const;
    HRESULT RegisterRightDragContextMenuHandler(PCWSTR pszProgID, PCWSTR pszDescription) const;

    HRESULT RegisterAppShortcutInSendTo() const;

    HRESULT RegisterThumbnailHandler(PCWSTR pszExtension) const;
    HRESULT RegisterPropertyHandler(PCWSTR pszExtension) const;
    HRESULT UnRegisterPropertyHandler(PCWSTR pszExtension) const;

    HRESULT RegisterLinkHandler(PCWSTR pszProgID) const;

    HRESULT RegisterProgID(PCWSTR pszProgID, PCWSTR pszTypeName, UINT idIcon) const;
    HRESULT UnRegisterProgID(PCWSTR pszProgID, PCWSTR pszFileExtension) const;
    HRESULT RegisterExtensionWithProgID(PCWSTR pszFileExtension, PCWSTR pszProgID) const;
    HRESULT RegisterOpenWith(PCWSTR pszFileExtension, PCWSTR pszProgID) const;
    HRESULT RegisterNewMenuNullFile(PCWSTR pszFileExtension, PCWSTR pszProgID) const;
    HRESULT RegisterNewMenuData(PCWSTR pszFileExtension, PCWSTR pszProgID, PCSTR pszBase64) const;
    HRESULT RegisterKind(PCWSTR pszFileExtension, PCWSTR pszKindValue) const;
    HRESULT UnRegisterKind(PCWSTR pszFileExtension) const;
    HRESULT RegisterPropertyHandlerOverride(PCWSTR pszProperty) const;

    HRESULT RegisterHandlerSupportedProtocols(PCWSTR pszProtocol) const;

    HRESULT RegisterProgIDValue(PCWSTR pszProgID, PCWSTR pszValueName) const;
    HRESULT RegisterProgIDValue(PCWSTR pszProgID, PCWSTR pszValueName, PCWSTR pszValue) const;
    HRESULT RegisterProgIDValue(PCWSTR pszProgID, PCWSTR pszValueName, DWORD dwValue) const;

    HRESULT RegSetKeyValuePrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValueName, PCWSTR pszValue, ...) const;
    HRESULT RegSetKeyValuePrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValueName, DWORD dwValue, ...) const;
    HRESULT RegSetKeyValuePrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValueName, const unsigned char pc[], DWORD dwSize, ...) const;
    HRESULT RegSetKeyValueBinaryPrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValueName, PCSTR pszBase64, ...) const;

    HRESULT RegDeleteKeyPrintf(HKEY hkey, PCWSTR pszKeyFormatString, ...) const;
    HRESULT RegDeleteKeyValuePrintf(HKEY hkey, PCWSTR pszKeyFormatString, PCWSTR pszValue, ...) const;

    PCWSTR GetCLSIDString() const { return _szCLSID; };

    bool HasClassID() const { return _clsid != CLSID_NULL ? true : false; };

private:

    HRESULT _EnsureModule() const;
    bool _IsBaseClassProgID(PCWSTR pszProgID)  const;
    HRESULT _EnsureBaseProgIDVerbIsNone(PCWSTR pszProgID) const;
    void _UpdateAssocChanged(HRESULT hr, PCWSTR pszKeyFormatString) const;

    CLSID _clsid;
    HKEY _hkeyRoot;
    WCHAR _szCLSID[39];
    WCHAR _szModule[MAX_PATH];
    bool _fAssocChanged;
};
