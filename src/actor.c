#include "actor.h"
#include "color.h"
#include "loader.h"
#include "log.h"
#include "luafunc.h"

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

    xmlChar* a = 0;
    for(xmlNodePtr node = root->children; node; node = node->next) {
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"display")) {
            if((a = xmlGetProp(node, (const xmlChar*)"color"))) {
                act->color = color_str((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"display"))) {
                act->display = a[0];
                free(a);
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

void kill_actor(actor* act)
{
    fprintf(stderr, "kill_actor() stub!\n");
    free(act);
}

int lua_kill_actor(lua_State* state)
{
    fprintf(stderr, "lua_kill_actor() stub!\n");
    return 0;
}
