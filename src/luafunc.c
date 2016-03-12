#define true TRUE
#define false FALSE
#include <stdlib.h>
#include <string.h>
#include "color.h"
#include "log.h"
#include "luafunc.h"

bool callback(const char* name, lua_State* state)
{
    lua_getglobal(state, name);
    if(!lua_isfunction(state, -1)) {
        lua_pop(state, 1);
        return false;
    } else {
        if(lua_pcall(state, 0, 0, 0)) {
            const char* err = lua_tostring(state, -1);
            add_message(COLOR_WARNING, err);
            lua_pop(state, 1);
        }
        return false;
    }
    return true;
}

int lua_log_message(lua_State* state)
{
    add_message(color_str(lua_tostring(state, 1)), lua_tostring(state, 2));
    return 0;
}

int lua_ask_question(lua_State* state)
{
    if(!lua_isstring(state, 2))
        return 0;
    char* str = strdup(lua_tostring(state, 2));
    if(ask_question(color_str(lua_tostring(state, 1)), str))
        lua_pushboolean(state, 1);
    else
        lua_pushboolean(state, 0);
    free(str);
    return 1;
}
