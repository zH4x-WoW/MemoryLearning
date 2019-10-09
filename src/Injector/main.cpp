#include <windows.h> 
#include <tlhelp32.h> 
#include <shlwapi.h> 
#include <conio.h> 
#include <stdio.h> 

BOOL Inject(DWORD pID, const char * DLL_NAME)
{
	HANDLE process_handle;
	char buf[50] = { 0 };

	if (!pID)
		return false;

	process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (!process_handle)
	{
		sprintf(buf, "OpenProcess() failed: %d", GetLastError());
		printf(buf);
		return false;
	}

	void* loadlibrary_address = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

	// Allocate space in the process for our DLL 
	void* external_memory = (LPVOID)VirtualAllocEx(process_handle, NULL, strlen(DLL_NAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// Write the string name of our DLL in the memory allocated 
	WriteProcessMemory(process_handle, external_memory, DLL_NAME, strlen(DLL_NAME), NULL);

	// Load our DLL 
	HANDLE thread = CreateRemoteThread(process_handle, nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(loadlibrary_address), external_memory, 0, nullptr);
	if (!thread)
	{
		printf("Failed to create remote thread!\n");		
		return 0;
	}

	WaitForSingleObject(thread, INFINITE);
	printf("Dll has been injected it has thread handle: %i\n", thread);

	CreateRemoteThread(process_handle, NULL, NULL, (LPTHREAD_START_ROUTINE)loadlibrary_address, (LPVOID)external_memory, NULL, NULL);

	CloseHandle(process_handle);
	return true;
}

DWORD GetProcessID(const char* processName)
{
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	do
		if (!strcmp(entry.szExeFile, processName)) {
			auto pid = entry.th32ProcessID;
			printf("Wow process found with PID: %i", pid);
			CloseHandle(handle);
			return pid;
		}
	while (Process32Next(handle, &entry));

	return false;
}

int main(int argc, char * argv[])
{
	// Retrieve process ID 
	auto pid = GetProcessID("Wow.exe");

	// Get the dll's full path name 
	char buf[MAX_PATH] = { 0 };
	GetFullPathName("InjectMeDLL.dll", MAX_PATH, buf, NULL);
	printf("DLL Path: %s\n", buf);

	// Inject our main dll 
	Inject(pid, buf);
	
	_getch();
	return 0;
}
