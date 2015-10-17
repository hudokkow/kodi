/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "WMIInterface.h"
#include "../Util.h"

CWIN32Wmi::CWIN32Wmi(void)
{
  bconnected = false;
  pclsObj = nullptr;
  Connect();
}

CWIN32Wmi::~CWIN32Wmi(void)
{
  Release();
}

bool CWIN32Wmi::Connect()
{
  // Initialize COM. ------------------------------------------

  hres =  CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hres))
  {
      return false;                  // Program has failed.
  }

	hres =  CoInitializeSecurity(
        nullptr,
        -1,                          // COM authentication
        nullptr,                        // Authentication services
        nullptr,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        nullptr,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        nullptr                         // Reserved
        );

	if (FAILED(hres))
  {
      return false;                    // Program has failed.
  }

	pLoc = nullptr;

  hres = CoCreateInstance(
      CLSID_WbemLocator,
      0,
      CLSCTX_INPROC_SERVER,
      IID_IWbemLocator, (LPVOID *) &pLoc);

  if (FAILED(hres))
  {
      return false;                 // Program has failed.
  }

	pSvc = nullptr;

  // Connect to the root\cimv2 namespace with
  // the current user and obtain pointer pSvc
  // to make IWbemServices calls.
  hres = pLoc->ConnectServer(
       _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
       nullptr,                    // User name. nullptr = current user
       nullptr,                    // User password. nullptr = current
       0,                       // Locale. nullptr indicates current
       nullptr,                    // Security flags.
       0,                       // Authority (e.g. Kerberos)
       0,                       // Context object
       &pSvc                    // pointer to IWbemServices proxy
       );

  if (FAILED(hres))
  {
      pLoc->Release();
      CoUninitialize();
      return false;                // Program has failed.
  }

	hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       nullptr,                        // Server principal name
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       nullptr,                        // client identity
       EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return false;               // Program has failed.
    }

		pEnumerator = nullptr;

    bconnected = true;
    return true;
}

void CWIN32Wmi::Release()
{
  if(pSvc != nullptr)
    pSvc->Release();
  if(pLoc != nullptr)
    pLoc->Release();
  if(pEnumerator != nullptr)
    pEnumerator->Release();
  if(pclsObj != nullptr)
    pclsObj->Release();

  CoUninitialize();

  bconnected = false;
  pSvc = nullptr;
  pLoc = nullptr;
  pEnumerator = nullptr;
  pclsObj = nullptr;
}

void CWIN32Wmi::testquery()
{
  hres = pSvc->ExecQuery(
      bstr_t("WQL"),
      //bstr_t("SELECT * FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled=TRUE"),
      //bstr_t("SELECT * FROM Win32_NetworkAdapter WHERE PhysicalAdapter=TRUE"),
      bstr_t("SELECT * FROM Win32_NetworkAdapter WHERE Description='Atheros AR5008X Wireless Network Adapter'"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
      nullptr,
      &pEnumerator);

  if (FAILED(hres))
  {
      return;               // Program has failed.
  }
  ULONG uReturn = 0;

  while (pEnumerator)
  {
      HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
          &pclsObj, &uReturn);

      if(0 == uReturn)
      {
          break;
      }

      VARIANT vtProp;
      VariantInit(&vtProp);

      // Get the value of the Name property
      //hr = pclsObj->Get(L"Description", 0, &vtProp, 0, 0);

      vtProp.bstrVal = bstr_t("192.168.1.209");
      hr = pclsObj->Put(L"IPAddress",0,&vtProp,0);
      VariantClear(&vtProp);
			//iCpu++;
  }
	pclsObj->Release();
  pclsObj = nullptr;
}

std::vector<std::string> CWIN32Wmi::GetWMIStrVector(std::string& strQuery, std::wstring& strProperty)
{
  std::vector<std::string> strResult;
  pEnumerator = nullptr;
  pclsObj = nullptr;

  if(!bconnected)
    return strResult;

  hres = pSvc->ExecQuery(
      bstr_t("WQL"),
      bstr_t(strQuery.c_str()),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
      nullptr,
      &pEnumerator);

  if (FAILED(hres))
  {
      return strResult;               // Program has failed.
  }
  ULONG uReturn = 0;

  while (pEnumerator)
  {
      HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
          &pclsObj, &uReturn);

      if(0 == uReturn)
      {
          break;
      }

      VARIANT vtProp;
      VariantInit(&vtProp);

      hr = pclsObj->Get(strProperty.c_str(), 0, &vtProp, 0, 0);
      strResult.push_back(vtProp.bstrVal);
      VariantClear(&vtProp);
  }
  if(pEnumerator != nullptr)
    pEnumerator->Release();
  pEnumerator = nullptr;
  if(pclsObj != nullptr)
	  pclsObj->Release();
  pclsObj = nullptr;
  return strResult;
}

std::string CWIN32Wmi::GetWMIString(std::string& strQuery, std::wstring& strProperty)
{
  return GetWMIStrVector(strQuery, strProperty)[0];
}
