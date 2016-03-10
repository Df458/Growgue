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
    act->name = strdup("Creature");

    xmlChar* a = 0;
    for(xmlNodePtr node = root->children; node; node = node->next) {
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"display")) {
            if((a = xmlGetProp(node, (const xmlChar*)"color"))) {
                act->color = color_str((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"char"))) {
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
    }
    xmlFreeDoc(doc);

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
    int x, y, dx, dy;
    player_get_position(&x, &y);
    dx = abs(act->x - x);
    dy = abs(act->y - y);
    if(dx <= act->aggro && dy <= act->aggro) {
        if(step_towards_player(act, cmap)) {
        } else {
            if(dx <= 1 && dy <= 1) {
                int dmg = damage_player(act->str);
                char* damage_text = 0;
                if(dmg > 0) {
                    int len = snprintf(0, 0, "The %s hits you for %d damage", act->name, dmg);
                    damage_text = calloc(len + 1, sizeof(char));
                    snprintf(damage_text, len + 1, "The %s hits you for %d damage", act->name, dmg);
                } else {
                    int len = snprintf(0, 0, "The %s misses you", act->name);
                    damage_text = calloc(len + 1, sizeof(char));
                    snprintf(damage_text, len + 1, "The %s misses you", act->name);
                }
                add_message(COLOR_DEFAULT, damage_text);
                free(damage_text);
                lua_getglobal(act->script_state, "attack");
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
        }
    }
}

void kill_actor(actor* act)
{
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
    return damage - act->def;
}
