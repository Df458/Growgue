#define true TRUE
#define false FALSE
#ifdef PDCURSES
#include <xcurses/panel.h>
#else
#include <panel.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "color.h"
#include "input.h"
#include "item.h"
#include "luafunc.h"
#include "loader.h"
#include "player.h"
#include "log.h"
#include "reqs.h"

static WINDOW* inv_win = 0;
static PANEL* inv_panel   = 0;

const char* purpose_list[] = 
{
    "Pick up an item",
    "Drop an item",
    "Apply an item",
    "Plant an item",
    "Equip an item",
    "Remove an item",
};

void init_items()
{
    int rows, cols;
    max_size(stdscr, rows, cols);
    inv_win= newwin(rows, cols / 2, 0, cols / 2 - 1);
    inv_panel  = new_panel(inv_win);
}

void cleanup_items()
{
    del_panel(inv_panel);
    delwin(inv_win);
}

item* create_item(const char* file)
{
    xmlDocPtr doc = load_xml(file);
    if(!doc) {
        fprintf(stderr, "Can't load item file\n");
        return 0;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    for(; root; root = root->next)
        if(root->type == XML_ELEMENT_NODE && !xmlStrcmp(root->name, (const xmlChar*)"item"))
            break;
    if(!root) {
        fprintf(stderr, "Item file is invalid\n");
        return 0;
    }

    item* it = malloc(sizeof(item));
    it->script_state = luaL_newstate();

    luaL_openlibs(it->script_state);
    lua_newtable(it->script_state);
    luaL_setfuncs(it->script_state, global_funcs, 0);
    lua_setglobal(it->script_state, "game");
    insert_player_into_lua(it->script_state);

    it->on_ground = false;
    it->display = '.';
    it->color = COLOR_DEFAULT;
    it->name = strdup("Item");
    it->count = 1;
    it->can_equip = false;
    it->can_use = false;
    it->can_plant = false;
    it->str = 0;
    it->def = 0;
    it->slot = -1;
    it->orig_path = strdup(file);
    it->plant_id = 0;

    xmlChar* a = 0;
    for(xmlNodePtr node = root->children; node; node = node->next) {
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"display")) {
            if((a = xmlGetProp(node, (const xmlChar*)"color"))) {
                it->color = color_str((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"char"))) {
                it->display = a[0];
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"name"))) {
                free(it->name);
                it->name = (char*)a;
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"stats")) {
            it->can_equip = true;
            if((a = xmlGetProp(node, (const xmlChar*)"slot"))) {
                if(!strcmp((char*)a, "head"))
                    it->slot = SLOT_HEAD;
                else if(!strcmp((char*)a, "body"))
                    it->slot = SLOT_BODY;
                else if(!strcmp((char*)a, "weapon"))
                    it->slot = SLOT_WEAPON;
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"str"))) {
                it->str = atoi((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"def"))) {
                it->def = atoi((char*)a);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"script")) {
            if((a = xmlGetProp(node, (const xmlChar*)"id"))) {
                char* path = create_path((char*)a);
                if(luaL_loadfile(it->script_state, path)) {
                    add_message(COLOR_WARNING, "Failed to prepare script");
                }
                if(lua_isfunction(it->script_state, -1) && lua_pcall(it->script_state, 0, 0, 0)) {
                    const char* err = lua_tostring(it->script_state, -1);
                    add_message(COLOR_WARNING, err);
                    lua_pop(it->script_state, 1);
                }
                free(path);
                free(a);
                a = 0;
                lua_getglobal(it->script_state, "apply");
                it->can_use = lua_isfunction(it->script_state, -1);
                lua_pop(it->script_state, 1);
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"plant")) {
            if((a = xmlGetProp(node, (const xmlChar*)"id"))) {
                it->plant_id = strdup((char*)a);
                it->can_plant = true;
                free(a);
                a = 0;
            }
        }
    }
    xmlFreeDoc(doc);

    insert_item_into_lua(it->script_state, it);
    lua_setglobal(it->script_state, "this");

    return it;
}

item* clone_item(const item* it)
{
    item* new_item = create_item(it->orig_path);
    new_item->count = it->count;
    return new_item;
}

void destroy_item(item* item)
{
    lua_close(item->script_state);
    free(item->orig_path);
    free(item->name);
    free(item);
}

item* get_item(item** item_list, int item_count, int purpose, bool auto_select)
{
    if(item_count == 0 || item_list == 0)
        return 0;

    item** temp_list = 0;
    int temp_count = 0;
    int temp_used = 0;
    item*  found_item = 0;

    for(int i = 0; i < item_count; ++i) {
        if(item_list[i] && (purpose != PURPOSE_EQUIP || item_list[i]->can_equip) && (purpose != PURPOSE_APPLY || item_list[i]->can_use) && (purpose != PURPOSE_PLANT || item_list[i]->can_plant)) {
            if(temp_used >= temp_count) {
                temp_count += 5;
                temp_list = realloc(temp_list, sizeof(item*) * temp_count);
            }
            temp_list[temp_used] = item_list[i];
            temp_used++;
        }
    }

    if(!temp_list)
        return 0;
    if(temp_used == 1 && auto_select) {
        found_item = temp_list[0];
        free(temp_list);
        return found_item;
    }

    show_panel(inv_panel);
    update_panels();
    doupdate();

    bool should_continue = true;
    int selected = 0;
    while(should_continue) {
        set_color(inv_win, COLOR_DEFAULT);
        wborder(inv_win, 179, 179, 196, 196, 218, 191, 192, 217);
        mvwaddstr(inv_win, 0, 1, purpose_list[purpose]);
        for(int i = 0; i < temp_used; ++i) {
            set_color(inv_win, COLOR_DEFAULT);
            if(i == selected)
                set_color(inv_win, COLOR_SELECTION);
            mvwprintw(inv_win, i + 1, 1, "%s - %d", temp_list[i]->name, temp_list[i]->count);
        }
        wrefresh(inv_win);
        switch(get_action()) {
            case ACTION_INVALID:
                if(get_last_direction() == DIRECTION_NORTH && selected > 0)
                    --selected;
                else if(get_last_direction() == DIRECTION_SOUTH && selected < temp_used - 1)
                    ++selected;
                break;
            case ACTION_SELECT:
                should_continue = false;
                found_item = temp_list[selected];
                break;
            case ACTION_QUIT:
                should_continue = false;
                found_item = 0;
                break;
            // TODO: Add scrolling
        }
    }

    hide_panel(inv_panel);
    update_panels();
    doupdate();

    free(temp_list);

    return found_item;
}

void insert_item_into_lua(lua_State* state, item* it)
{
    lua_newtable(state);
    item** i = lua_newuserdata(state, sizeof(item*));
    *i = it;
    lua_setfield(state, 1, "instance");
    luaL_setfuncs(state, item_funcs, 0);
    lua_newtable(state);
    luaL_setfuncs(state, item_meta, 0);
    lua_setmetatable(state, 1);
}

int lua_item_index(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the instance is missing!");
    item* it = *(item**)lua_touserdata(state, -1);
    lua_pop(state, 1);
    if(!strcmp(index, "count")) lua_pushinteger(state, it->count);
    else if(!strcmp(index, "x") && it->on_ground) lua_pushinteger(state, it->x);
    else if(!strcmp(index, "y") && it->on_ground) lua_pushinteger(state, it->y);
    else if(!strcmp(index, "name")) lua_pushstring(state, it->name);
    else if(!strcmp(index, "display")) lua_pushinteger(state, it->display);
    else if(!strcmp(index, "color")) lua_pushstring(state, get_color_str(it->color));
    else if(!strcmp(index, "str") && it->can_equip) lua_pushinteger(state, it->str);
    else if(!strcmp(index, "def") && it->can_equip) lua_pushinteger(state, it->def);
    else return 0;
    return 1;
}

int lua_item_newindex(lua_State* state)
{
    const char* index = lua_tostring(state, 2);
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to modify data, but the instance is missing!");
    item* it = *(item**)lua_touserdata(state, -1);
    lua_pop(state, 1);
    if(!strcmp(index, "count")) it->count = lua_tointeger(state, 3);
    /* else if(!strcmp(index, "x") && it->on_ground) lua_pushinteger(state, it->x); */
    /* else if(!strcmp(index, "y") && it->on_ground) lua_pushinteger(state, it->y); */
    else if(!strcmp(index, "name")) { free(it->name); it->name = strdup(lua_tostring(state, 3)); }
    else if(!strcmp(index, "display")) it->display = lua_tointeger(state, 3);
    else if(!strcmp(index, "color")) it->color = color_str(lua_tostring(state, 3));
    else if(!strcmp(index, "str") && it->can_equip) it->str = lua_tointeger(state, 3);
    else if(!strcmp(index, "def") && it->can_equip) it->def = lua_tointeger(state, 3);
    return 0;
}
