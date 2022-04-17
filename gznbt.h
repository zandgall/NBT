#pragma once
#include <zlib.h>
#include <vector>
#include <assert.h>
#define NBT_CHUNK 16384
namespace nbt {
	int deflate(char* in, size_t length, std::vector<char>* out, int level);
	int inflate(char* in, size_t length, std::vector<char>* out);
#ifdef NBT_INCLUDE
#undef NBT_INCLUDE
	int deflate(char* in, size_t length, std::vector<char>* out, int level) {
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		int ret = deflateInit2(&stream, level, Z_DEFLATED, 31, 9, Z_DEFAULT_STRATEGY);
		int flush;
		if (ret != Z_OK)
			return ret;
		size_t index = 0;
		size_t outdex = 0;
		do {
			stream.avail_in = (uInt) std::min<size_t>(length, NBT_CHUNK);

			stream.next_in = (Bytef*)&in[index];

			flush = (index + NBT_CHUNK > length) ? Z_FINISH : Z_NO_FLUSH;

			do {
				stream.avail_out = NBT_CHUNK;
				char* tmpOut = new char[NBT_CHUNK];
				stream.next_out = (Bytef*)&tmpOut[0];

				ret = deflate(&stream, flush);

				out->insert(out->end(), &tmpOut[0], &tmpOut[NBT_CHUNK - stream.avail_out]);
			} while (stream.avail_out == 0);
			
			assert(stream.avail_in == 0);

			index += NBT_CHUNK;
		} while (flush != Z_FINISH);
		assert(ret == Z_STREAM_END);

		(void)deflateEnd(&stream);
		return Z_OK;
	}
	int inflate(char* in, size_t length, std::vector<char>* out) {
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		int ret = inflateInit2(&stream, 31);
		if (ret != Z_OK)
			return ret;
		size_t index = 0;
		size_t outdex = 0;
		do {
			stream.avail_in = (uInt)std::min<size_t>(length, NBT_CHUNK);

			if (stream.avail_in == 0)
				break;

			stream.next_in = (Bytef*)&in[index];
			index += stream.avail_in;
			do {
				stream.avail_out = NBT_CHUNK;
				char* tmpOut = new char[NBT_CHUNK];
				stream.next_out = (Bytef*)&tmpOut[0];

				ret = inflate(&stream, Z_NO_FLUSH);


				assert(ret != Z_STREAM_ERROR);

				switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&stream);
					return ret;
				}

				out->insert(out->end(), &tmpOut[0], &tmpOut[NBT_CHUNK - stream.avail_out]);
				delete[] tmpOut;
			} while (stream.avail_out == 0);

		} while (ret != Z_STREAM_END);
		(void)inflateEnd(&stream);
		return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
	}
#endif
}