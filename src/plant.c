#include <string.h>
#include <stdlib.h>

#include "color.h"
#include "loader.h"
#include "log.h"
#include "luafunc.h"
#include "map.h"
#include "plant.h"
#include "player.h"

plant* create_plant(const char* file, int x, int y, map* ref)
{
    xmlDocPtr doc = load_xml(file);
    if(!doc) {
        fprintf(stderr, "Can't load item file\n");
        return 0;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    for(; root; root = root->next)
        if(root->type == XML_ELEMENT_NODE && !xmlStrcmp(root->name, (const xmlChar*)"plant"))
            break;
    if(!root) {
        fprintf(stderr, "Plant file is invalid\n");
        return 0;
    }

    plant* pl = malloc(sizeof(plant));
    pl->script_state = luaL_newstate();

    luaL_openlibs(pl->script_state);
    lua_newtable(pl->script_state);
    luaL_setfuncs(pl->script_state, global_funcs, 0);
    lua_setglobal(pl->script_state, "game");
    insert_player_into_lua(pl->script_state);

    pl->display = '.';
    pl->color = COLOR_DEFAULT;
    pl->name = strdup("plant");
    pl->can_harvest = false;
    pl->item_id = 0;
    pl->item_count = 0;
    pl->x = x;
    pl->y = y;
    pl->map_ref = ref;
    pl->dead = false;

    xmlChar* a = 0;
    for(xmlNodePtr node = root->children; node; node = node->next) {
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"display")) {
            if((a = xmlGetProp(node, (const xmlChar*)"color"))) {
                pl->color = color_str((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"char"))) {
                pl->display = a[0];
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"name"))) {
                free(pl->name);
                pl->name = strdup((char*)a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"script")) {
            if((a = xmlGetProp(node, (const xmlChar*)"id"))) {
                char* path = create_path((char*)a);
                if(luaL_loadfile(pl->script_state, path)) {
                    add_message(COLOR_WARNING, "Failed to prepare script");
                }
                if(lua_isfunction(pl->script_state, -1) && lua_pcall(pl->script_state, 0, 0, 0)) {
                    const char* err = lua_tostring(pl->script_state, -1);
                    add_message(COLOR_WARNING, err);
                    lua_pop(pl->script_state, 1);
                }
                free(path);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"stats")) {
            if((a = xmlGetProp(node, (const xmlChar*)"time"))) {
                pl->growth_time = atoi((char*) a);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"needs")) {
            if((a = xmlGetProp(node, (const xmlChar*)"water"))) {
                pl->water_need = atof((char*) a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"nutrients"))) {
                pl->nutrient_need = atof((char*) a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"minerals"))) {
                pl->mineral_need = atof((char*) a);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"harvest")) {
            pl->can_harvest = true;
            if((a = xmlGetProp(node, (const xmlChar*)"id"))) {
                pl->item_id = strdup((char*)a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"count"))) {
                pl->item_count = atoi((char*) a);
                free(a);
                a = 0;
            }
        }
    }
    xmlFreeDoc(doc);

    insert_plant_into_lua(pl->script_state, pl);
    lua_setglobal(pl->script_state, "this");

    return pl;
}

void update_plant(plant* pl, int delta)
{
    callback("update", pl->script_state);
    if(pl->growth_time == 0)
        return;
    pl->growth_time -= delta;
    float w, n, m;
    int chance = 0;
    get_tile_growth_info(pl->x, pl->y, &w, &n, &m, pl->map_ref);
    if(w < pl->water_need)
        chance++;
    if(n < pl->nutrient_need)
        chance++;
    if(m < pl->mineral_need)
        chance++;
    for(int i = 0; i < delta; ++i) {
        if(rand() % 100 < chance * 3) {
            pl->dead = true;
            return;
        }
    }

    if(pl->growth_time <= 0) {
        pl->growth_time = 0;
        callback("grow", pl->script_state);
        update_tile_growth_info(pl->x, pl->y, pl->water_need, pl->nutrient_need, pl->mineral_need, pl->map_ref);
    }
}

void kill_plant(plant* pl)
{
    pl->map_ref->tiles[pl->y * pl->map_ref->width + pl->x].plant_ref = 0;
    free(pl->item_id);
    free(pl->name);
    lua_close(pl->script_state);
    free(pl);
}

void insert_plant_into_lua(lua_State* state, plant* it)
{
    lua_newtable(state);
    plant** i = lua_newuserdata(state, sizeof(plant*));
    *i = it;
    lua_setfield(state, 1, "instance");
    luaL_setfuncs(state, plant_funcs, 0);
    lua_newtable(state);
    luaL_setfuncs(state, plant_meta, 0);
    lua_setmetatable(state, 1);
}

int lua_plant_index(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the instance is missing!");
    plant* pl = *(plant**)lua_touserdata(state, -1);
    lua_pop(state, 1);
    if(!strcmp(index, "timer")) lua_pushinteger(state, pl->growth_time);
    else if(!strcmp(index, "water")) lua_pushnumber(state, pl->water_need);
    else if(!strcmp(index, "minerals")) lua_pushnumber(state, pl->mineral_need);
    else if(!strcmp(index, "nutrients")) lua_pushnumber(state, pl->nutrient_need);
    else if(!strcmp(index, "ready")) lua_pushboolean(state, pl->can_harvest && pl->growth_time <= 0);
    else if(!strcmp(index, "x")) lua_pushinteger(state, pl->x);
    else if(!strcmp(index, "y")) lua_pushinteger(state, pl->y);
    else if(!strcmp(index, "name")) lua_pushstring(state, pl->name);
    else if(!strcmp(index, "display")) lua_pushinteger(state, pl->display);
    else if(!strcmp(index, "color")) lua_pushstring(state, get_color_str(pl->color));
    else return 0;
    return 1;
}

int lua_plant_newindex(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to modify data, but the instance is missing!");
    plant* pl = *(plant**)lua_touserdata(state, -1);
    lua_pop(state, 1);
    if(!strcmp(index, "timer")) pl->growth_time = lua_tointeger(state, 3);
    else if(!strcmp(index, "water")) pl->water_need = lua_tonumber(state, 3);
    else if(!strcmp(index, "minerals")) pl->mineral_need = lua_tonumber(state, 3);
    else if(!strcmp(index, "nutrients")) pl->nutrient_need = lua_tonumber(state, 3);
    /* else if(!strcmp(index, "x")) lua_tointeger(state, pl->x); */
    /* else if(!strcmp(index, "y")) lua_tointeger(state, pl->y); */
    else if(!strcmp(index, "name")) { free(pl->name); pl->name = strdup(lua_tostring(state, 3)); }
    else if(!strcmp(index, "display")) pl->display = lua_tointeger(state, 3);
    else if(!strcmp(index, "color")) pl->color = color_str(lua_tostring(state, 3));
    return 0;
}
