#pragma once
#include "ShellHelpers.h"
#include "RegisterExtension.h"
#include <strsafe.h>
#include <vector>
#include <new>

void DllAddRef();
void DllRelease();


class __declspec(uuid("8C186C9D-B490-4995-855A-C7E3972A4A34")) CExplorerCommandVerb;

HRESULT CExplorerCommandVerb_CreateInstance(REFIID riid, void **ppv);
HRESULT CExplorerCommandVerb_RegisterUnRegister(bool fRegister);
