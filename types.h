#ifndef TYPES_H
#define TYPES_H

typedef unsigned char U8;
typedef unsigned short U16;

#if UINT_MAX == 4294967295U
typedef unsigned int U32;
#else
typedef unsigned long U32;
#endif

typedef unsigned long long U64;

typedef U8 bool;

enum {false, true};

#endif
