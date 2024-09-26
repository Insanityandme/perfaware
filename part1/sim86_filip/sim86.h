#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef char unsigned u8;
typedef short unsigned u16;
typedef int unsigned u32;
typedef long long unsigned u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef s32 b32;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

static u32 const SIM86_VERSION = 4;
