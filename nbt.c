#include "nbt.h"
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>

#define NBT_LITTLE_ENDIAN 0
#define NBT_BIG_ENDIAN 1

#ifndef NBT_ENDIANNESS
#define NBT_ENDIANNESS NBT_BIG_ENDIAN
#endif

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
    defined(__BIG_ENDIAN__) || \
    defined(__ARMEB__) || \
    defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
// It's a big-endian target architecture
#define NBT_HOST_ENDIAN NBT_BIG_ENDIAN
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
// It's a little-endian target architecture
#define NBT_HOST_ENDIAN NBT_LITTLE_ENDIAN
#else
#define NBT_HOST_ENDIAN -1
// Used when host endianness can't be determined by preprocessor
static int8_t host_endian = -1;
static void set_endianness() {
	int16_t end_check = 0x0100;
	host_endian = *(int8_t*)&end_check;
	printf("In %d Endianness\n", host_endian);
}
#endif


/* Functions to read primitives from bytes */
static int16_t readInt16(const char* const bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		return *(int16_t*)bytes;
	else
		return *(int16_t*)(char[2]){bytes[1], bytes[0]};
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	return *(int16_t*)bytes;
#else
	return *(int16_t*)(char[2]){bytes[1], bytes[0]};
#endif
}

static void writeInt16(int16_t v, char* bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		*(int16_t*)bytes = v;
	else {
		char* b = (char*)&v;
		bytes[0] = b[1];
		bytes[1] = b[0];
	}
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	*(int16_t*)bytes = v;
#else
	char* b = (char*)&v;
	bytes[0] = b[1];
	bytes[1] = b[0];
#endif
}

static uint16_t readUInt16(const char* const bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		return *(uint16_t*)bytes;
	else
		return *(uint16_t*)(char[2]){bytes[1], bytes[0]};
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	return *(uint16_t*)bytes;
#else
	return *(uint16_t*)(char[2]){bytes[1], bytes[0]};
#endif
}

static void writeUInt16(uint16_t v, char* bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		*(uint16_t*)bytes = v;
	else {
		char* b = (char*)&v;
		bytes[0] = b[1];
		bytes[1] = b[0];
	}
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	*(uint16_t*)bytes = v;
#else
	char* b = (char*)&v;
	bytes[0] = b[1];
	bytes[1] = b[0];
#endif
}

static int32_t readInt32(const char* const bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		return *(int32_t*)bytes;
	else
		return *(int32_t*)(char[4]){bytes[3], bytes[2], bytes[1], bytes[0]};
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	return *(int32_t*)bytes;
#else
	return *(int32_t*)(char[4]){bytes[3], bytes[2], bytes[1], bytes[0]};
#endif
}

static void writeInt32(int32_t v, char* bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		*(int32_t*)bytes = v;
	else {
		char* b = (char*)&v;
		bytes[0] = b[3];
		bytes[1] = b[2];
		bytes[2] = b[1];
		bytes[3] = b[0];
	}
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	*(int32_t*)bytes = v;
#else
	char* b = (char*)&v;
	bytes[0] = b[3];
	bytes[1] = b[2];
	bytes[2] = b[1];
	bytes[3] = b[0];
#endif
}


static uint32_t readUInt32(const char* const bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		return *(uint32_t*)bytes;
	else
		return *(uint32_t*)(char[4]){bytes[3], bytes[2], bytes[1], bytes[0]};
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	return *(uint32_t*)bytes;
#else
	return *(uint32_t*)(char[4]){bytes[3], bytes[2], bytes[1], bytes[0]};
#endif
}

static void writeUInt32(uint32_t v, char* bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		*(uint32_t*)bytes = v;
	else {
		char* b = (char*)&v;
		bytes[0] = b[3];
		bytes[1] = b[2];
		bytes[2] = b[1];
		bytes[3] = b[0];
	}
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	*(uint32_t*)bytes = v;
#else
	char* b = (char*)&v;
	bytes[0] = b[3];
	bytes[1] = b[2];
	bytes[2] = b[1];
	bytes[3] = b[0];
#endif
}

static int64_t readInt64(const char* const bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		return *(int64_t*)bytes;
	else
		return *(int64_t*)(char[8]){bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]};
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	return *(int64_t*)bytes;
#else
	return *(int64_t*)(char[8]){bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]};
#endif
}

static void writeInt64(int64_t v, char* bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		*(int64_t*)bytes = v;
	else {
		char* b = (char*)&v;
		bytes[0] = b[7];
		bytes[1] = b[6];
		bytes[2] = b[5];
		bytes[3] = b[4];
		bytes[4] = b[3];
		bytes[5] = b[2];
		bytes[6] = b[1];
		bytes[7] = b[0];
	}
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	*(int64_t*)bytes = v;
#else
	char* b = (char*)&v;
	bytes[0] = b[7];
	bytes[1] = b[6];
	bytes[2] = b[5];
	bytes[3] = b[4];
	bytes[4] = b[3];
	bytes[5] = b[2];
	bytes[6] = b[1];
	bytes[7] = b[0];
#endif
}

static _Float32 readFloat32(const char* const bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		return *(_Float32*)bytes;
	else
		return *(_Float32*)(char[4]){bytes[3], bytes[2], bytes[1], bytes[0]};
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	return *(_Float32*)bytes;
#else
	return *(_Float32*)(char[4]){bytes[3], bytes[2], bytes[1], bytes[0]};
#endif
}

static void writeFloat32(_Float32 v, char* bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		*(_Float32*)bytes = v;
	else {
		char* b = (char*)&v;
		bytes[0] = b[3];
		bytes[1] = b[2];
		bytes[2] = b[1];
		bytes[3] = b[0];
	}
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	*(_Flat32*)bytes = v;
#else
	char* b = (char*)&v;
	bytes[0] = b[3];
	bytes[1] = b[2];
	bytes[2] = b[1];
	bytes[3] = b[0];
#endif
}

static int64_t readFloat64(const char* const bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		return *(_Float64*)bytes;
	else
		return *(_Float64*)(char[8]){bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]};
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	return *(_Float64*)bytes;
#else
	return *(_Float64*)(char[8]){bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]};
#endif
}

static void writeFloat64(_Float64 v, char* bytes) {
#if NBT_HOST_ENDIAN == -1
	if(host_endian == -1)
		set_endianness();
	if(host_endian == NBT_ENDIANNESS)
		*(_Float64*)bytes = v;
	else {
		char* b = (char*)&v;
		bytes[0] = b[7];
		bytes[1] = b[6];
		bytes[2] = b[5];
		bytes[3] = b[4];
		bytes[4] = b[3];
		bytes[5] = b[2];
		bytes[6] = b[1];
		bytes[7] = b[0];
	}
#elif NBT_HOST_ENDIAN == NBT_ENDIANNESS
	*(_Float64*)bytes = v;
#else
	char* b = (char*)&v;
	bytes[0] = b[7];
	bytes[1] = b[6];
	bytes[2] = b[5];
	bytes[3] = b[4];
	bytes[4] = b[3];
	bytes[5] = b[2];
	bytes[6] = b[1];
	bytes[7] = b[0];
#endif
}

/* Return a tag based on a compound tag or payload and name */
tag nbtPayloadGet(const tag* const compound, int8_t compound_length, const char* const name) {
	for(int i = 0; i < compound_length; i++)
		if(strcmp(name, compound[i].name) == 0)
			return compound[i];
	return (tag){0};
}

tag nbtGet(tag compoundTag, const char* const name) {
	return nbtPayloadGet(compoundTag.payload.asCompound, compoundTag.length, name);
}

const char* nbtReadInto(tag* destination, const char* bytes);

const char* nbtReadPayload(int8_t type, uint32_t* length, union payload* payload, const char* bytes) {
	switch(type) {
		case 1:
			payload->asByte = bytes[0];
			*length = 1;
			break;
		case 2:
			payload->asShort = readInt16(bytes);
			*length = 2;
			break;
		case 3:
			payload->asInt = readInt32(bytes);
			*length = 4;
			break;
		case 4:
			payload->asInt = readInt64(bytes);
			*length = 8;
			break;
		case 5:
			payload->asInt = readFloat32(bytes);
			*length = 4;
			break;
		case 6:
			payload->asInt = readFloat64(bytes);
			*length = 8;
			break;
		case 7:
			*length = readUInt32(bytes);
			payload->asBytes = malloc(*length);
			bytes += 4;
			for(int i = 0; i < *length; i++, bytes++)
				payload->asBytes[i] = *bytes;
			return bytes;
		case 8:
			*length = readUInt16(bytes);
			payload->asString = malloc(*length + 1);
			bytes += 2;
			for(int i = 0; i < *length; i++, bytes++)
				payload->asString[i] = *bytes;
			payload->asString[*length] = 0;
			return bytes;
		case 9:
			type = bytes[0];
			*length = readUInt32(bytes+1);
			payload->asList = malloc(*length * sizeof(tag));
			bytes += 5;
			for(uint32_t i = 0; i < *length; i++) { // define "sublength" so something can be passed for length
				payload->asList[i].name_length = 0;
				payload->asList[i].name = NULL;
				payload->asList[i].id = type;
				bytes = nbtReadPayload(type, &payload->asList[i].length, &payload->asList[i].payload, bytes);
			}
			return bytes;
		case 10:
			*length = 0;
			payload->asCompound = NULL;
			while(bytes[0] != 0) {
				if(*length == 0)
					payload->asCompound = malloc(++*length * sizeof(tag));
				else
					payload->asCompound = realloc(payload->asCompound, ++*length * sizeof(tag));
				bytes = nbtReadInto(payload->asCompound + *length - 1, bytes);
			}
			return bytes + 1;
		case 11:
			*length = readUInt32(bytes);
			payload->asInts = malloc(*length * 4);
			bytes += 4;
			for(int i = 0; i < *length; i++, bytes+=4)
				payload->asInts[i] = readInt32(bytes);
			return bytes;
		case 12:
			*length = readUInt32(bytes);
			payload->asBytes = malloc(*length * 8);
			bytes += 4;
			for(int i = 0; i < *length; i++, bytes+=8)
				payload->asBytes[i] = readInt64(bytes);
			return bytes;
	}
	return bytes + *length;
}

const char* nbtReadInto(tag* tag, const char* bytes) {
	tag->id = bytes[0];
	tag->name_length = readUInt16(bytes + 1);
	tag->name = malloc(tag->name_length);
	memcpy(tag->name, bytes+3, tag->name_length);
	tag->name[tag->name_length] = 0;
	bytes += 3 + tag->name_length;

	// Read payload for respective tag types
	return nbtReadPayload(tag->id, &tag->length, &tag->payload, bytes);
}

tag nbtRead(const char* bytes) {
	tag tag = {0};
	nbtReadInto(&tag, bytes);
	return tag;
}

char* nbtWritePayload(int8_t id, union payload payload, int32_t length, char* bytes) {
	switch (id) {
		default:
			memcpy(bytes, payload.asBytes, length);
			return bytes + length;
		case 7:
			writeUInt32(length, bytes);
			bytes += 4;
			memcpy(bytes, payload.asBytes, length);
			return bytes + length;
		case 8:
			writeUInt16(length, bytes);
			bytes += 2;
			memcpy(bytes, payload.asBytes, length);
			return bytes + length;
		case 9:
			bytes[0] = length == 0 ? 0 : payload.asList[0].id;
			writeUInt32(length, bytes+1);
			bytes += 5;
			for(int i = 0; i < length; i++)
				bytes = nbtWritePayload(payload.asList[i].id, payload.asList[i].payload, payload.asList[i].length, bytes);
			return bytes;
		case 10:
			for(int i = 0; i < length; i++)
				bytes = nbtWrite(payload.asCompound[i], bytes);
			bytes[0] = 0;
			return bytes + 1;
		case 11:
			writeUInt32(length, bytes);
			bytes += 4;
			memcpy(bytes, payload.asBytes, length*4);
			return bytes + length*4;
		case 12:
			writeUInt32(length, bytes);
			bytes += 4;
			memcpy(bytes, payload.asBytes, length*8);
			return bytes + length*8;
	}

}

char* nbtWrite(tag t, char* bytes) {
	// Write id, name length, and name
	*bytes = t.id;
	writeUInt16(t.name_length, ++bytes);
	memcpy(bytes + 2, t.name, t.name_length);
	bytes+=2+t.name_length;

	// Different types of payloads
	return nbtWritePayload(t.id, t.payload, t.length, bytes);
}

size_t nbtPeekLength(tag t) {
	// id + short(name_length) + name_length string(name) + payload size
	size_t out = 1 + 2 + t.name_length;
	switch (t.id) {
		default: return out + t.length;
		case 7: return out + t.length + 4;
		case 8: return out + t.length + 2;
		case 9: 
			for(int i = 0; i < t.length; i++)
				out += nbtPeekLength(t.payload.asList[i]) - 3 - t.payload.asList[i].name_length;
			return out + 5;
		case 11: return out + t.length * 4 + 4;
		case 12: return out + t.length * 8 + 4;
		case 10:
			for(int i = 0; i < t.length; i++)
				out += nbtPeekLength(t.payload.asCompound[i]);
			return out+1;
	}
}

