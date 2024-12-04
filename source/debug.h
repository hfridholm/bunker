/*
 * debug.h - declarations of functions in debug.c
 *
 * This header file is independent of debug.c
 *
 * Written by Hampus Fridholm
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

extern int debug_print(FILE* stream, const char* title, const char* format, ...);

extern int error_print(const char* format, ...);

extern int info_print(const char* format, ...);

#endif // DEBUG_H
