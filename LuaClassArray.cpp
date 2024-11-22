#include "LuaClassArray.h"

int LuaClassArray::CreateMetaTable(lua_State* L, UD userdata, lua_CFunction getArrayFunc, lua_CFunction setArrayFunc, lua_CFunction rawgetter)
{
    lua_newtable(L);
    int top = lua_gettop(L);

    lua_pushstring(L, "__index");
    lua_pushvalue(L, userdata);
    lua_pushcfunction(L, getArrayFunc);
    lua_pushcfunction(L, rawgetter);
    lua_pushcclosure(L, Index, 3);
    lua_settable(L, top);
    if (setArrayFunc)
    {
        lua_pushstring(L, "__newindex");
        lua_pushvalue(L, userdata);
        lua_pushcfunction(L, setArrayFunc);
        lua_pushcclosure(L, NewIndex, 2);
        lua_settable(L, top);
    }
    return top;
}

int LuaClassArray::Index(lua_State* L)
{
    if (strcmp(lua_tostring(L, 2), "__getter")==0)
    {
        lua_pushvalue(L, 1);
        return 1;
    }
    else if (strcmp(lua_tostring(L, 2), "__raw") == 0)
    {
        lua_CFunction cf = lua_tocfunction(L, lua_upvalueindex(3));
        if (cf != NULL)
        {
            lua_pushvalue(L, lua_upvalueindex(1)); //Userdata
            lua_pushcclosure(L, cf, 1); //function
            lua_pushvalue(L, 2); //index
            lua_call(L, 1, 1);
        }
        else
            lua_pushvalue(L, 1);
        return 1;
    }
    lua_CFunction cf = lua_tocfunction(L, lua_upvalueindex(2));
    lua_pushvalue(L, lua_upvalueindex(1)); //Userdata
    lua_pushcclosure(L, cf, 1); //function
    lua_pushvalue(L, 2); //index
    lua_call(L, 1, 1);

    return 1;
}

int LuaClassArray::NewIndex(lua_State* L)
{
    if (strcmp(lua_tostring(L, 2), "__set"))
    {
        lua_pushvalue(L, 1);
        return 1;
    }

    lua_CFunction cf = lua_tocfunction(L, lua_upvalueindex(2));
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushcclosure(L, cf, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_call(L, 2, 0);

    return 1;
}
