#pragma once
#include "RenderFish.hpp"


#ifndef RENDERFISH_L1_CACHE_LINE_SIZE
#define RENDERFISH_L1_CACHE_LINE_SIZE 64
#endif

inline void * alloc_aligned(size_t size) {
#ifdef _WIN32
	return _aligned_malloc(size, RENDERFISH_L1_CACHE_LINE_SIZE);
#endif
}

template <typename T>
inline T * alloc_aligned(uint32_t count) {
	return (T *)alloc_aligned(count * sizeof(T));
}

inline void free_aligned(void * ptr) {
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

template <typename T, int log_block_size> 
class BlockedArray
{
private:
	T * data;
	uint32_t u_resolution, v_resolution, u_blocks;

public:
	BlockedArray(uint32_t nu, uint32_t nv, const T * d = nullptr) 
		: u_resolution(nu), v_resolution(nv) {
		u_blocks = round_up_pow_2(u_resolution) >> log_block_size;
		uint32_t n_alloc = round_up(u_resolution) * round_up(v_resolution);
		data = alloc_aligned<T>(n_alloc);
		for (uint32_t i = 0; i < n_alloc; ++i)
			new (data + i) T();
		if (d) {
			for (uint32_t v = 0; v < v_resolution; ++v)
				for (uint32_t u = 0; u < u_resolution; ++u)
					(*this)(u, v) = d[v * u_resolution + u];
		}

	}
	~BlockedArray() {
		for (uint32_t i = 0; i < u_resolution * v_resolution; ++i)
			data[i].~T();
		free_aligned(data);
	}

	uint32_t block_size() const { return 1 << log_block_size; }
	uint32_t round_up(uint32_t x) const {
		return (x + block_size() - 1) & ~(block_size() - 1);
	}
	uint32_t u_size() const { return u_resolution; }
	uint32_t v_size() const { return v_resolution; }
	uint32_t block(uint32_t a) const { return a >> log_block_size; }
	uint32_t offset(uint32_t a) const { return (a & (block_size() - 1)); }

	T & operator()(uint32_t u, uint32_t v) {
		uint32_t bu = block(u), bv = block(v);
		uint32_t ou = offset(u), ov = offset(v);
		uint32_t offset = block_size() * block_size() * (u_blocks * bv + bu);
		offset += block_size() * ov + ou;
		return data[offset];
	}
	T & operator()(uint32_t u, uint32_t v) const {
		uint32_t bu = block(u), bv = block(v);
		uint32_t ou = offset(u), ov = offset(v);
		uint32_t offset = block_size() * block_size() * (u_blocks * bv + bu);
		offset += block_size() * ov + ou;
		return data[offset];
	}
	void get_linear_array(T * a) const {
		for (uint32_t v = 0; v < v_resolution; ++v)
			for (uint32_t u = 0; u < u_resolution; ++u)
				*a++ = (*this)(u, v);
	}
};

//template <typename T, int 2> class BlockedArray;