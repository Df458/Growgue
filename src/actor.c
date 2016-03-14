#define true TRUE
#define false FALSE
#include <string.h>

#include "actor.h"
#include "color.h"
#include "loader.h"
#include "log.h"
#include "luafunc.h"
#include "map.h"
#include "player.h"

actor* create_actor(const char* file)
{
    xmlDocPtr doc = load_xml(file);
    if(!doc)
        return 0;
    xmlNodePtr root = xmlDocGetRootElement(doc);
    for(; root; root = root->next)
        if(root->type == XML_ELEMENT_NODE && !xmlStrcmp(root->name, (const xmlChar*)"actor"))
            break;
    if(!root)
        return 0;

    actor* act = malloc(sizeof(actor));
    act->script_state = luaL_newstate();

    luaL_openlibs(act->script_state);
    lua_newtable(act->script_state);
    luaL_setfuncs(act->script_state, global_funcs, 0);
    lua_setglobal(act->script_state, "game");

    act->to_kill = false;
    act->hp_max = 0;
    act->hp = 0;
    act->str = 0;
    act->def = 0;
    act->xp = 0;
    act->name = strdup("Creature");
    act->drops = 0;
    act->drop_rarities = 0;
    act->drop_counts = 0;
    act->drop_count = 0;
    act->aggro = 0;
    act->angry = false;

    xmlChar* a = 0;
    for(xmlNodePtr node = root->children; node; node = node->next) {
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"display")) {
            if((a = xmlGetProp(node, (const xmlChar*)"color"))) {
                act->color = color_str((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"char"))) {
                if(strlen((char*)a) > 1)
                    act->display = atoi((char*)a);
                else
                    act->display = a[0];
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"name"))) {
                free(act->name);
                act->name = (char*)a;
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"stats")) {
            if((a = xmlGetProp(node, (const xmlChar*)"hp"))) {
                act->hp_max = atoi((char*)a);
                act->hp = act->hp_max;
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"str"))) {
                act->str = atoi((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"def"))) {
                act->def = atoi((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"xp"))) {
                act->xp = atoi((char*)a);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"aggro")) {
            if((a = xmlGetProp(node, (const xmlChar*)"radius"))) {
                act->aggro = atoi((char*)a);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"script")) {
            if((a = xmlGetProp(node, (const xmlChar*)"id"))) {
                char* path = create_path((char*)a);
                if(luaL_loadfile(act->script_state, path)) {
                    add_message(COLOR_WARNING, "Failed to prepare script");
                }
                if(lua_isfunction(act->script_state, -1) && lua_pcall(act->script_state, 0, 0, 0)) {
                    const char* err = lua_tostring(act->script_state, -1);
                    add_message(COLOR_WARNING, err);
                    lua_pop(act->script_state, 1);
                }
                free(path);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"drops")) {
            for(xmlNodePtr pnode = node->children; pnode; pnode = pnode->next) {
                if(pnode->type == XML_ELEMENT_NODE && !xmlStrcmp(pnode->name, (const xmlChar*)"item")) {
                    if((a = xmlGetProp(pnode, (const xmlChar*)"id"))) {
                        char* path = strdup((char*)a);
                        act->drop_count++;
                        act->drops = realloc(act->drops, act->drop_count * sizeof(char*));
                        act->drop_counts = realloc(act->drop_counts, act->drop_count * sizeof(char*));
                        act->drop_rarities = realloc(act->drop_rarities, act->drop_count * sizeof(char*));
                        act->drops[act->drop_count - 1] = path;
                        act->drop_counts[act->drop_count - 1] = 1;
                        act->drop_rarities[act->drop_count - 1] = 1;
                        free(a);
                        a = 0;
                    } else
                        continue;
                    if((a = xmlGetProp(pnode, (const xmlChar*)"rarity"))) {
                        act->drop_rarities[act->drop_count - 1] = atoi((char*)a);
                        free(a);
                        a = 0;
                    }
                    if((a = xmlGetProp(pnode, (const xmlChar*)"count"))) {
                        act->drop_counts[act->drop_count - 1] = atoi((char*)a);
                        free(a);
                        a = 0;
                    }
                }
            }
        }
    }
    xmlFreeDoc(doc);

    insert_actor_into_lua(act->script_state, act);
    lua_setglobal(act->script_state, "this");

    return act;
}

void init_actor(actor* act)
{
    lua_getglobal(act->script_state, "create");
    if(!lua_isfunction(act->script_state, -1)) {
        lua_pop(act->script_state, 1);
    } else {
        if(lua_pcall(act->script_state, 0, 0, 0)) {
            const char* err = lua_tostring(act->script_state, -1);
            add_message(COLOR_WARNING, err);
            lua_pop(act->script_state, 1);
        }
    }
}

void update_actor(actor* act, struct map* cmap)
{
    callback("update", act->script_state);
    int x, y, dx, dy;
    player_get_position(&x, &y);
    dx = abs(act->x - x);
    dy = abs(act->y - y);
    if((dx <= act->aggro && dy <= act->aggro) || act->angry) {
        if(step_towards_player(act, cmap)) {
        } else {
            if(dx <= 1 && dy <= 1) {
                int dmg = damage_player(act->str);
                if(dmg > 0) {
                    printf_message(COLOR_DEFAULT, "The %s hits you for %d damage", act->name, dmg);
                } else {
                    printf_message(COLOR_DEFAULT, "The %s misses you", act->name);
                }
                callback("attack", act->script_state);
            }
        }
    } else {
        step(act, act->x + (rand() % 3 - 1), act->y + (rand() % 3 - 1), cmap);
    }
    if(act->hp <= 0) {
        act->to_kill = true;
        add_xp(act->xp);
    }
}

void kill_actor(actor* act, bool force)
{
    if(!force) {
        lua_getglobal(act->script_state, "kill");
        if(!lua_isfunction(act->script_state, -1)) {
            lua_pop(act->script_state, 1);
        } else {
            if(lua_pcall(act->script_state, 0, 0, 0)) {
                const char* err = lua_tostring(act->script_state, -1);
                add_message(COLOR_WARNING, err);
                lua_pop(act->script_state, 1);
            }
        }
    }
    lua_close(act->script_state);
    if(act->drop_count) {
        for(int i = 0; i < act->drop_count; ++i)
            free(act->drops[i]);
        free(act->drops);
        free(act->drop_rarities);
        free(act->drop_counts);
    }
    free(act->name);
    free(act);
}

int lua_kill_actor(lua_State* state)
{
    fprintf(stderr, "lua_kill_actor() stub!\n");
    return 0;
}

int damage_actor(actor* act, int damage)
{
    act->hp -= damage - act->def;
    if(!act->angry) {
        act->angry = true;
        if(act->aggro == 0) {
            printf_message(COLOR_WARNING, "The %s gets angry!", act->name);
        }
    }
    if(act->hp <= 0)
        act->to_kill = true;
    return damage - act->def;
}

void drop_loot(actor* act, map* cmap)
{
    for(int i = 0; i < act->drop_count; ++i) {
        if(!(rand() % act->drop_rarities[i])) {
            item* it = create_item(act->drops[i]);
            if(!it)
                continue;
            it->count = act->drop_counts[i];
            place_item(act->x, act->y, it, cmap);
        }
    }
}

void insert_actor_into_lua(lua_State* state, actor* it)
{
    lua_newtable(state);
    actor** i = lua_newuserdata(state, sizeof(actor*));
    *i = it;
    lua_setfield(state, 1, "instance");
    luaL_setfuncs(state, actor_funcs, 0);
    lua_newtable(state);
    luaL_setfuncs(state, actor_meta, 0);
    lua_setmetatable(state, 1);
}

int lua_actor_index(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the instance is missing!");
    actor* it = *(actor**)lua_touserdata(state, -1);
    lua_pop(state, 1);
    if(!strcmp(index, "aggro")) lua_pushinteger(state, it->aggro);
    else if(!strcmp(index, "x")) lua_pushinteger(state, it->x);
    else if(!strcmp(index, "y")) lua_pushinteger(state, it->y);
    else if(!strcmp(index, "name")) lua_pushstring(state, it->name);
    else if(!strcmp(index, "display")) lua_pushinteger(state, it->display);
    else if(!strcmp(index, "color")) lua_pushstring(state, get_color_str(it->color));
    else if(!strcmp(index, "str")) lua_pushinteger(state, it->str);
    else if(!strcmp(index, "def")) lua_pushinteger(state, it->def);
    else if(!strcmp(index, "hp")) lua_pushinteger(state, it->hp);
    else if(!strcmp(index, "hp_max")) lua_pushinteger(state, it->hp_max);
    else return 0;
    return 1;
}

int lua_actor_newindex(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to modify data, but the instance is missing!");
    actor* it = *(actor**)lua_touserdata(state, -1);
    lua_pop(state, 1);
    if(!strcmp(index, "aggro")) it->aggro = lua_tointeger(state, 3);
    /* else if(!strcmp(index, "x") && it->on_ground) lua_pushinteger(state, it->x); */
    /* else if(!strcmp(index, "y") && it->on_ground) lua_pushinteger(state, it->y); */
    else if(!strcmp(index, "name")) { free(it->name); it->name = strdup(lua_tostring(state, 3)); }
    else if(!strcmp(index, "display")) it->display = lua_tointeger(state, 3);
    else if(!strcmp(index, "color")) it->color = color_str(lua_tostring(state, 3));
    else if(!strcmp(index, "str")) it->str = lua_tointeger(state, 3);
    else if(!strcmp(index, "def")) it->def = lua_tointeger(state, 3);
    else if(!strcmp(index, "hp")) it->hp = lua_tointeger(state, 3);
    else if(!strcmp(index, "hp_max")) it->hp_max = lua_tointeger(state, 3);
    return 0;
}
