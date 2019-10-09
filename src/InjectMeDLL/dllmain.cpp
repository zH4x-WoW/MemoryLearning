#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

namespace MainDLL
{
	inline HANDLE MyThread;

	DWORD WINAPI Init(void* const(param))
	{
		AllocConsole();
		SetConsoleTitle("Debug Console");
		FILE* stream;
		freopen_s(&stream, "CONOUT$", "w", stdout);
		printf("Hello from internal\n");
		
		auto baseAddress = GetModuleHandleW(NULL);
		printf("Base Address is: 0x%llX\n", baseAddress);
		auto health = *(int*)(baseAddress + 0x223F0);
		printf("Health Read: %i\n", health);		

		//F4 to unload dll 
		while (1 & !GetAsyncKeyState(VK_F4))
		{	
			Sleep(1);
		}

		const auto conHandle = GetConsoleWindow();
		FreeConsole();
		PostMessage(conHandle, WM_CLOSE, 0, 0);
		FreeLibraryAndExitThread(static_cast<HMODULE>(param), NULL);
		
		return 0;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		MainDLL::MyThread = CreateThread(nullptr, 0, MainDLL::Init, hModule, 0, nullptr);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	default:
		break;
	}
	return TRUE;
}

