#include "LuaClass.h"
#define DEFAULTINTINDEXGET "__defaultintegergetindexhandler"
#define DEFAULTINTINDEXSET "__defaultintegersetindexhandler"
#define DEFAULTSTRINGINDEXGET "__defaultstringgetindexhandler"
#define DEFAULTSTRINGINDEXSET "__defaultstringsetindexhandler"

extern void Lua_Print(lua_State* L, const char* message);
std::map<size_t, ClassEntry> storeroom = std::map<size_t, ClassEntry>();
std::vector<TheClass> TheClasses = std::vector<TheClass>();
std::map<size_t, ClassEntry>& GetStore()
{
	return storeroom;
}

std::vector<TheClass>& GetTheStore()
{
	return TheClasses;
}

void LuaClass::LuaObjectToMetatable(lua_State* L, LuaObject* instance, bool bGarbageCollectable)
{
	if (instance)
	{
		ClassEntry ce = FindObjectClassEntry(instance->klasstype.hash_code());
		if (ce.bIsInit)
		{
			int top = lua_gettop(L);
			void** p = (void**)lua_newuserdata(L, sizeof(void*));
			*p = instance;
			top = lua_gettop(L);
			LuaClass::RegisterNewLuaClassFromEntry(L, ce, bGarbageCollectable);
		}
		else
			luaL_error(L, "Error: No registered metafunction of the given instance found");
	}
	else
		lua_pushnil(L);
}

void LuaClass::RegisterNewLuaClassFromEntry(lua_State* L, ClassEntry& classEntry, bool bGarbageCollectable)
{
	MT metatable = 0;
	UD userdata = 0;
	userdata = lua_gettop(L);
	metatable = CreateMetatable(L, bGarbageCollectable);
	if (classEntry.Destructor)
	{
		lua_pushstring(L, "OnDestroy");
		lua_pushvalue(L, userdata);
		lua_pushcclosure(L, (lua_CFunction)classEntry.Destructor, 1);
		lua_settable(L, metatable);
	}
	classEntry.Register(L, metatable, userdata);
	lua_setmetatable(L, userdata);
}

MT LuaClass::CreateMetatable(lua_State* L, bool bGarbageCollection)
{
	lua_newtable(L);
	int top = lua_gettop(L);

	lua_pushstring(L, "__autodestroy");
	lua_pushboolean(L, bGarbageCollection);
	lua_settable(L, top);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, LuaClass::Index);
	lua_settable(L, top);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, LuaClass::NewIndex);
	lua_settable(L, top);
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, LuaClass::GC);
	lua_settable(L, top);

	return MT(top);
}

bool LuaClass::RegisterFunction(lua_State* L, MT metatable, UD userdata, lua_CFunction function, const char* functionName)
{
	std::string str(functionName);
	str[0] = toupper(str[0]);
	lua_pushstring(L, str.c_str());
	lua_pushvalue(L, userdata);
	lua_pushcclosure(L, function, 1);
	lua_settable(L, metatable);
	if (strlen(functionName) > 0)
	{
		str[0] = tolower(str[0]);
		lua_pushstring(L, str.c_str());
		lua_pushvalue(L, userdata);
		lua_pushcclosure(L, function, 1);
		lua_settable(L, metatable);
	}
	return true;
}

bool LuaClass::RegisterMember(lua_State* L, MT metatable, UD userdata, const char* propertyname, lua_CFunction getfunction, lua_CFunction setfunction)
{
	std::string pnm(propertyname);
	pnm[0] = toupper(pnm[0]);
	lua_pushstring(L, pnm.c_str());
	lua_newtable(L);
	int top = lua_gettop(L);
	if (getfunction)
	{
		lua_pushstring(L, "__getter");
		lua_pushvalue(L, userdata);
		lua_pushcclosure(L, getfunction, 1);
		lua_settable(L, top);
	}

	if (setfunction)
	{
		lua_pushstring(L, "__setter");
		lua_pushvalue(L, userdata);
		lua_pushcclosure(L, setfunction, 1);
		lua_settable(L, top);
	}
	lua_settable(L, metatable);
	top = lua_gettop(L);
	lua_pushstring(L, pnm.c_str());
	if (lua_gettable(L, metatable) == LUA_TTABLE)
	{
		pnm[0] = tolower(pnm[0]);
		lua_setfield(L, metatable, pnm.c_str());
		/*lua_pushstring(L, pnm.c_str());
		lua_rotate(L, 1, 1);
		lua_settable(L, metatable);*/
	}
	lua_settop(L, top);
	return true;
}

bool LuaClass::UnregisterObject(lua_State* L, MT metatable, UD userdata, const char* propertyname)
{
	int top = lua_gettop(L);
	if (lua_istable(L, metatable) && lua_getfield(L, metatable, propertyname))
	{
		lua_pushnil(L);
		lua_setfield(L, metatable, propertyname);
		//return true;
	}
	lua_settop(L, top);
	return false;
}

bool LuaClass::RegisterArrayProperty(lua_State* L, MT metatable, UD userdata, const char* propertyname, lua_CFunction getfunction, lua_CFunction setfunction, lua_CFunction rawgetter)
{
	int xtop = lua_gettop(L);
	std::string pnm(propertyname);
	pnm[0] = toupper(pnm[0]);
	//Add with uppercase first letter
	lua_pushstring(L, pnm.c_str());
	lua_newtable(L);
	int top = lua_gettop(L);

	LuaClassArray::CreateMetaTable(L, userdata, getfunction, setfunction, rawgetter);
	lua_setmetatable(L, top);
	lua_settable(L, metatable);
	//Now add ref with lowercase
	lua_pushstring(L, pnm.c_str());
	if (lua_gettable(L, metatable) == LUA_TTABLE)
	{
		lua_pushvalue(L, -1);
		pnm[0] = tolower(pnm[0]);
		lua_setfield(L, metatable, pnm.c_str());
		lua_pop(L, 1);
	}

	xtop = lua_gettop(L);
	lua_settop(L, xtop);
	return true;
}

bool LuaClass::RegisterDefaultArrayProperty(lua_State* L, MT metatable, UD userdata, lua_CFunction getfunction, lua_CFunction setfunction)
{
	lua_pushstring(L, DEFAULTINTINDEXGET);
	if (getfunction)
	{
		lua_pushvalue(L, userdata);
		lua_pushcclosure(L, getfunction, 1);
	}
	else
		lua_pushnil(L);

	lua_settable(L, metatable);

	lua_pushstring(L, DEFAULTINTINDEXSET);
	if (setfunction)
	{
		lua_pushvalue(L, userdata);
		lua_pushcclosure(L, setfunction, 1);
	}
	else
		lua_pushnil(L);

	lua_settable(L, metatable);
	return true;
}

bool LuaClass::RegisterDefaultStringArrayProperty(lua_State* L, MT metatable, UD userdata, lua_CFunction getfunction, lua_CFunction setfunction)
{
	lua_pushstring(L, DEFAULTSTRINGINDEXGET);
	if (getfunction)
	{
		lua_pushvalue(L, userdata);
		lua_pushcclosure(L, getfunction, 1);
	}
	else
		lua_pushnil(L);

	lua_settable(L, metatable);

	lua_pushstring(L, DEFAULTSTRINGINDEXSET);
	if (setfunction)
	{
		lua_pushvalue(L, userdata);
		lua_pushcclosure(L, setfunction, 1);
	}
	else
		lua_pushnil(L);

	lua_settable(L, metatable);
	return true;
}

int LuaClass::Index(lua_State* L)
{
	int top = lua_gettop(L);
	int type = 0;
	int output = 0;
	if (top == 2)
	{
		lua_getmetatable(L, 1);
		int metatable = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, metatable);
		if (lua_istable(L, -1))
		{
			type = lua_type(L, -1);
			lua_pushstring(L, "__getter");
			lua_gettable(L, -2);
			type = lua_type(L, -1);
			if (lua_isfunction(L, -1))
			{
				//The getter functions must load the value to the lua stack when called
				lua_call(L, 0, 1);
				return 1;
			}
			else if (lua_istable(L, -1))
			{
				
			}
			else //return the table that was stored in the metatable (so undo the result of getting __get)
				//lua_pop(L, 1);
			type = lua_type(L, -1);
		}
		else
		{
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				if (lua_isnumber(L, 2))
				{
					lua_pushstring(L, DEFAULTINTINDEXGET);
					lua_gettable(L, metatable);
					if (lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 2);
						lua_call(L, 1, 1);
						return 1;
					}
					else
						lua_pop(L, 1);
				}
				else if (lua_type(L, 2) == LUA_TSTRING)
				{
					lua_pushstring(L, DEFAULTSTRINGINDEXGET);
					lua_gettable(L, metatable);
					if (lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 2);
						lua_call(L, 1, 1);
						return 1;
					}
					else
						lua_pop(L, 1);
				}
				lua_pushstring(L, "__norealclass");
				lua_gettable(L, metatable);
				if (lua_isboolean(L, -1) && lua_toboolean(L, -1))
				{
					lua_pop(L, 1);
					lua_pushnil(L);
				}

				//Let's see if this is a published property
				//lua_pushcfunction(L, lua_getProperty);
				//lua_pushvalue(L, 1); //userdata
				//lua_pushvalue(L, 2); //keyname
				//lua_call(L, 2, 1);
			}
		}
		//if (lua_isfunction(L, -1))
		return 1;
	}
	return output;
}

int LuaClass::NewIndex(lua_State* L)
{
	// __newindex = (self = -3/1, key = -2/2, value = -1/3)
	int top = lua_gettop(L);
	int output = 0;
	if (top == 3)
	{
		lua_getmetatable(L, 1);
		int metatable = lua_gettop(L);
		lua_pushvalue(L, 2);
		//Lua_Print(L, lua_tostring(L, -1));
		lua_gettable(L, metatable);
		if (lua_istable(L, -1))
		{
			lua_pushstring(L, "__setter");
			lua_gettable(L, -2);
			if (lua_isfunction(L, -1))
			{
				lua_pushvalue(L, 3);
				lua_call(L, 1, 0);
				output = 1;
			}
		}
	}
	return 0;
}

int LuaClass::GC(lua_State* L)
{
	lua_getmetatable(L, 1);
	int top = lua_gettop(L);
	if (!lua_istable(L, -1))
		return 0;
	lua_pushstring(L, "__autodestroy");
	lua_gettable(L, top);

	if (lua_toboolean(L, -1))
	{
		lua_pushstring(L, "OnDestroy");
		lua_gettable(L, top);
		if (lua_isfunction(L, -1))
			lua_call(L, 0, 0);
	}

	return 0;
}

LuaObject* LuaClass::GetClassObject(lua_State* L, size_t matchhash)
{
	//static_assert(std::is_base_of<LuaObject, T>::value, "Error: T is not a type of LuaObject");
	try
	{
		if (lua_type(L, lua_upvalueindex(1)) == LUA_TUSERDATA)
		{
			return *(LuaObject**)lua_touserdata(L, lua_upvalueindex(1));
		}
		/*int top = lua_gettop(L);
		while (top > 0)
		{
			int type = lua_type(L, -top);
			if (type == LUA_TUSERDATA)
			{
				if (matchhash)
				{
					LuaObject* retv = *(LuaObject**)lua_touserdata(L, -top);
					size_t hash = retv->klasstype.hash_code();
					if (hash == matchhash)
						return (LuaObject*)retv;
				}
				else
					return *(LuaObject**)lua_touserdata(L, -top);
			}
			else if (type == LUA_TLIGHTUSERDATA)
				return (LuaObject*)lua_touserdata(L, -top);

			top--;
		}*/
	}
	catch (const std::exception& e)
	{
		Lua_Print(L, e.what());
	}

	return nullptr;
}

bool LuaClass::DestroyLuaObject(lua_State* L, LuaObject* lo)
{
	if (lua_type(L, lua_upvalueindex(1)) == LUA_TUSERDATA)
	{
		void** ud = reinterpret_cast<void**>(lua_touserdata(L, lua_upvalueindex(1)));
		if (ud && *ud == lo)
		{
			if (lua_getmetatable(L, lua_upvalueindex(1)) == 1) //If has a metatable then;
			{
				lua_pop(L, 1);
				//Empty metatable
				lua_newtable(L);
				lua_setmetatable(L, lua_upvalueindex(1));
			}
			*ud = nullptr;
			return true;
		}
	}
	return false;
}

bool LuaClass::AddToLuaClass(LuaObject* object, MetaDataRegister registers, bool bForceDelete, OnDestroyFunc destructor)
{
	if (!object)
		return false;
	size_t hash = object->klasstype.hash_code();
	std::vector<TheClass>& storer = GetTheStore();
	for (TheClass& c : storer)
	{
		if (c.hash == hash)
			return false;
	}
	storer.push_back(TheClass(hash, ClassEntry({ object->klasstype, registers, destructor, true })));
	storer.shrink_to_fit();
	if (bForceDelete)
		delete object;
	return true;
}

MetaDataRegister LuaClass::FindObjectMetadata(size_t hash)
{
	std::vector<TheClass>& storer = GetTheStore();
	for (TheClass& c : storer)
	{
		if (c.hash == hash)
			return c.entry.Register;
	}
	return MetaDataRegister(0);

}

ClassEntry LuaClass::FindObjectClassEntry(size_t hash)
{
	std::vector<TheClass>& storer = GetTheStore();
	for (TheClass& c : storer)
	{
		if (c.hash == hash)
			return c.entry;
	}
	return ClassEntry();

}
