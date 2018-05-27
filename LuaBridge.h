#pragma once
#include "rlua.h"

extern "C" {
	// lua core
#include "Lua/lua.h"
#include "Lua/lauxlib.h"
#include "Lua/lualib.h"
#include "Lua/lobject.h"
}

lua_State* L;



#define r_protect(x) try { x; } catch (const std::exception& e) { luaL_error(L, e.what()); }

DWORD luaState;

int CallHandler(lua_State *L); /* Vanilla Call Handler  */
DWORD rbxFunctionHandler(DWORD rL); /* Roblox Call Function Handler  */

/* RAWRJZ Byass by tepig, credits to him */

DWORD rawrJzAddr = ASLR(0x7385f7);

#define BypassRawrXD() (WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(rawrJzAddr), "\x75", 1, 0))
//Changes the instruction to an unconditional short jmp

#define RestoreBeforeMemcheckCatchesYouRawrXD() (WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(rawrJzAddr), "\x74", 1, 0))

extern DWORD int3breakpoint;

/* Credits to Mainly, Eternal, but aero for editing his SEH Bypass. */

void SEH_BYPASS() {
	int aero = INT_MAX;
	UINT32 *fs = reinterpret_cast<UINT32*>(__readfsdword(0));
	UINT32 *fsb = reinterpret_cast<UINT32*>(fs);
	if (fs[0] < _UI16_MAX || fs[6] < _UI16_MAX) {
		fs[0] = (UINT32)_UI16_MAX;
		fs[6] = (UINT32)_UI16_MAX;
		fsb[0] = fs[0];
		fsb[6] = fs[6];
	}
}

void fakeChain(DWORD* chain)
{
	chain[1] = 0x1555555;
	((DWORD*)chain[0])[1] = 0x1555555;
}

void restoreChain(DWORD* chain, DWORD unk, DWORD nextUnk)
{
	chain[1] = unk;
	((DWORD*)chain[0])[1] = nextUnk;
}

INT r_lua_pcall(int lst, int nargs, int nresults, int errfunc) {
	DWORD* exceptionChain = (DWORD*)__readfsdword(0);
	DWORD unk = exceptionChain[1];
	((((DWORD*)exceptionChain[0])[1]) != NULL);
	{
		DWORD nextUnk = ((DWORD*)exceptionChain[0])[1];
		fakeChain(exceptionChain);
		BypassRawrXD();
		int ret = r_lua_pcall2(lst, nargs, nresults, (int*)errfunc);
		RestoreBeforeMemcheckCatchesYouRawrXD();
		restoreChain(exceptionChain, unk, nextUnk);
		return ret;
	}

	fakeChain(exceptionChain);
	BypassRawrXD();
	int ret = r_lua_pcall2(lst, nargs, nresults, (int*)errfunc);
	RestoreBeforeMemcheckCatchesYouRawrXD();
	restoreChain(exceptionChain, unk, 0);
	return ret;
}

/* Main setfield bypass, credits to aero. And DOGGO for leaking it */


void rlua_setfield(DWORD L, int idx, const char *k) {
	r_lua_pushvalue(L, idx);
	if (r_lua_getmetatable(L, -1)) {
		r_lua_getfield(L, -1, "__newindex");
		r_lua_pushvalue(L, -3);
		r_lua_pushstring(L, k);
		r_lua_pushvalue(L, -6);
		r_lua_pcall(L, 3, 1, 0);
		r_lua_pop(L, 3);
	}
	else {
		r_lua_pop(L, 1);
		r_lua_setfield(L, idx, k);
	};
};

struct userdata {
	int key;
};

void omithus_pushtouserstate(DWORD rL, lua_State* L, int idx)
{
	/* Wrap all the vanilla string, boolean, numbers, threads, etc.  */
	//printf("VANILLA: %d\n", r_lua_type(rL, idx));
	switch (r_lua_type(rL, idx))
	{
	case R_LUA_TNIL:
	{
		lua_pushnil(L);
		break;
	}
	case R_LUA_TSTRING:
	{
		lua_pushstring(L, r_lua_tolstring(rL, idx, 0));
		r_lua_remove(rL, idx);
		break;
	}
	case R_LUA_TNUMBER:
	{
		lua_pushnumber(L, r_lua_tonumber(rL, idx));
		r_lua_remove(rL, idx);
		break;
	}
	case R_LUA_TBOOLEAN:
	{
		lua_pushboolean(L, r_lua_toboolean(rL, idx));
		r_lua_remove(rL, idx);
		break;
	}
	case R_LUA_TUSERDATA:
	{
		int K = r_luaL_ref(rL, LUA_REGISTRYINDEX);
		userdata* userd = (userdata*)lua_newuserdata(L, sizeof(userdata));
		userd->key = K;
		lua_getfield(L, LUA_REGISTRYINDEX, "object_set");
		lua_setmetatable(L, -2);
		break;
	}
	case R_LUA_TFUNCTION:
	{
		lua_pushnumber(L, r_luaL_ref(rL, LUA_REGISTRYINDEX));
		lua_pushcclosure(L, CallHandler, 1);
		break;
	}
	case R_LUA_TTABLE:
		lua_newtable(L);
		r_lua_pushnil(rL);
		while (r_lua_next(rL, -2) != 0)
		{
			r_lua_pushvalue(rL, -2);
			const char* key = r_lua_tolstring(rL, -1, NULL);
			r_lua_pushvalue(rL, -2);
			omithus_pushtouserstate(rL, L, -1);
			lua_setfield(L, -2, key);
			r_lua_pop(rL, 2);
		}
		r_lua_pop(rL, 1);
		break;
	default:
		break;
	}
}

void omithus_pushtorobloxstate(lua_State *L, DWORD rL, int idx)
{
	//printf("ROBLOX: %d\n", lua_type(L, idx));
	switch (lua_type(L, idx))
	{
	case LUA_TNIL:
	{
		r_lua_pushnil(rL);
		break;
	}
	case LUA_TSTRING:
	{
		r_lua_pushstring(rL, lua_tostring(L, idx));
		break;
	}
	case LUA_TNUMBER:
	{
		r_lua_pushnumber(rL, lua_tonumber(L, idx));
		break;
	}
	case LUA_TBOOLEAN:
	{
		r_lua_pushboolean(rL, lua_toboolean(L, idx));
		break;
	}
	case LUA_TUSERDATA:
	{
		userdata* oof = (userdata*)lua_touserdata(L, idx);
		r_lua_rawgeti(rL, LUA_REGISTRYINDEX, oof->key);
		break;
	}
	case LUA_TFUNCTION:
	{
		r_lua_pushnumber(rL, luaL_ref(L, LUA_REGISTRYINDEX));
		r_lua_pushcclosure(rL, breakpointaddys[0], 1);
		break;
	}
	case LUA_TTABLE:
	{
		r_lua_createtable(rL, 0, 0);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			lua_pushvalue(L, -2);
			const char *name = lua_tolstring(L, -1, NULL);
			lua_pushvalue(L, -2);
			omithus_pushtorobloxstate(L, rL, -1);
			rlua_setfield(rL, -2, name);
			lua_pop(L, 2);
		};
		lua_pop(L, 1);
		break;
	}
	default:
		break;
	}
}


DWORD Thread;

int omithus_index(lua_State *L)
{
	printf("[Omithus]: Index.\n");
	const char* Key = lua_tostring(L, -1);
	lua_pop(L, 1);
	omithus_pushtorobloxstate(L, luaState, 1);
	printf("RKey: %s\n", Key);
	r_lua_getfield(luaState, -1, Key);

	omithus_pushtouserstate(luaState, L, -1);
	r_lua_pop(luaState, 1);
	return 1;
}



int omithus_yieldstart(DWORD rL)
{
	printf("on yielding.\n");
	lua_State* L = (lua_State*)(int)r_lua_tonumber(rL, lua_upvalueindex(1));
	int args = r_lua_gettop(rL);
	for (int elemiq = 1; elemiq < args + 1; elemiq++)
		omithus_pushtouserstate(rL, L, elemiq);//danghui_index2adr(rL, elemiq));
	return lua_resume(L, args);
}

int rbxHandleYield(lua_State* L, DWORD rL)
{
	r_lua_pushcclosure(rL, breakpointaddys[1], 0);
	*(BYTE*)(rL + 6) = 0;
	return lua_yield(L, 0);
}

/*for (int elemiq = 1;elemiq <= elems; elemiq++)*/

static int resume(lua_State* thread)
{
	lua_State* L = lua_tothread(thread, lua_upvalueindex(1));
	const int nargs = lua_gettop(thread);
	lua_xmove(thread, L, nargs);
	return lua_resume(L, nargs);
}

void wrap(DWORD rL, lua_State* L, int idx);
void unwrap(DWORD rL, lua_State* L, int idx);


int CallHandler(lua_State *L)
{
	int Key = lua_tonumber(L, lua_upvalueindex(1));
	int ArgC = lua_gettop(L);
	r_lua_rawgeti(luaState, R_LUA_REGISTRYINDEX, Key);
	for (int i = 1; i <= ArgC; i++) {
		omithus_pushtorobloxstate(L, luaState, i * -1);
	}
	int err = r_lua_pcall(luaState, ArgC, LUA_MULTRET, 0);
	if (err == LUA_YIELD)
		return rbxHandleYield(L, luaState);
	ArgC = r_lua_gettop(luaState);
	for (int i = 1; i <= ArgC; i++) {
		omithus_pushtouserstate(luaState, L, i * -1);
	}
	return ArgC;
}

DWORD rbxFunctionHandler(DWORD rL)
{
	printf("On LUA FUNCTION [6].\n");
	int elems = 0;
	int key = r_lua_tonumber(rL, lua_upvalueindex(1));
	lua_rawgeti(L, LUA_REGISTRYINDEX, key);
	elems = r_lua_gettop(rL);
	for (int i = 1; i <= elems; i++)
		wrap(rL, L, i);
	switch (lua_pcall(L, r_lua_gettop(rL), LUA_MULTRET, 0))
	{
	case LUA_YIELD:
		r_lua_pushlightud(rL, (void*)L);
		r_lua_pushcclosure(rL, breakpointaddys[1], 1);
		break;
	case LUA_ERRRUN:
		printf("Yield Error: %s\n", r_lua_tostring(rL, -1));
		break;
	}
	elems = lua_gettop(L);
	for (int i = 1; i <= elems; i++)
	       unwrap(rL, L, i);
	return elems;
}

static int luaFunctionHandler(lua_State* L);

int registry = 0;

void wrap(DWORD rL, lua_State* L, int idx);
void unwrap(DWORD rL, lua_State* L, int idx)
{
	//printf("WRAP: ROBLOX %d\n", lua_type(L, idx));
	switch (lua_type(L, idx))
	{
	case LUA_TNIL:
		r_lua_pushnil(rL);
		break;
	case LUA_TBOOLEAN:
		r_lua_pushboolean(rL, lua_toboolean(L, idx));
		break;
	case LUA_TSTRING:
		r_lua_pushstring(rL, lua_tostring(L, idx));
		break;
	case LUA_TNUMBER:
		r_lua_pushnumber(rL, lua_tonumber(L, idx));
		break;
	case LUA_TTHREAD:
		r_lua_newthread(rL);
		break;
	case LUA_TTABLE:
	{
		lua_pushvalue(L, idx);
		r_lua_createtable(rL, 0, 0);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			unwrap(rL, L, -2);
			unwrap(rL, L, -1);
			r_lua_settable(rL, -3);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
		break;
	}
	case LUA_TLIGHTUSERDATA:
		lua_pushlightuserdata(L, (DWORD*)(r_lua_touserdata(rL, idx)));
		break;
	case LUA_TFUNCTION:
	{
		lua_pushvalue(L, idx);
		r_lua_pushnumber(rL, luaL_ref(L, LUA_REGISTRYINDEX));
		r_lua_pushcclosure(rL, breakpointaddys[0], 1);
		break;
	}
	case LUA_TUSERDATA:
	{

		lua_pushvalue(L, idx);
		lua_gettable(L, LUA_REGISTRYINDEX);
		if (!lua_isnil(L, -1)) {
			r_lua_getfield(rL, LUA_REGISTRYINDEX, lua_tostring(L, -1));
		}
		else {
			r_lua_newuserdata(rL, 0);
		}
		lua_pop(L, 1);
		break;
	}
	default: break;
	}
}
static int luaFunctionHandler(lua_State* L)
{
	//MessageBoxA(NULL, "vanillabridge", "vanillabridge", NULL);
	r_lua_pushstring(luaState, std::to_string(++registry).c_str());
	DWORD rL = r_lua_newthread(luaState);
	r_lua_settable(luaState, LUA_REGISTRYINDEX);

	int key = lua_tonumber(L, lua_upvalueindex(1));

	r_lua_rawgeti(rL, LUA_REGISTRYINDEX, key);

	for (int arg = 1; arg <= lua_gettop(L); ++arg)
		unwrap(rL, L, arg);

	if (r_lua_pcall(rL, lua_gettop(L), LUA_MULTRET, 0))
	{
		const char* errormessage = r_lua_tostring(rL, -1, 0);

		if (!errormessage || strlen(errormessage) == 0)
			errormessage = "Error occoured, no output from Lua\n";

		if (strcmp(errormessage, "attempt to yield across metamethod/C-call boundary") == 0)
		{

			r_lua_pop(rL, 1);
			lua_pushthread(L);
			lua_pushcclosure(L, &resume, 1);
			unwrap(rL, L, -1);

			return lua_yield(L, 0);
		}
		//printf("RVX VANILLA ERROR: %s\n", r_lua_tostring(rL, -1));
		return 0;
		//MessageBoxA(NULL, "SUCCESS VANILLA", "vanillabridge", NULL);
		delete[] errormessage;
	}

	int args = 0;

	for (int arg = 1; arg <= r_lua_gettop(rL); ++arg, ++args)
		wrap(rL, L, arg);

	r_lua_settop(rL, 0);

	return args;
	lua_close(L);
	MessageBoxA(NULL, "SUCCESS VANILLA 2", "vanillabridge", NULL);
}

int regidx_1 = 5;

void wrap(DWORD rL, lua_State* L, int idx)
{
	//printf("WRAP: VANILLA %d\n", r_lua_type(rL, idx));
	switch (r_lua_type(rL, idx))
	{
	case R_LUA_TNIL:
		lua_pushnil(L);
		break;
	case R_LUA_TBOOLEAN:
		lua_pushboolean(L, r_lua_toboolean(rL, idx));
		break;
	case R_LUA_TSTRING:
		lua_pushstring(L, r_lua_tostring(rL, idx));
		break;
	case R_LUA_TNUMBER:
		lua_pushnumber(L, r_lua_tonumber(rL, idx));
		break;
	case R_LUA_TTHREAD:
		lua_newthread(L);
		break;
	case R_LUA_TTABLE:
	{
		r_lua_pushvalue(rL, idx);
		lua_newtable(L);
		r_lua_pushnil(rL);
		while (r_lua_next(rL, -2) != R_LUA_TNIL) {
			wrap(rL, L, -2);
			wrap(rL, L, -1);
			lua_settable(L, -3);
			r_lua_pop(rL, 1);
		}
		r_lua_pop(rL, 1);
		break;
	}
	case R_LUA_TFUNCTION:
	{
		r_lua_pushvalue(rL, idx);
		lua_pushnumber(L, r_luaL_ref(rL, LUA_REGISTRYINDEX));
		lua_pushcclosure(L, luaFunctionHandler, 1);
		break;
	}
	case R_LUA_TUSERDATA:
	{
		r_lua_pushvalue(rL, idx);

		r_lua_pushstring(rL, std::to_string(++registry).c_str());
		r_lua_pushvalue(rL, -2);
		r_lua_settable(rL, LUA_REGISTRYINDEX);
		r_lua_pop(rL, 1);

		lua_newuserdata(L, 0);

		lua_pushvalue(L, -1);
		lua_pushstring(L, std::to_string(registry).c_str());
		lua_settable(L, LUA_REGISTRYINDEX);

		r_lua_getmetatable(rL, idx);
		wrap(rL, L, -1);
		r_lua_pop(rL, 1);

		lua_setmetatable(L, -2);
		break;
	}
	default: break;
	}
}

void PushGlobal(DWORD rL, lua_State* L, const char* name)
{
	r_lua_getfield(rL, -10002, name);
	wrap(rL, L, -1);
	lua_setglobal(L, name);
	r_lua_pop(rL, 1);
}

extern DWORD InitiateSC();
extern DWORD ScriptContext;

void callcheck_exceptionBreak();


void Init()
{
	lua_State* vL = luaL_newstate(); /* Make the vanilla Lua State  */
	luaL_openlibs(vL); /* Open libs for the lua execution, or else it would not work fully. */

	L = lua_newthread(vL);
	luaL_openlibs(L);
	// Scan script context

	int v41 = ScriptContext;
	int v50 = 0;

	//callcheck_exceptionBreak(); /*  Call expection breakpoint, aka CallCheck. (still a method of bypassing breakpoints) */
	
	luaState = (v41 + 56 * v50 + 164) ^ *(DWORD *)(v41 + 56 * v50 + 164);//r_lua_newthread(luaState_old);

	DWORD m_rL = r_lua_newthread(luaState);

	lua_State* m_L = L;

	lua_pushvalue(m_L, LUA_REGISTRYINDEX);

	PushGlobal(m_rL, m_L, "printidentity");
	PushGlobal(m_rL, m_L, "game");
	PushGlobal(m_rL, m_L, "Game");
	PushGlobal(m_rL, m_L, "workspace");
	PushGlobal(m_rL, m_L, "Workspace");
	//printf("yep they do\r\n");
	//MessageBoxA(NULL, "yep they do", "yes indeed", NULL);

	PushGlobal(m_rL, m_L, "Axes");
	PushGlobal(m_rL, m_L, "BrickColor");
	PushGlobal(m_rL, m_L, "CFrame");
	PushGlobal(m_rL, m_L, "Color3");
	PushGlobal(m_rL, m_L, "ColorSequence");
	PushGlobal(m_rL, m_L, "ColorSequenceKeypoint");
	PushGlobal(m_rL, m_L, "NumberRange");
	PushGlobal(m_rL, m_L, "NumberSequence");
	PushGlobal(m_rL, m_L, "NumberSequenceKeypoint");
	PushGlobal(m_rL, m_L, "PhysicalProperties");
	PushGlobal(m_rL, m_L, "Ray");
	PushGlobal(m_rL, m_L, "Rect");
	PushGlobal(m_rL, m_L, "Region3");
	PushGlobal(m_rL, m_L, "Region3int16");
	PushGlobal(m_rL, m_L, "TweenInfo");
	PushGlobal(m_rL, m_L, "UDim");
	PushGlobal(m_rL, m_L, "UDim2");
	PushGlobal(m_rL, m_L, "Vector2");
	PushGlobal(m_rL, m_L, "Vector2int16");
	PushGlobal(m_rL, m_L, "Vector3");
	PushGlobal(m_rL, m_L, "Vector3int16");
	PushGlobal(m_rL, m_L, "Enum");
	PushGlobal(m_rL, m_L, "Faces");
	PushGlobal(m_rL, m_L, "Instance");
	PushGlobal(m_rL, m_L, "math");
	PushGlobal(m_rL, m_L, "warn");
	PushGlobal(m_rL, m_L, "typeof");
	PushGlobal(m_rL, m_L, "type");
	PushGlobal(m_rL, m_L, "spawn");
	PushGlobal(m_rL, m_L, "Spawn");
	PushGlobal(m_rL, m_L, "print");
	PushGlobal(m_rL, m_L, "printidentity");
	PushGlobal(m_rL, m_L, "ypcall");
	PushGlobal(m_rL, m_L, "Wait");
	PushGlobal(m_rL, m_L, "wait");
	PushGlobal(m_rL, m_L, "delay");
	PushGlobal(m_rL, m_L, "Delay");
	PushGlobal(m_rL, m_L, "tick");
	PushGlobal(m_rL, m_L, "LoadLibrary");


}