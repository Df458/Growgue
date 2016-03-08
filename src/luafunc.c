#include "color.h"
#include "log.h"
#include "luafunc.h"

int lua_log_message(lua_State* state)
{
    add_message(color_str(lua_tostring(state, 1)), lua_tostring(state, 2));
    return 0;
}
