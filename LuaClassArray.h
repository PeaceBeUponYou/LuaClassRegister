/****************************************************************************************************** //
Author: PeaceBeUponYou


Based on: https://github.com/cheat-engine/cheat-engine/blob/master/Cheat%20Engine/LuaClassArray.pas
LuaClassArray class is the indexer for array types.
It can be changed based on number of parameters to be passed.

// ******************************************************************************************************/
#pragma once
#include "LuaObject.h"

typedef LONG32 MT;
typedef ULONG32 UD;

class LuaClassArray
{
public:
	static int CreateMetaTable(lua_State* L, UD userdata, lua_CFunction getArrayFunc, lua_CFunction setArrayFunc = nullptr, lua_CFunction rawgetter = nullptr);
	static int Index(lua_State* L);
	static int NewIndex(lua_State* L);
};

