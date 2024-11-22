/****************************************************************************************************** //
Author: PeaceBeUponYou



LuaObject class is the base/parent class of every class that is to be registered as a Lua Userdata 
object. 

All sub-classes must have these static functions declared in them:

1: AddMetadataToMetatable
2: OnDestroy

// ******************************************************************************************************/


#pragma once
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <string>
extern "C"
{
#include "cepluginsdk.h"
}
extern lua_State* GetLuaState();
void Lua_Print(lua_State* L, const char* message);
void Lua_Print(lua_State* L, std::string& message);
class LuaObject
{
public:
	std::type_index klasstype;
	//RegisterClass(T);
	LuaObject() : klasstype(typeid(*this))
	{

	}
	virtual ~LuaObject()
	{

	}
	virtual const char* GetName()
	{
		return klasstype.name();
	}

	virtual void ToMetatable(lua_State* L)
	{

	}

	/// <summary>
	/// MUST: This function is called whenever a Userdata of the class is created to register metamethods, metafields and metaproperties.
	/// It must be declared for every class
	/// </summary>
	/// <param name="L">lua stack</param>
	/// <param name="metatable">The index of metatable in the lua stack</param> 
	/// <param name="userdata">The index of userdata pointer in the lua stack</param>
	static void AddMetadataToMetatable(lua_State* L, LONG64 metatable, ULONG64 userdata)
	{

	}

	/// <summary>
	/// Must include an override of this function if you want manual cleanup during garbage collection
	/// </summary>
	/// <param name="L">lua stack</param>
	static void OnDestroy(lua_State* L)
	{

	}
};
