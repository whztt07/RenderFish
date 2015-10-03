#include "MemoryArena.hpp"
#include <stdlib.h>
#include <algorithm> // max

void * MemoryArena::alloc(uint32_t size) {
	// round up sz to minimum machine alignment
	size = ((size + 15) & (~15)); // 16-bytes alignment
	if (_cur_block_pos + size > _block_size) {
		// get new block of memory for MemoryArena
		_used_blocks.push_back(_current_block);
		if (_available_blocks.size() && size <= _block_size) {
			_current_block = _available_blocks.back();
			_available_blocks.pop_back();
		}
		else {
			_current_block = alloc_aligned<char>(std::max(size, _block_size));
		}
		_cur_block_pos = 0;
	}
	void * ret = _current_block + _cur_block_pos;
	_cur_block_pos += size;
	return ret;
}

void MemoryArena::free_all() {
	_cur_block_pos = 0;
	while (!_used_blocks.empty()) {
		_available_blocks.push_back(_used_blocks.back());
		_used_blocks.pop_back();
	}
}
