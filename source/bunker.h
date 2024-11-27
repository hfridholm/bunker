/*
 *
 */

#ifndef BUNKER_H
#define BUNKER_H

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "file.h"

typedef struct
{
  char* name;
  char* address;
  int   port;
} room_t;

extern char* getstr(const char* format, ...);


extern int address_and_port_split(char** address, int* port, const char* string);

extern int address_and_port_add(char* address, int port, char* name);

extern int address_and_port_get(char** address, int* port, const char* string);


extern int  rooms_load(room_t** rooms, size_t* count);

extern int  rooms_save(room_t* rooms, size_t count);

extern void rooms_free(room_t** rooms, size_t count);

extern int room_del(room_t** rooms, size_t* count, const char* name);

#endif // BUNKER_H
