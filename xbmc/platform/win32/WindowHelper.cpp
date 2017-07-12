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

#include "WindowHelper.h"

extern HWND g_hWnd;

CWHelper g_windowHelper;

CWHelper::CWHelper(void) : CThread("WindowHelper")
{
  m_hwnd = nullptr;
  m_hProcess = nullptr;
}

CWHelper::~CWHelper(void)
{
  StopThread();
  m_hwnd = nullptr;
  if(m_hProcess != nullptr)
  {
    CloseHandle(m_hProcess);
    m_hProcess = nullptr;
  }
}

void CWHelper::OnStartup()
{
  if((m_hwnd == nullptr) && (m_hProcess == nullptr))
    return;

  // Minimize XBMC if not already
  ShowWindow(g_hWnd,SW_MINIMIZE);
  if(m_hwnd != nullptr)
    ShowWindow(m_hwnd,SW_RESTORE);

  OutputDebugString(L"WindowHelper thread started\n");
}

void CWHelper::OnExit()
{
  // Bring back XBMC window
  ShowWindow(g_hWnd,SW_RESTORE);
  SetForegroundWindow(g_hWnd);
  m_hwnd = nullptr;
  if(m_hProcess != nullptr)
  {
    CloseHandle(m_hProcess);
    m_hProcess = nullptr;
  }
  LockSetForegroundWindow(LSFW_LOCK);
  OutputDebugString(L"WindowHelper thread ended\n");
}

void CWHelper::Process()
{
  while (( !m_bStop ))
  {
    if(WaitForSingleObject(m_hProcess,500) != WAIT_TIMEOUT)
      break;
    /*if((m_hwnd != nullptr) && (IsIconic(m_hwnd) == TRUE))
      break;*/
  }
}

void CWHelper::SetHWND(HWND hwnd)
{
  m_hwnd = hwnd;
}

void CWHelper::SetHANDLE(HANDLE hProcess)
{
  m_hProcess = hProcess;
}

