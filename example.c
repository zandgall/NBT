#include "nbt.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** args) {
	FILE *f = fopen("out", "rb");
	fseek(f, 0L, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0L, SEEK_SET);
	char* full = malloc(size);
	fread(full, size, 1, f);
	fclose(f);

	tag t = nbtRead(full);
	free(full);

	printf("Read tag: %s\n", t.name);

	size = nbtPeekLength(t);
	full = malloc(size);
	nbtWrite(t, full);
	f = fopen("out2", "wb");
	fwrite(full, size, 1, f);
	fclose(f);
}
