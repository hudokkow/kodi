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

#include "Win32DllLoader.h"
#include "DllLoader.h"
#include "DllLoaderContainer.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "filesystem/SpecialProtocol.h"
#include "platform/win32/CharsetConverter.h"

#include "dll_tracker_library.h"
#include "dll_tracker_file.h"
#include "exports/emu_msvcrt.h"

#include <limits>

extern "C" FARPROC WINAPI dllWin32GetProcAddress(HMODULE hModule, LPCSTR function);

//dllLoadLibraryA, dllFreeLibrary, dllGetProcAddress are from dllLoader,
//they are wrapper functions of COFF/PE32 loader.
extern "C" HMODULE WINAPI dllLoadLibraryA(LPCSTR libname);
extern "C" HMODULE WINAPI dllLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
extern "C" BOOL WINAPI dllFreeLibrary(HINSTANCE hLibModule);
extern "C" FARPROC WINAPI dllGetProcAddress(HMODULE hModule, LPCSTR function);
extern "C" HMODULE WINAPI dllGetModuleHandleA(LPCSTR lpModuleName);
extern "C" DWORD WINAPI dllGetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);

// our exports
Export win32_exports[] =
{
  { "LoadLibraryA",                                 -1, (void*)dllLoadLibraryA,                              (void*)track_LoadLibraryA },
  { "FreeLibrary",                                  -1, (void*)dllFreeLibrary,                               (void*)track_FreeLibrary },
// msvcrt
  { "_close",                     -1, (void*)dll_close,                     (void*)track_close},
  { "_lseek",                     -1, (void*)dll_lseek,                     nullptr },
  { "_read",                      -1, (void*)dll_read,                      nullptr },
  { "_write",                     -1, (void*)dll_write,                     nullptr },
  { "_lseeki64",                  -1, (void*)dll_lseeki64,                  nullptr },
  { "_open",                      -1, (void*)dll_open,                      (void*)track_open },
  { "fflush",                     -1, (void*)dll_fflush,                    nullptr },
  { "fprintf",                    -1, (void*)dll_fprintf,                   nullptr },
  { "fwrite",                     -1, (void*)dll_fwrite,                    nullptr },
  { "putchar",                    -1, (void*)dll_putchar,                   nullptr },
  { "_fstat",                     -1, (void*)dll_fstat,                     nullptr },
  { "_mkdir",                     -1, (void*)dll_mkdir,                     nullptr },
  { "_stat",                      -1, (void*)dll_stat,                      nullptr },
  { "_fstat32",                   -1, (void*)dll_fstat,                     nullptr },
  { "_stat32",                    -1, (void*)dll_stat,                      nullptr },
  { "_findclose",                 -1, (void*)dll_findclose,                 nullptr },
  { "_findfirst",                 -1, (void*)dll_findfirst,                 nullptr },
  { "_findnext",                  -1, (void*)dll_findnext,                  nullptr },
  { "_findfirst64i32",            -1, (void*)dll_findfirst64i32,            nullptr },
  { "_findnext64i32",             -1, (void*)dll_findnext64i32,             nullptr },
  { "fclose",                     -1, (void*)dll_fclose,                    (void*)track_fclose},
  { "feof",                       -1, (void*)dll_feof,                      nullptr },
  { "fgets",                      -1, (void*)dll_fgets,                     nullptr },
  { "fopen",                      -1, (void*)dll_fopen,                     (void*)track_fopen},
  { "fopen_s",                    -1, (void*)dll_fopen_s,                   nullptr },
  { "putc",                       -1, (void*)dll_putc,                      nullptr },
  { "fputc",                      -1, (void*)dll_fputc,                     nullptr },
  { "fputs",                      -1, (void*)dll_fputs,                     nullptr },
  { "fread",                      -1, (void*)dll_fread,                     nullptr },
  { "fseek",                      -1, (void*)dll_fseek,                     nullptr },
  { "ftell",                      -1, (void*)dll_ftell,                     nullptr },
  { "getc",                       -1, (void*)dll_getc,                      nullptr },
  { "fgetc",                      -1, (void*)dll_getc,                      nullptr },
  { "rewind",                     -1, (void*)dll_rewind,                    nullptr },
  { "vfprintf",                   -1, (void*)dll_vfprintf,                  nullptr },
  { "fgetpos",                    -1, (void*)dll_fgetpos,                   nullptr },
  { "fsetpos",                    -1, (void*)dll_fsetpos,                   nullptr },
  { "_stati64",                   -1, (void*)dll_stati64,                   nullptr },
  { "_stat64",                    -1, (void*)dll_stat64,                    nullptr },
  { "_stat64i32",                 -1, (void*)dll_stat64i32,                 nullptr },
  { "_fstati64",                  -1, (void*)dll_fstati64,                  nullptr },
  { "_fstat64",                   -1, (void*)dll_fstat64,                   nullptr },
  { "_fstat64i32",                -1, (void*)dll_fstat64i32,                nullptr },
  { "_telli64",                   -1, (void*)dll_telli64,                   nullptr },
  { "_tell",                      -1, (void*)dll_tell,                      nullptr },
  { "_fileno",                    -1, (void*)dll_fileno,                    nullptr },
  { "ferror",                     -1, (void*)dll_ferror,                    nullptr },
  { "freopen",                    -1, (void*)dll_freopen,                   (void*)track_freopen},
  { "fscanf",                     -1, (void*)dll_fscanf,                    nullptr },
  { "ungetc",                     -1, (void*)dll_ungetc,                    nullptr },
  { "_fdopen",                    -1, (void*)dll_fdopen,                    nullptr },
  { "clearerr",                   -1, (void*)dll_clearerr,                  nullptr },
  // for debugging
  { "printf",                     -1, (void*)dllprintf,                     nullptr },
  { "vprintf",                    -1, (void*)dllvprintf,                    nullptr },
  { "perror",                     -1, (void*)dllperror,                     nullptr },
  { "puts",                       -1, (void*)dllputs,                       nullptr },
  // workarounds for non-win32 signals
  { "signal",                     -1, (void*)dll_signal,                    nullptr },

  // libdvdnav + python need this (due to us using dll_putenv() to put stuff only?)
  { "getenv",                     -1, (void*)dll_getenv,                    nullptr },
  { "_environ",                   -1, (void*)&dll__environ,                 nullptr },
  { "_open_osfhandle",            -1, (void*)dll_open_osfhandle,            nullptr },

  { nullptr,                          -1, nullptr,                                nullptr }
};

Win32DllLoader::Win32DllLoader(const std::string& dll, bool isSystemDll)
  : LibraryLoader(dll)
  , bIsSystemDll(isSystemDll)
{
  m_dllHandle = nullptr;
  DllLoaderContainer::RegisterDll(this);
}

Win32DllLoader::~Win32DllLoader()
{
  if (m_dllHandle)
    Unload();
  DllLoaderContainer::UnRegisterDll(this);
}

bool Win32DllLoader::Load()
{
  using namespace KODI::PLATFORM::WINDOWS;

  if (m_dllHandle != nullptr)
    return true;

  std::string strFileName = GetFileName();

  auto strDllW = ToW(CSpecialProtocol::TranslatePath(strFileName));
  m_dllHandle = LoadLibraryExW(strDllW.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
  if (!m_dllHandle)
  {
    DWORD dw = GetLastError();
    wchar_t* lpMsgBuf = nullptr;
    DWORD strLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dw, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&lpMsgBuf, 0, nullptr);
    if (strLen == 0)
      strLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&lpMsgBuf, 0, nullptr);

    if (strLen != 0)
    {
      auto strMessage = FromW(lpMsgBuf, strLen);
      CLog::Log(LOGERROR, "%s: Failed to load \"%s\" with error %lu: \"%s\"", __FUNCTION__, CSpecialProtocol::TranslatePath(strFileName).c_str(), dw, strMessage.c_str());
    }
    else
      CLog::Log(LOGERROR, "%s: Failed to load \"%s\" with error %lu", __FUNCTION__, CSpecialProtocol::TranslatePath(strFileName).c_str(), dw);

    LocalFree(lpMsgBuf);
    return false;
  }

  // handle functions that the dll imports
  if (NeedsHooking(strFileName.c_str()))
    OverrideImports(strFileName);

  return true;
}

void Win32DllLoader::Unload()
{
  // restore our imports
  RestoreImports();

  if (m_dllHandle)
  {
    if (!FreeLibrary(m_dllHandle))
       CLog::Log(LOGERROR, "%s Unable to unload %s", __FUNCTION__, GetName());
  }

  m_dllHandle = nullptr;
}

int Win32DllLoader::ResolveExport(const char* symbol, void** f, bool logging)
{
  if (!m_dllHandle && !Load())
  {
    if (logging)
      CLog::Log(LOGWARNING, "%s - Unable to resolve: %s %s, reason: DLL not loaded", __FUNCTION__, GetName(), symbol);
    return 0;
  }

  void *s = GetProcAddress(m_dllHandle, symbol);

  if (!s)
  {
    if (logging)
      CLog::Log(LOGWARNING, "%s - Unable to resolve: %s %s", __FUNCTION__, GetName(), symbol);
    return 0;
  }

  *f = s;
  return 1;
}

bool Win32DllLoader::IsSystemDll()
{
  return bIsSystemDll;
}

HMODULE Win32DllLoader::GetHModule()
{
  return m_dllHandle;
}

bool Win32DllLoader::HasSymbols()
{
  return false;
}

void Win32DllLoader::OverrideImports(const std::string &dll)
{
  using KODI::PLATFORM::WINDOWS::ToW;
  auto strdllW = ToW(CSpecialProtocol::TranslatePath(dll));
  auto image_base = reinterpret_cast<BYTE*>(GetModuleHandleW(strdllW.c_str()));

  if (!image_base)
  {
    CLog::Log(LOGERROR, "%s - unable to GetModuleHandle for dll %s", __FUNCTION__, dll.c_str());
    return;
  }

  auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(image_base);
  auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(image_base + dos_header->e_lfanew); // e_lfanew = value at 0x3c

  auto imp_desc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
    image_base + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

  if (!imp_desc)
  {
    CLog::Log(LOGERROR, "%s - unable to get import directory for dll %s", __FUNCTION__, dll.c_str());
    return;
  }

  // loop over all imported dlls
  for (int i = 0; imp_desc[i].Characteristics != 0; i++)
  {
    auto dllName = reinterpret_cast<char*>(image_base + imp_desc[i].Name);

    // check whether this is one of our dll's.
    if (NeedsHooking(dllName))
    {
      // this will do a loadlibrary on it, which should effectively make sure that it's hooked
      // Note that the library has obviously already been loaded by the OS (as it's implicitly linked)
      // so all this will do is insert our hook and make sure our DllLoaderContainer knows about it
      auto hModule = dllLoadLibraryA(dllName);
      if (hModule)
        m_referencedDlls.push_back(hModule);
    }

    PIMAGE_THUNK_DATA orig_first_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(image_base + imp_desc[i].OriginalFirstThunk);
    PIMAGE_THUNK_DATA first_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(image_base + imp_desc[i].FirstThunk);

    // and then loop over all imported functions
    for (int j = 0; orig_first_thunk[j].u1.Function != 0; j++)
    {
      void *fixup = nullptr;
      if (orig_first_thunk[j].u1.Function & 0x80000000)
        ResolveOrdinal(dllName, (orig_first_thunk[j].u1.Ordinal & 0x7fffffff), &fixup);
      else
      { // resolve by name
        PIMAGE_IMPORT_BY_NAME orig_imports_by_name = (PIMAGE_IMPORT_BY_NAME)(
          image_base + orig_first_thunk[j].u1.AddressOfData);

        ResolveImport(dllName, (char*)orig_imports_by_name->Name, &fixup);
      }/*
      if (!fixup)
      { // create a dummy function for tracking purposes
        PIMAGE_IMPORT_BY_NAME orig_imports_by_name = (PIMAGE_IMPORT_BY_NAME)(
          image_base + orig_first_thunk[j].u1.AddressOfData);
        fixup = CreateDummyFunction(dllName, (char*)orig_imports_by_name->Name);
      }*/
      if (fixup)
      {
        // save the old function
        Import import;
        import.table = &first_thunk[j].u1.Function;
        import.function = first_thunk[j].u1.Function;
        m_overriddenImports.push_back(import);

        DWORD old_prot = 0;

        // change to protection settings so we can write to memory area
        VirtualProtect((PVOID)&first_thunk[j].u1.Function, 4, PAGE_EXECUTE_READWRITE, &old_prot);

        // patch the address of function to point to our overridden version
        first_thunk[j].u1.Function = (uintptr_t)fixup;

        // reset to old settings
        VirtualProtect((PVOID)&first_thunk[j].u1.Function, 4, old_prot, &old_prot);
      }
    }
  }
}

bool Win32DllLoader::NeedsHooking(const char *dllName)
{
  if ( !StringUtils::EndsWithNoCase(dllName, "libdvdcss-2.dll")
  && !StringUtils::EndsWithNoCase(dllName, "libdvdnav.dll"))
    return false;

  LibraryLoader *loader = DllLoaderContainer::GetModule(dllName);
  if (loader)
  {
    // may have hooked this already (we can have repeats in the import table)
    for (unsigned int i = 0; i < m_referencedDlls.size(); i++)
    {
      if (loader->GetHModule() == m_referencedDlls[i])
        return false;
    }
  }
  return true;
}

void Win32DllLoader::RestoreImports()
{
  // first unhook any referenced dll's
  for (auto& module : m_referencedDlls)
    dllFreeLibrary(module);
  m_referencedDlls.clear();

  for (auto& import : m_overriddenImports)
  {
    // change to protection settings so we can write to memory area
    DWORD old_prot = 0;
    VirtualProtect(import.table, 4, PAGE_EXECUTE_READWRITE, &old_prot);

    *static_cast<uintptr_t *>(import.table) = import.function;

    // reset to old settings
    VirtualProtect(import.table, 4, old_prot, &old_prot);
  }
}

bool FunctionNeedsWrapping(Export *exports, const char *functionName, void **fixup)
{
  Export *exp = exports;
  while (exp->name)
  {
    if (strcmp(exp->name, functionName) == 0)
    { //! @todo Should we be tracking stuff?
      if (0)
        *fixup = exp->track_function;
      else
        *fixup = exp->function;
      return true;
    }
    exp++;
  }
  return false;
}

bool Win32DllLoader::ResolveImport(const char *dllName, const char *functionName, void **fixup)
{
  return FunctionNeedsWrapping(win32_exports, functionName, fixup);
}

bool Win32DllLoader::ResolveOrdinal(const char *dllName, unsigned long ordinal, void **fixup)
{
  Export *exp = win32_exports;
  while (exp->name)
  {
    if (exp->ordinal == ordinal)
    { //! @todo Should we be tracking stuff?
      if (0)
        *fixup = exp->track_function;
      else
        *fixup = exp->function;
      return true;
    }
    exp++;
  }
  return false;
}

extern "C" FARPROC __stdcall dllWin32GetProcAddress(HMODULE hModule, LPCSTR function)
{
  // if the high-order word is zero, then lpProcName is the function's ordinal value
  if (reinterpret_cast<uintptr_t>(function) > std::numeric_limits<WORD>::max())
  {
    // first check whether this function is one of the ones we need to wrap
    void *fixup = nullptr;
    if (FunctionNeedsWrapping(win32_exports, function, &fixup))
      return (FARPROC)fixup;
  }

  // Nope
  return GetProcAddress(hModule, function);
}

