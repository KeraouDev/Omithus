#include "LuaBridge.h"
#include "Scan.hpp"

void veh_bypass()
{
	HMODULE module = GetModuleHandle("ntdll.dll");
	DWORD KiUserExceptionDispatcher = reinterpret_cast<DWORD>(GetProcAddress(module, "KiUserExceptionDispatcher"));
	DWORD oldProtect;
	VirtualProtect(reinterpret_cast<void*>(KiUserExceptionDispatcher), 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (int n = 0; n < 24; n++)
		*(BYTE*)(KiUserExceptionDispatcher + n) = 0x90;
	VirtualProtect(reinterpret_cast<void*>(KiUserExceptionDispatcher), 1, oldProtect, &oldProtect);
} /* credits to aero & sloppey */

DWORD int3breakpoint = 0;
DWORD ScriptContext = 0;
LONG WINAPI OmithusMainBypass(PEXCEPTION_POINTERS ex)
{
	switch (ex->ExceptionRecord->ExceptionCode)
	{
	case (DWORD)0x80000003L:
	{
		if (ex->ContextRecord->Eip == breakpointaddys[0])
		{
			//MessageBoxA(NULL, "rbxFunctionBridge", "rbxFunctionBridge", NULL);
			ex->ContextRecord->Eip = (DWORD)(rbxFunctionHandler);
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		

		if (ex->ContextRecord->Eip == breakpointaddys[1])
		{
			ex->ContextRecord->Eip = (DWORD)(omithus_yieldstart);
			return EXCEPTION_CONTINUE_EXECUTION;
		} 
		return -1;
	}
	default: return 0;
	}
	return 0;
}

/* credits to sloppey  */

DWORD locateINT3() {
	DWORD _s = ASLR(0x400000);
	const char i3_8opcode[10] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
	for (int i = 0; i < MAX_INT; i++) {
		if (memcmp((void*)(_s + i), i3_8opcode, sizeof(i3_8opcode)) == 0) {
			return (_s + i);
		}
	}
	return NULL;
}

void callcheck_exceptionBreak()
{
	/* Main VEH Bypass for CallCheck  */
	breakpointaddys.push_back(locateINT3());
	breakpointaddys.push_back(locateINT3());
	AddVectoredExceptionHandler(1, OmithusMainBypass);
}

void Omithus_AExecute(std::string buffer)
{
	*(DWORD*)(ASLR(0x1703938)) = 6;
	/* Execute script (string/number/boolean, etc.) function  */
	std::string fscript = std::string("spawn(function()" + std::string(buffer) + "end)");
	if (!luaL_dostring(L, buffer.c_str()))
	{
		printf("Lua Error: %s\n", lua_tostring(L, -1));
	}
	lua_pcall(L, 0, 0, 0);
	return; //FALSE;
}

bool CompareData(const char* Data, const char* Pattern, const char* Mask) {
	while (*Mask) {
		if (*Mask != '?') {
			if (*Data != *Pattern) {
				return false;
			};
		};
		++Mask;
		++Data;
		++Pattern;
	};
	return true;
};

DWORD ScanForScriptContext(const char* SCVFT_Offsetted) {
	MEMORY_BASIC_INFORMATION BasicMemoryInformation = {};
	SYSTEM_INFO SystemInformation = {};
	GetSystemInfo(&SystemInformation);
	DWORD StartingMemorySearchPosition = (DWORD)SystemInformation.lpMinimumApplicationAddress;
	DWORD MaximumSearchBoundary = (DWORD)SystemInformation.lpMaximumApplicationAddress;
	do {
		while (VirtualQuery((void*)StartingMemorySearchPosition, &BasicMemoryInformation, sizeof(BasicMemoryInformation))) {
			if ((BasicMemoryInformation.Protect & PAGE_READWRITE) && !(BasicMemoryInformation.Protect & PAGE_GUARD)) {
				for (DWORD Key = (DWORD)(BasicMemoryInformation.BaseAddress); ((Key - (DWORD)(BasicMemoryInformation.BaseAddress) < BasicMemoryInformation.RegionSize)); ++Key) {
					if (CompareData((const char*)Key, SCVFT_Offsetted, "xxxx")) {
						return Key;
					};
				};
			};
			StartingMemorySearchPosition += BasicMemoryInformation.RegionSize;
		};
	} while (StartingMemorySearchPosition < MaximumSearchBoundary);
	return 0x0;
};

namespace util {
	

	void pause() {
		THREADENTRY32 te32;
		te32.dwSize = sizeof(THREADENTRY32);
		HANDLE hThreads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
		if (Thread32First(hThreads, &te32)) {
			while (Thread32Next(hThreads, &te32)) {
				if (te32.th32OwnerProcessID == GetCurrentProcessId() && te32.th32ThreadID != GetCurrentThreadId()) {
					HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, false, te32.th32ThreadID);
					SuspendThread(hThread);
					CloseHandle(hThread);
				}
			}
		}
		CloseHandle(hThreads);
	}

	void resume() {
		THREADENTRY32 te32;
		te32.dwSize = sizeof(THREADENTRY32);
		HANDLE hThreads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
		if (Thread32First(hThreads, &te32)) {
			while (Thread32Next(hThreads, &te32)) {
				if (te32.th32OwnerProcessID == GetCurrentProcessId() && te32.th32ThreadID != GetCurrentThreadId()) {
					HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, false, te32.th32ThreadID);
					ResumeThread(hThread);
					CloseHandle(hThread);
				}
			}
		}
		CloseHandle(hThreads);
	}
}

DWORD InitiateSC()
{
	/* Initiate "SC" aka ScriptContext on the main scan. */
	DWORD SCVT = ASLR(0x1121698);
	ScriptContext = ScanForScriptContext((char*)&SCVT);//PAGE_READWRITE, (char*)&SCVT, "xxxx");
	return ScriptContext;
}

void ConsoleHacks() /* I don't know who made the bypass method, but I believe it was [FuZion] Dion. Credits to him. (:  */
{
	DWORD nothing;
	VirtualProtect((PVOID)&FreeConsole, 1, PAGE_EXECUTE_READWRITE, &nothing);
	*(BYTE*)(&FreeConsole) = 0xC3;
}

void Console(char* title) { /* Credits to Louka, to the console method  */
	AllocConsole();
	SetConsoleTitleA(title);
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	HWND ConsoleHandle = GetConsoleWindow();
	::SetWindowPos(ConsoleHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	::ShowWindow(ConsoleHandle, SW_NORMAL);
}


DWORD RtlUnwindJmpTo;
DWORD KiUserExceptionDispatcherJmpTo;

__declspec(naked) void VEHK_Hook()
{
	__asm
	{
		mov ebx, [esp]
		push ecx
		push ebx
		call RtlUnwindJmpTo
		jmp KiUserExceptionDispatcherJmpTo
	}
}

DWORD NtDllLocate(PCHAR FnName)
{
	static HMODULE NtDll;
	if (!NtDll)
		NtDll = GetModuleHandleA("ntdll.dll");

	return (DWORD)(GetProcAddress(NtDll, FnName));
}

DWORD KernelBaseLocate(PCHAR FnName)
{
	static HMODULE KernelBase;
	if (!KernelBase)
		KernelBase = GetModuleHandleA("KERNELBASE.dll");

	return (DWORD)(GetProcAddress(KernelBase, FnName));
}

BOOL PlaceHook(DWORD Where)
{
	DWORD nOldProtect;
	if (!VirtualProtect((LPVOID)Where, 5, PAGE_EXECUTE_READWRITE, &nOldProtect))
		return FALSE;
	*(BYTE*)(Where) = 0xE9;
	if (!VirtualProtect((LPVOID)Where, 5, nOldProtect, &nOldProtect))
		return FALSE;
	return TRUE;
}

BOOL CreateHooks()
{
	//VMProtectBeginMutation("CreateHooks (VEH)");
	DWORD RtlUnwind = NtDllLocate("RtlUnwind");
	DWORD KiUserExceptionDispatcher = NtDllLocate("KiUserExceptionDispatcher");
	if (RtlUnwind && KiUserExceptionDispatcher)
	{
		RtlUnwindJmpTo = RtlUnwind + 0x13E;
		KiUserExceptionDispatcherJmpTo = KiUserExceptionDispatcher + 0xF;
		return PlaceHook(KiUserExceptionDispatcher + 5);
	}
	//VMProtectEnd();
	return FALSE;
}

LPVOID WINAPI OmithusPipe(void)
{
	HANDLE hPipe;
	char buffer[9096];
	DWORD dwRead;


	hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\OmithusPipe"),
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
		1,
		9096,
	    9096,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);
	while (hPipe != INVALID_HANDLE_VALUE)
	{
		if (ConnectNamedPipe(hPipe, NULL) != FALSE)   // wait for someone to connect to the pipe
		{
			while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
			{
				/* add terminating zero */
				buffer[dwRead] = '\0';
				//util::pause();
				Omithus_AExecute(buffer);
				//util::resume();
				/* do something with data in buffer */
				//printf("%s", buffer);
			}
		}

		DisconnectNamedPipe(hPipe);
	}
}

void Main()
{
	//ConsoleHacks();
	//Console("Omithus");
	printf("Initiating for Script Context.. ");
	InitiateSC(); /* Call the init ScriptContext first */
	printf("Done, Got Script Context.\n");
	/* And finally, you execute the script! */
	printf("ScriptContext: %x\r\n", ScriptContext);
	lua_State* vL = lua_open();
	callcheck_exceptionBreak();
	Init();
	
	printf("LuaState: %x\r\n", luaState);
	Thread = r_lua_newthread(luaState);
}

BOOL WINAPI DllMain(HMODULE Dll, DWORD jReason, LPVOID)
{
	if (jReason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Main, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)OmithusPipe, NULL, NULL, NULL);
	return TRUE;
}