#include "LuaObject.h"
//#define _CRT_SECURE_NO_WARNINGS 1
void Lua_Print(lua_State* L, const char* message)
{
	//luaL_dostring(L, string_format("print('%s')", message).c_str());
	lua_getglobal(L, "print");
	lua_pushstring(L, message);
	if (lua_pcall(L, 1, 1, 0) != LUA_OK)
	{
		//error
	}
}
void Lua_Print(lua_State* L, std::string& message)
{
	Lua_Print(L, message.c_str());
}