#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "biome.h"
#include "color.h"
#include "loader.h"

biome* create_biome(const char* file)
{
    xmlDocPtr doc = load_xml(file);
    if(!doc) {
        fprintf(stderr, "Can't load item file\n");
        return 0;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    for(; root; root = root->next)
        if(root->type == XML_ELEMENT_NODE && !xmlStrcmp(root->name, (const xmlChar*)"biome"))
            break;
    if(!root) {
        fprintf(stderr, "Biome file is invalid\n");
        return 0;
    }
    biome* b = malloc(sizeof(biome));
    b->floor_char = '.';
    b->wall_char = '#';
    b->floor_color = COLOR_DEFAULT;
    b->wall_color = COLOR_DEFAULT;
    b->water_min = 0;
    b->water_max = 0;
    b->nutrients_min = 0;
    b->nutrients_max = 0;
    b->minerals_min = 0;
    b->minerals_max = 0;

    xmlChar* a = 0;
    for(xmlNodePtr node = root->children; node; node = node->next) {
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"floor")) {
            if((a = xmlGetProp(node, (const xmlChar*)"color"))) {
                b->floor_color = color_str((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"char"))) {
                b->floor_char = a[0];
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"wall")) {
            if((a = xmlGetProp(node, (const xmlChar*)"color"))) {
                b->wall_color = color_str((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"char"))) {
                b->wall_char = a[0];
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"water")) {
            if((a = xmlGetProp(node, (const xmlChar*)"min"))) {
                b->water_min = atof((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"max"))) {
                b->water_max = atof((char*)a);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"nutrients")) {
            if((a = xmlGetProp(node, (const xmlChar*)"min"))) {
                b->nutrients_min = atof((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"max"))) {
                b->nutrients_max = atof((char*)a);
                free(a);
                a = 0;
            }
        }
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"minerals")) {
            if((a = xmlGetProp(node, (const xmlChar*)"min"))) {
                b->minerals_min = atof((char*)a);
                free(a);
                a = 0;
            }
            if((a = xmlGetProp(node, (const xmlChar*)"max"))) {
                b->minerals_max = atof((char*)a);
                free(a);
                a = 0;
            }
        }
    }
    xmlFreeDoc(doc);

    return b;
}

void destroy_biome(biome* b)
{
    free(b);
}

void apply_biome(tile* t, biome* b)
{
    if(!t->can_till) {
        if(t->solid) {
            t->display = b->wall_char;
            t->color = b->wall_color;
        } else {
            t->display = b->floor_char;
            t->color = b->floor_color;
        }
    }
    if(b->water_min >= b->water_max)
        t->water = b->water_min;
    else
        t->water = rand() % (int)(b->water_max - b->water_min) + (int) b->water_min;
    if(b->nutrients_min >= b->nutrients_max)
        t->nutrients = b->nutrients_min;
    else
        t->nutrients = rand() % (int)(b->nutrients_max - b->nutrients_min) + (int) b->nutrients_min;
    if(b->minerals_min >= b->minerals_max)
        t->minerals = b->minerals_min;
    else
        t->minerals = rand() % (int)(b->minerals_max - b->minerals_min) + (int) b->minerals_min;
}
