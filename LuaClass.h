/****************************************************************************************************** //
Author: PeaceBeUponYou

Base on: https://github.com/cheat-engine/cheat-engine/blob/master/Cheat%20Engine/LuaClass.pas

LuaClass is the storage point for every LuaObject class. It is responsible for storing, extracting and deleting lua userdata
Basically a metatable for every class.

// ******************************************************************************************************/

#pragma once

#include <string>
#include <Windows.h>
#include <map>
#include <vector>
#include "LuaClassArray.h"

#define RegisterLuaClassEx(k,f,a,d)\
static bool bRegistered##k = LuaClass::AddToLuaClass(new k(), &f, a, d);
#define RegisterLuaClass(klass, metafuncreg) RegisterLuaClassEx(klass, metafuncreg, true, nullptr)
#define RegisterLuaClassWithDestroy(klass, metafuncreg, destroyfunction) RegisterLuaClassEx(klass, metafuncreg, true, destroyfunction)
#define Combiner(x,y) x##y
#define Combine(x,y) Combiner(x,y)
#define RegisterClassL(k,f,d) \
static bool Combine(bRegistered,__LINE__) = LuaClass::AddToLuaClass(new k(), &f, true, d);

#define AddFunctionWithName(func, name) LuaClass::RegisterFunction(L, metatable, userdata, func, name)
#define AddFunction(func) AddFunctionWithName(func, #func)
#define AddProperty(membername, getter, setter) LuaClass::RegisterMember(L, metatable, userdata, #membername, getter, setter)
#define AddArrayProperty(Name, Get, Set, RawGet) LuaClass::RegisterArrayProperty(L, metatable, userdata, #Name, Get, Set, RawGet)
#define AddArrayIntProperty(Get, Set) LuaClass::RegisterDefaultArrayProperty(L, metatable, userdata, Get, Set)
#define AddArrayStrProperty(Get, Set) LuaClass::RegisterDefaultStringArrayProperty(L, metatable, userdata, Get, Set)

#define RemoveObj(objName) LuaClass::UnregisterObject(L, metatable, userdata, #objName)
#define RemoveProperty(prp) RemoveObj(prp)
#define RemoveFunction(fn) RemoveObj(fn)

#define DefineMetadataFunction() \

static void AddMetadataToMetatable(lua_State* L, LONG64 metatable, ULONG64 userdata);

typedef void(*MetaDataRegister)(lua_State* L, LONG64 metatable, ULONG64 userdata);
typedef void(*OnDestroyFunc)(lua_State* L);

typedef struct _ClassEntry
{
	std::type_index index = std::type_index(typeid(void));
	MetaDataRegister Register = nullptr;
	OnDestroyFunc Destructor = nullptr;
	bool bIsInit = false;
}ClassEntry, *PClassEntry;

struct TheClass
{
	size_t hash = 0;
	ClassEntry entry;
};

std::map<size_t, ClassEntry>& GetStore();
std::vector<TheClass>& GetTheStore();
class LuaClass
{

public:
	

	/// <summary>
	/// This function creates a new metatable with associated instance. The Instance must be an object of class <B>LuaObject</B>
	/// </summary>
	static void LuaObjectToMetatable(lua_State* L, LuaObject* instance, bool bGarbageCollectable = false);
	

	/// <summary>
	/// This function instatiates the userdata at the top of the stack by creating a new metatable with all the data defined in ClassEntry parameter.
	/// </summary>
	static void RegisterNewLuaClassFromEntry(lua_State* L, ClassEntry& classEntry, bool bGarbageCollectable);
	

	/// <summary>
	/// The function that creates a table and basic metamethods.
	/// </summary>
	static MT CreateMetatable(lua_State* L, bool bGarbageCollection = false);
	

	/// <summary>
	/// Registers the given cfunction with the given name as a new metamethod for indexing.
	/// Equal to "luaclass_addClassFunctionToTable"
	/// </summary>
	static bool RegisterFunction(lua_State* L, MT metatable, UD userdata, lua_CFunction function, const char* functionName);
	

	/// <summary>
	/// Registers a given field with __getter and __setter functions.
	/// The __getter function must return the field value when called.
	/// The __setter function must set the value of the class field to the given value when called.
	/// Equal to "luaclass_addPropertyToTable"
	/// </summary>
	static bool RegisterMember(lua_State* L, MT metatable, UD userdata, const char* propertyname, lua_CFunction getfunction, lua_CFunction setfunction);
	
	/// <summary>
	/// Removes a property from the metatable.
	/// </summary>
	static bool UnregisterObject(lua_State* L, MT metatable, UD userdata, const char* propertyname);
	
	/// <summary>
	/// Register a property that can get/set arrays
	/// </summary>
	static bool RegisterArrayProperty(lua_State* L, MT metatable, UD userdata, const char* propertyname, lua_CFunction getfunction, lua_CFunction setfunction = nullptr, lua_CFunction rawgetter = nullptr);

	/// <summary>
	/// What happens when the userdata is indexed with a number e.g x[1]
	/// </summary>
	static bool RegisterDefaultArrayProperty(lua_State* L, MT metatable, UD userdata, lua_CFunction getfunction, lua_CFunction setfunction = nullptr);
	
	/// <summary>
	/// What happens when the userdata is indexed with a string e.g x['NameOfItem']
	/// </summary>
	static bool RegisterDefaultStringArrayProperty(lua_State* L, MT metatable, UD userdata, lua_CFunction getfunction, lua_CFunction setfunction = nullptr);
	
	/// <summary>
	/// The basic __index metamethod; Can be changed for suitable purpose. Used to read properties and call functions.
	/// </summary>
	static int Index(lua_State* L);
		  

	/// <summary>
	/// The basic __newindex metamethod; This method is used to set values of properties.
	/// </summary>
	static int NewIndex(lua_State* L);
	

	/// <summary>
	/// The basic __gc metamethod for garbagecollection when an object is discarded.
	/// </summary>
	static int GC(lua_State* L);
	

	/// <summary>
	/// Method used to get an object of associated class, most when a c-closure is set.
	/// </summary>
	static LuaObject* GetClassObject(lua_State* L, size_t matchhash = 0);
	
	/// <summary>
	/// Destroys a LuaObject and clears the userdata associated with it
	/// </summary>
	static bool DestroyLuaObject(lua_State* L, LuaObject* lo);
	
	/// <summary>
	/// Returns the T* form the given userdata at index.
	/// </summary>
	template<typename T>
	static T* ToUserData(lua_State* L, int index)
	{
		T** result = (T**)lua_touserdata(L, index);
		if (result)
			return *result;
		return nullptr;
	}
	template<typename T>
	static T* ToUserDataChecked(lua_State* L, int index)
	{
		if (luaL_checkudata(L, index, typeid(T).name()))
		{
			T** result = (T**)lua_touserdata(L, index);
			if (result)
				return *result;
		}
		return nullptr;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="object"></param>
	/// <param name="registers"></param>
	/// <param name="bForceDelete"></param>
	/// <param name="destructor"></param>
	/// <returns></returns>
	//template<typename T>
	static bool AddToLuaClass(LuaObject* object, MetaDataRegister registers, bool bForceDelete = true, OnDestroyFunc destructor = NULL);
	
	
	static MetaDataRegister FindObjectMetadata(size_t hash);
	

	static ClassEntry FindObjectClassEntry(size_t hash);
	

};
