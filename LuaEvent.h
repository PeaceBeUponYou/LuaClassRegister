/****************************************************************************************************** //
Author: PeaceBeUponYou



LuaEvent class is the controller of all lua-callable events.
It can be changed based on number of parameters to be passed.

// ******************************************************************************************************/

#pragma once
#include "LuaClass.h"
#define lua_isheavyUserdata(L, i) lua_type(L, i) == LUA_TUSERDATA
namespace PBUY
{
	class LuaEvent : LuaObject
	{
	protected:
		int funcIndex = -1;

	public:
		LuaEvent()
		{

		}
		LuaEvent(lua_State*);
		~LuaEvent();
		bool IsSet();
		void SetEvent(lua_State*);
		void PushFunction(lua_State* L = nullptr);
		void NotifyEventNoParm(lua_State*, int returnvalues = 0);
		void NotifyEventSingleParm(lua_State*, LuaObject*, int returnvalues = 0);
		int OnNotifyEventSingleParm(lua_State*, int returnvalues = 0);

	};
}

