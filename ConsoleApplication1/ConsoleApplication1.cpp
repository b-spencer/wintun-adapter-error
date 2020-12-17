#include <Windows.h>
#include <stdio.h>
#include "wintun.h"

static void CALLBACK
ConsoleLogger(_In_ WINTUN_LOGGER_LEVEL Level, _In_z_ const WCHAR* LogLine)
{
  fwprintf(stderr, L"wintun: %s\n", LogLine);
}

int
main(void)
{
  DWORD lastError = ERROR_SUCCESS;
  HMODULE wintun = nullptr;
  WINTUN_CREATE_ADAPTER_FUNC WintunCreateAdapter = nullptr;
  WINTUN_DELETE_ADAPTER_FUNC WintunDeleteAdapter = nullptr;
  WINTUN_FREE_ADAPTER_FUNC WintunFreeAdapter = nullptr;
  WINTUN_OPEN_ADAPTER_FUNC WintunOpenAdapter = nullptr;
  WINTUN_SET_LOGGER_FUNC WintunSetLogger = nullptr;
  WINTUN_ADAPTER_HANDLE adapter = nullptr;

  auto cleanup = [&]()
  {
    if(adapter)
    {
      WintunDeleteAdapter(adapter, false, nullptr);
      WintunFreeAdapter(adapter);
      adapter = nullptr;
    }

    if(wintun)
    {
      FreeLibrary(wintun);
      wintun = nullptr;
    }
  };

  for(int i = 0; i < 2; ++i)
  {
    printf("Run \n\n");

    wintun = LoadLibraryExW(L"wintun.dll", NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
    if(not wintun)
    {
      printf("Failed to load wintun.dll with error = 0x%x\n", lastError);
      lastError = GetLastError();
      return lastError;;
    }
    
    WintunCreateAdapter = (WINTUN_CREATE_ADAPTER_FUNC)GetProcAddress(wintun, "WintunCreateAdapter");
    WintunDeleteAdapter = (WINTUN_DELETE_ADAPTER_FUNC)GetProcAddress(wintun, "WintunDeleteAdapter");
    WintunFreeAdapter = (WINTUN_FREE_ADAPTER_FUNC)GetProcAddress(wintun, "WintunFreeAdapter");
    WintunOpenAdapter = (WINTUN_OPEN_ADAPTER_FUNC)GetProcAddress(wintun, "WintunOpenAdapter");
    WintunSetLogger= (WINTUN_SET_LOGGER_FUNC)GetProcAddress(wintun, "WintunSetLogger");

    if(not WintunCreateAdapter || not WintunDeleteAdapter
      || not WintunFreeAdapter || not WintunOpenAdapter
      || not WintunSetLogger)
    {
      printf("Failed to load functions\n");
      cleanup();
      return lastError;
    }

    WintunSetLogger(ConsoleLogger);

    printf("Opening adapter\n");
    // Using a suggested GUID here doesn't help.
    adapter = WintunOpenAdapter(L"APool", L"AName");
    if(not adapter)
    {
      adapter = WintunCreateAdapter(L"APool", L"AName", nullptr, nullptr);
      if(not adapter)
      {
        lastError = GetLastError();
        printf("Failed to create adapter with error = 0x%x\n", lastError);
        cleanup();
        return lastError;
      }
    }

    printf("Shutting down\n"); 
    cleanup();

    // Waiting here with Sleep() doesn't to help.
  }
  return lastError;
}
