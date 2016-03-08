#ifndef LOADER_H
#define LOADER_H
#include <libxml/parser.h>

char* create_path(const char* path);
xmlDocPtr load_xml(const char* path);

#endif
