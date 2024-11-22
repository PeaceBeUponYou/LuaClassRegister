
#include "LuaEvent.h"

namespace PBUY
{
	LuaEvent::LuaEvent(lua_State* L)
	{
		SetEvent(L);
	}
	LuaEvent::~LuaEvent()
	{
		lua_State* L = GetLuaState();
		if (L && IsSet())
			luaL_unref(L, LUA_REGISTRYINDEX, funcIndex);
	}
	bool LuaEvent::IsSet()
	{
		return funcIndex != -1;
	}
	void LuaEvent::SetEvent(lua_State* L)
	{
		if (IsSet())
			luaL_unref(L, LUA_REGISTRYINDEX, funcIndex);

		if (lua_isfunction(L, 1))
		{
			lua_pushvalue(L, 1);
			funcIndex = luaL_ref(L, LUA_REGISTRYINDEX);
		}

	}
	void LuaEvent::PushFunction(lua_State* L)
	{
		if (!L)
			L = GetLuaState();
		if (funcIndex == -1)
			//TODO: for the reference functions
			lua_pushnil(L);
		else
			lua_rawgeti(L, LUA_REGISTRYINDEX, funcIndex);
	}
	void LuaEvent::NotifyEventNoParm(lua_State* L, int returnvalues)
	{
		try
		{
			if (IsSet())
			{
				PushFunction(L);
				if (lua_pcall(L, 1, returnvalues, 0) != 0)
				{
					Lua_Print(L, lua_tostring(L, 1));
				}
			}
		}
		catch (const std::exception& e)
		{
			Lua_Print(L, e.what());
		}
	}
	void LuaEvent::NotifyEventSingleParm(lua_State* L, LuaObject* sender, int returnvalues)
	{
		try
		{
			if (IsSet())
			{
				PushFunction(L);
				LuaClass::LuaObjectToMetatable(L, sender);
				if (lua_pcall(L, 1, returnvalues, 0) != 0)
				{
					Lua_Print(L, lua_tostring(L, 1));
				}
			}
		}
		catch (const std::exception& e)
		{
			Lua_Print(L, e.what());
		}
	}
	int LuaEvent::OnNotifyEventSingleParm(lua_State* L, int returnvalues)
	{
		//int params;
		if (IsSet())
		{
			LuaObject* object = nullptr;
			if (lua_gettop(L) >= 1 && lua_isheavyUserdata(L, 1))
			{
				object = LuaClass::ToUserData<LuaObject>(L, 1);
				lua_pop(L, lua_gettop(L));
				NotifyEventSingleParm(L, object, returnvalues);
			}
		}
		return 0;
	}
}