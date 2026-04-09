#pragma once
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#ifndef NBT_H
#define NBT_H

struct tag_t;

// TODO: Maybe substitute with specialized payload structs, like (struct list_payload asList;) or something
union payload {
	int8_t asByte;
	int16_t asShort;
	int32_t asInt;
	int64_t asLong;
	_Float32 asFloat;
	_Float64 asDouble;
	int8_t* asBytes;
	char* asString;
	struct tag_t* asList; // Index based, subtags have no name
	struct tag_t* asCompound; // Name based, subtags have names
	int32_t* asInts;
	int64_t* asLongs;
};

typedef struct tag_t {
	int8_t id;
	uint16_t name_length;
	char* name;
	uint32_t length; // For list type payloads, tells how many items are in the list, otherwise dictates payload size in bytes
	union payload payload;
} tag;

/* Retrieve a tag from a given compound tag payload */
tag nbtPayloadGet(const struct tag_t* const compound, int8_t compound_length, const char* const name);
/* Retrieve a tag from a given compound tag */
tag nbtGet(tag compoundTag, const char* const name);
/* Read a tag from a given byte string */
tag nbtRead(const char* bytes);
/* Write a tag into a given byte string */
char* nbtWrite(tag tag, char* bytes);
/* Tally up how many bytes a tag needs in order to be written */
size_t nbtPeekLength(tag tag);
#endif
