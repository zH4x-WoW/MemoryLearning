#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <cstdint>
#include <windef.h>
#include <list>
#include <easyhook.h>
#include <memory>
#include <string>

namespace MainDLL
{
	inline HANDLE MyThread;
	inline uintptr_t OnUpdateTrampoline;

	struct ByteBuffer final
	{
		BYTE data[32];
	};

	inline std::list<std::shared_ptr<ByteBuffer>> ByteBuffers;

	/// <summary>
	/// Creates a simple hook.
	/// </summary>
	/// <param name="target">The address of the function to hook.</param>
	/// <param name="handler">The address of the callback function.</param>
	/// <param name="trampoline">The address of the allocated memory containing the overwritten bytes of the original hooked function.</param>
	/// <param name="size">The number of bytes of the original function to hook(must be > 5).</param>
	/// <returns>True if the hook was successful.</returns>
	bool Create(const uintptr_t target, const uintptr_t* handler, uintptr_t* trampoline, const size_t size)
	{
		auto result = false;
		DWORD dwProtect;
		DWORD tVar;

		//protect original read/write
		if (VirtualProtect(reinterpret_cast<LPVOID>(target), size, PAGE_EXECUTE_READWRITE, &dwProtect))
		{
			auto hook = std::make_shared<ByteBuffer>();
			ByteBuffers.emplace_back(hook);

			for (auto i = 0; i < static_cast<int32_t>(size); ++i)
			{
				hook->data[i] = *reinterpret_cast<BYTE*>(target + i);                                   // copy original bytes to trampoline																							        
				printf("Copied: %02X\n", hook->data[i]);
			}

			hook->data[size] = static_cast<BYTE>(0x68);								                    // push ...
			*reinterpret_cast<int32_t*>(&hook->data[size + 1]) = target + size;			                // the address of the next valid instruction in the target
			hook->data[size + 5] = static_cast<BYTE>(0xC3);							                    // return
			*trampoline = reinterpret_cast<uintptr_t>(hook->data);
			VirtualProtect(reinterpret_cast<LPVOID>(hook->data), size + 6, PAGE_EXECUTE_READWRITE, &tVar);

			*reinterpret_cast<BYTE*>(target) = 0x68;							                        // push ...
			*reinterpret_cast<int32_t*>(target + 1) = reinterpret_cast<int32_t>(handler);	            // the address of the detour
			*reinterpret_cast<BYTE*>(target + 5) = 0xC3;							                    // return

			for (auto i = 6; i < static_cast<int32_t>(size); ++i)                                       // if size > 6
				*reinterpret_cast<BYTE*>(target + i) = 0x90;						                    // fill the gap with NOPs

			result = VirtualProtect(reinterpret_cast<LPVOID>(target), size, dwProtect, &tVar);          // restore original memory protection
		}
		return result;
	}

	int32_t __cdecl OnUpdate(float *a1)
	{
		printf("We in our OnUpdate\n");
		return reinterpret_cast<int32_t(__cdecl*)(float*)>(OnUpdateTrampoline)(a1);
	}

	DWORD WINAPI Init(void* const(param))
	{
		AllocConsole();
		SetConsoleTitle("Debug Console");
		FILE* stream;
		freopen_s(&stream, "CONOUT$", "w", stdout);
		printf("Hello from internal\n");
		
		auto baseAddress = GetModuleHandleW(NULL);
		printf("Base Address is: 0x%llX\n", (long long)baseAddress);

		//F4 to unload dll 
		while (1 & !GetAsyncKeyState(VK_F4))
		{	
			if (GetAsyncKeyState(VK_F5))
			{
				auto health = *(int*)((uintptr_t)baseAddress + 0x1F340);
				printf("Health Read: %i\n", health);
			}

			if (GetAsyncKeyState(VK_F6))
			{
				*(int*)((uintptr_t)baseAddress + 0x1F340) += 10;
				auto health = *(int*)((uintptr_t)baseAddress + 0x1F340);
				printf("Health Read: %i\n", health);
			}

			if (GetAsyncKeyState(VK_F7))
			{
				auto result = Create((uintptr_t)baseAddress + 0x126E0, reinterpret_cast<uintptr_t*>(&OnUpdate), &OnUpdateTrampoline, 6);
				printf("Result of Function Detour: %d\n", result);
			}

			if (GetAsyncKeyState(VK_F9))
			{
				printf("EasyHook Install Starting...");
				// Easy hook test.
				HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook

				auto address = baseAddress + 0x126E0;
				
				NTSTATUS result = LhInstallHook(
					address,
					&OnUpdate,
					NULL,
					&hHook);
				if (FAILED(result))
				{
					std::wstring s(RtlGetLastErrorString());
					wprintf(L"Failed to install hook: %ls\n", s.c_str());										
				}
				else
				{
					printf("EasyHook Install Successful...\n", );
				}

				// If the threadId in the ACL is set to 0, 
				// then internally EasyHook uses GetCurrentThreadId()
				ULONG ACLEntries[1] = { 0 };
				LhSetInclusiveACL(ACLEntries, 1, &hHook);
				printf("LhSetInclusiveACL done...\n");
			}

			Sleep(100);
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

