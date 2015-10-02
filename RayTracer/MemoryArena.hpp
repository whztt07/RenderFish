#pragma once
#include "RenderFish.hpp"


#ifndef RENDERFISH_L1_CACHE_LINE_SIZE
#define RENDERFISH_L1_CACHE_LINE_SIZE 64
#endif

static void * alloc_aligned(size_t size) {
#ifdef _WIN32
	return _aligned_malloc(size, RENDERFISH_L1_CACHE_LINE_SIZE);
#endif
}

template <typename T>
static T * alloc_aligned(uint32_t count) {
	return (T *)alloc_aligned(count * sizeof(T));
}

static void free_aligned(void * ptr) {
#ifdef _WIN32
	_aligned_free(ptr);
#endif
}

class MemoryArena
{
private:
	uint32_t _cur_block_pos, _block_size;
	char *_current_block;
	vector<char *> _used_blocks, _available_blocks;
public:
	MemoryArena(uint32_t block_size = 32768)
		: _block_size(block_size), _cur_block_pos(0) {
		_current_block = alloc_aligned<char>(block_size);
	}

	void * alloc(uint32_t size);

	template<typename T>
	T * alloc(uint32_t count = 1) {
		T *ret = (T *)alloc(count * sizeof(T));
		for (uint32_t i = 0; i < count; ++i) {
			new (&ret[i]) T(); // placement new
		}
		return ret;
	}

	void free_all();
};
