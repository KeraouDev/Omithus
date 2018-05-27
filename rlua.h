#pragma once

#include <Windows.h>
#include <sstream>
#include <TlHelp32.h>
#include <iostream>
#include <vector>
#include "Retcheck.h"
#define VERSION "1.0"
#define DbgPrint printf
#define Error MessageBoxA

#define GetGlobalStateIndex(SC, idx) (int*)(SC[14 * idx + 41] - (DWORD)&SC[14 * idx + 41])
#define ASLR(x) (x - 0x400000 + (int)GetModuleHandle(0))

#define R_LUA_REGISTRYINDEX 0xFFFFD8F0

#define NEWTHREAD 0x73FF90 //
#define PUSHSTRING 0x740880//
#define PUSHNUMBER 0x7407F0 //
#define PCALL 0x7402C0 //
#define SETMETATABLE 0x741440//
#define NEWUSERDATA 0x740080 //
#define TOLSTRING 0x7418A0//
#define TOBOOLEAN 0x7417E0//
#define TOUSERDATA 0x740F10//
#define SETFIELD 0x741290//
#define PUSHVALUE 0x740950//
#define PUSHCCLOSURE 0x740400 //
#define TONUMBER 0x741B90//
#define GETMETAFIELD 0x739300//
#define TYPE 0x740F40//
#define SETTOP 0x7408B0//
#define RAWGETI 0x740C10//
#define REF 0x73A490//
#define NEXT 0x740140
#define GETTABLE 0x145CB0//
#define REMOVE 0x740EA0//
#define SETTHREADIDENTITY 0x133D90

#define R_LUA_TNIL 0
#define R_LUA_TLIGHTUSERDATA 1
#define R_LUA_TNUMBER 2
#define R_LUA_TBOOLEAN 3
#define R_LUA_TSTRING 4
#define R_LUA_TTHREAD 5
#define R_LUA_TFUNCTION 6
#define R_LUA_TTABLE 7
#define R_LUA_TUSERDATA 8
#define R_LUA_TPROTO 9
#define R_LUA_TUPVALUE 10

typedef void(__cdecl* rsetthreadidentity)(int, int, int);
rsetthreadidentity r_lua_setthreadidentity = (rsetthreadidentity)ASLR(SETTHREADIDENTITY);
Retcheck ret;

typedef void*(__fastcall* rgetfield)(int, int, const char*);
rgetfield r_lua_getfield = (rgetfield)unprotect(ASLR(0x73F4E0));

typedef int(__cdecl* rnewthread)(int);
rnewthread r_lua_newthread = (rnewthread)unprotect(ASLR(0x73FF90));

typedef void(__cdecl* rpushstring)(int, const char*);
rpushstring r_lua_pushstring = (rpushstring)ASLR(0x740880);

typedef void(__thiscall* rpushnumber)(int, double);
rpushnumber r_lua_pushnumber = (rpushnumber)unprotect(ASLR(0x7407F0));

typedef int(__cdecl* rpcall)(int, int, int, int*);
rpcall r_lua_pcall2 = (rpcall)unprotect(ASLR(0x741060));


typedef int(__cdecl *rcall)(int, DWORD, DWORD);
rcall r_call = (rcall)unprotect(ASLR(0x73eed0));

typedef void(__cdecl* rsetmetatable)(int, int);
rsetmetatable r_lua_setmetatable = (rsetmetatable)unprotect(ASLR(0x741440));

typedef void*(__cdecl* rnewuserdata)(int, int);
rnewuserdata r_lua_newuserdata = (rnewuserdata)unprotect(ASLR(0x740080));

typedef char*(__stdcall* rtolstring)(int, int, int);
rtolstring r_lua_tolstring = (rtolstring)unprotect(ASLR(0x7418A0));

#define r_lua_tostring(L, idx) r_lua_tolstring(L, idx, 0)

typedef bool(__cdecl* rtoboolean)(int, int);
rtoboolean r_lua_toboolean = (rtoboolean)ASLR(0x7417E0);

typedef int(__cdecl* rtouserdata)(int, int);
rtouserdata r_lua_touserdata = (rtouserdata)ASLR(0x741CD0);

typedef void(__cdecl* rsetfield)(int, int, const char*);
rsetfield r_lua_setfield = (rsetfield)unprotect(ASLR(0x741290));

typedef void(__stdcall* rpushvalue)(int, int);
rpushvalue r_lua_pushvalue = (rpushvalue)unprotect(ASLR(0x740950));

typedef void(__cdecl* rpushcclosure)(int, int, int);
rpushcclosure r_lua_pushcclosure = (rpushcclosure)unprotect(ASLR(0x740400));

#define r_pushcfunction(L, f) r_pushcclosure(L, f, 0);

typedef double(__stdcall* rtonumber)(int, int);
rtonumber r_lua_tonumber = (rtonumber)ASLR(0x741B90);

typedef void(__cdecl* rgetmetafield)(int, int, const char*);
rgetmetafield r_lua_getmetafield = (rgetmetafield)ASLR(GETMETAFIELD);

typedef int(__cdecl* rtype)(int, int);
rtype r_lua_type = (rtype)ASLR(0x741D00);

typedef void(__fastcall* rsettop)(int, int);
rsettop r_lua_settop = (rsettop)ret.unprotect((BYTE*)ASLR(0x741660));

typedef void(__cdecl *rsettable)(int, int);
rsettable r_lua_settable = (rsettable)ret.unprotect((byte*)ASLR(0x7415D0));


typedef int(__cdecl* rref)(int, int);
rref r_luaL_ref = (rref)ret.unprotect((BYTE*)ASLR(0x73A490));

typedef void(__cdecl* rrawgeti)(int, int, int);
rrawgeti r_lua_rawgeti = (rrawgeti)ret.unprotect((BYTE*)ASLR(0x740C10));

typedef int(__cdecl* rnext)(int, int);
rnext r_lua_next = (rnext)unprotect(ASLR(0x740140));

typedef int(__cdecl* rgettable)(int, int);
rgettable r_lua_gettable = (rgettable)ASLR(GETTABLE);

typedef void(__cdecl* rremove)(int, int);
rremove r_lua_remove = (rremove)ret.unprotect((BYTE*)ASLR(0x740EA0));

typedef void*(__cdecl *lightud)(int, void*);
lightud r_lua_pushlightud = (lightud)unprotect(ASLR(0x740660));

typedef int*(__cdecl *rgmt)(int, DWORD);
rgmt r_lua_getmetatable = (rgmt)ret.unprotect((byte*)ASLR(0x73F7B0));

typedef void*(__cdecl *rcreatetable)(int, int, int);
rcreatetable r_lua_createtable = (rcreatetable)ret.unprotect((byte*)ASLR(0x73F190));

#define r_lua_pop(L, n) r_lua_settop(L, -(n)-1)

void _WriteMem(DWORD Address, char Byte) {
	DWORD Protection;
	VirtualProtect((LPVOID)Address, 0x5, PAGE_READWRITE, &Protection);
	*(char*)Address = Byte;
	VirtualProtect((LPVOID)Address, 0x5, Protection, &Protection);
}

int r_lua_gettop(int _State) {
	int a1 = (int)_State;
	return (*(DWORD *)(a1 + 8) - *(DWORD *)(a1 + 20)) >> 4;
}

void r_lua_pushboolean(int _State, bool boolean) {
	int State = (int)_State;
	int v2 = *(DWORD*)(State + 8);
	*(DWORD*)v2 = boolean;
	*(DWORD*)(v2 + 8) = 3;
	*(DWORD*)(State + 8) += 16;
}

void r_lua_pushnil(int _State) {
	int State = (int)_State;
	int v2 = *(DWORD*)(State + 8);
	*(DWORD*)(v2 + 8) = 0;
	*(DWORD*)(State + 8) += 16;
}
std::vector<DWORD> breakpointaddys;
INT r_pcall(int* lst, int nargs, int nresults, int errfunc);
