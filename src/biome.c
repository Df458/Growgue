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
    b->floor_count = 0;
    b->wall_count = 0;
    b->floor_chars = 0;
    b->wall_chars = 0;
    b->water_min = 0;
    b->water_max = 0;
    b->nutrients_min = 0;
    b->nutrients_max = 0;
    b->minerals_min = 0;
    b->minerals_max = 0;

    xmlChar* a = 0;
    for(xmlNodePtr node = root->children; node; node = node->next) {
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name, (const xmlChar*)"tiles")) {
            for(xmlNodePtr pnode = node->children; pnode; pnode = pnode->next) {
                if(pnode->type == XML_ELEMENT_NODE && !xmlStrcmp(pnode->name, (const xmlChar*)"floor")) {
                    b->floor_chars = realloc(b->floor_chars, (b->floor_count + 1) * sizeof(int));
                    b->floor_chars[b->floor_count * 2] = '.';
                    b->floor_chars[b->floor_count * 2 + 1] = COLOR_DEFAULT;
                    if((a = xmlGetProp(pnode, (const xmlChar*)"color"))) {
                        b->floor_chars[b->floor_count * 2 + 1] = color_str((char*)a);
                        free(a);
                        a = 0;
                    }
                    if((a = xmlGetProp(pnode, (const xmlChar*)"char"))) {
                        if(strlen((char*)a) > 1)
                            b->floor_chars[b->floor_count * 2] = atoi((char*)a);
                        else
                            b->floor_chars[b->floor_count * 2] = a[0];
                        free(a);
                        a = 0;
                    }
                    b->floor_count++;
                }
                if(pnode->type == XML_ELEMENT_NODE && !xmlStrcmp(pnode->name, (const xmlChar*)"wall")) {
                    b->wall_chars = realloc(b->wall_chars, (b->wall_count + 1) * sizeof(int));
                    b->wall_chars[b->wall_count * 2] = '.';
                    b->wall_chars[b->wall_count * 2 + 1] = COLOR_DEFAULT;
                    if((a = xmlGetProp(pnode, (const xmlChar*)"color"))) {
                        b->wall_chars[b->wall_count * 2 + 1] = color_str((char*)a);
                        free(a);
                        a = 0;
                    }
                    if((a = xmlGetProp(pnode, (const xmlChar*)"char"))) {
                        if(strlen((char*)a) > 1)
                            b->wall_chars[b->wall_count * 2] = atoi((char*)a);
                        else
                            b->wall_chars[b->wall_count * 2] = a[0];
                        free(a);
                        a = 0;
                    }
                    b->wall_count++;
                }
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

    if(b->floor_count == 0) {
        b->floor_count = 1;
        b->floor_chars = calloc(2, sizeof(int));
        b->floor_chars[0] = '.';
        b->floor_chars[1] = COLOR_DEFAULT;
    }
    if(b->wall_count == 0) {
        b->wall_count = 1;
        b->wall_chars = calloc(2, sizeof(int));
        b->wall_chars[0] = '#';
        b->wall_chars[1] = COLOR_DEFAULT;
    }

    return b;
}

void destroy_biome(biome* b)
{
    free(b->floor_chars);
    free(b->wall_chars);
    free(b);
}

void apply_biome(tile* t, biome* b)
{
    if(!t->can_till) {
        if(t->solid) {
            int index = rand() % b->wall_count;
            t->display = b->wall_chars[index * 2];
            t->color = b->wall_chars[index * 2 + 1];
        } else {
            int index = rand() % b->floor_count;
            t->display = b->floor_chars[index * 2];
            t->color = b->floor_chars[index * 2 + 1];
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
